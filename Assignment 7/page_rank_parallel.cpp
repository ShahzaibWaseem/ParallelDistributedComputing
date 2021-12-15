#include <iostream>
#include <cstdio>
#include "core/utils.h"
#include "core/graph.h"
#include <mpi.h>

#ifdef USE_INT
#define INIT_PAGE_RANK 100000
#define EPSILON 1000
#define PAGE_RANK(x) (15000 + (5 * x) / 6)
#define CHANGE_IN_PAGE_RANK(x, y) std::abs(x - y)
#define PAGERANK_MPI_TYPE MPI_LONG
#define PR_FMT "%ld"
typedef int64_t PageRankType;
#else
#define INIT_PAGE_RANK 1.0
#define EPSILON 0.01
#define DAMPING 0.85
#define PAGE_RANK(x) (1 - DAMPING + DAMPING * x)
#define CHANGE_IN_PAGE_RANK(x, y) std::fabs(x - y)
#define PAGERANK_MPI_TYPE MPI_FLOAT
#define PR_FMT "%f"
typedef float PageRankType;
#endif

void distributeWork(Graph &g, uintV* verticesArray, uintV* countsArray, uintV n, int n_workers){
	uintE n_edges = 0, edges_per_worker = 0;
	int curr_worker = 0;
	verticesArray[n_workers] = n;

	// Counting the number of edges
	for(uintV u = 0; u < n; u++)
		n_edges += g.vertices_[u].getOutDegree();

	for(uintV u = 0, edges_per_worker = n_edges/n_workers, n_edges = 0; u < n; u++) {
		n_edges += g.vertices_[u].getOutDegree();
		if (n_edges > edges_per_worker * curr_worker) {
			verticesArray[curr_worker++] = u;
			if (curr_worker == n_workers)
				break;
		}
	}
	for(int process = 0; process < n_workers; process++) {
		countsArray[process] = verticesArray[process+1] - verticesArray[process];
	}
}

void pageRankEdge(Graph &g, int max_iters, uintV* verticesArray, uintV* countsArray, int strategy) {
	uintV n = g.n_;
	long edge_count = 0;
	timer communication_timer;
	double communication_time = 0.0;
	int process_rank, n_processes;
	
	// Find out rank, size
	MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &n_processes);

	PageRankType *pr_curr = new PageRankType[n];
	PageRankType *pr_next = new PageRankType[n];
	PageRankType *pr_buffer = new PageRankType[n];
	PageRankType *pr_partial_buffer = new PageRankType[verticesArray[process_rank+1] - verticesArray[process_rank]];

	for (uintV i = 0; i < n; i++) {
		pr_curr[i] = INIT_PAGE_RANK;
		pr_next[i] = 0.0;
	}

	for (int iter = 0; iter < max_iters; iter++) {
		// for each vertex 'u', process all its outNeighbors 'v'
		// LOOP 1
		for (uintV u = verticesArray[process_rank]; u < verticesArray[process_rank+1]; u++) {
			uintE out_degree = g.vertices_[u].getOutDegree();
			edge_count += out_degree;
			for (uintE i = 0; i < out_degree; i++) {
				uintV v = g.vertices_[u].getOutNeighbor(i);
				pr_next[v] += (pr_curr[u] / out_degree);
			}
		}

		// SYNCHRONIZATION PHASE 1 START
		communication_timer.start();

		switch (strategy){
			case 1:
				MPI_Reduce(pr_next, pr_buffer, n, PAGERANK_MPI_TYPE, MPI_SUM, 0, MPI_COMM_WORLD);
				MPI_Scatterv(pr_buffer, countsArray, verticesArray, PAGERANK_MPI_TYPE, pr_partial_buffer, verticesArray[process_rank+1] - verticesArray[process_rank], PAGERANK_MPI_TYPE, 0, MPI_COMM_WORLD);
				break;
			case 2:
				for (int process = 0; process < n_processes; process++)
					MPI_Reduce(&pr_next[verticesArray[process]], pr_partial_buffer, countsArray[process], PAGERANK_MPI_TYPE, MPI_SUM, process, MPI_COMM_WORLD);
				break;
			default:
				break;
		}
		communication_time = communication_timer.stop();
		// SYNCHRONIZATION PHASE 1 END

		// LOOP 2
		for (uintV v = verticesArray[process_rank]; v < verticesArray[process_rank+1]; v++) {
			pr_next[v] = PAGE_RANK(pr_partial_buffer[v - verticesArray[process_rank]]);

			// reset pr_curr for the next iteration
			pr_curr[v] = pr_next[v];
		}
		for (uintV v = 0; v < n; v++)
			pr_next[v] = 0.0;
	}
	std::printf("%d, %ld, %f\n", process_rank, edge_count, communication_time);

	PageRankType local_sum = 0, global_sum = 0;
	// LOOP 3
	for (uintV u = verticesArray[process_rank]; u < verticesArray[process_rank+1]; u++)
		local_sum += pr_curr[u];

	// SYNCHRONIZATION PHASE 2 START
	MPI_Reduce(&local_sum, &global_sum, 1, PAGERANK_MPI_TYPE, MPI_SUM, 0, MPI_COMM_WORLD);
	// SYNCHRONIZATION PHASE 2 END

	if (process_rank == 0)
		std::printf("Sum of page rank : " PR_FMT "\n", global_sum);

	delete[] pr_curr;
	delete[] pr_next;
	delete[] pr_buffer;
	delete[] pr_partial_buffer;
}

int main(int argc, char *argv[]) {
	cxxopts::Options options("page_rank_push", "Calculate page_rank using serial and parallel execution");
	options.add_options("", {
		{"nIterations", "Maximum number of iterations", cxxopts::value<uint>()->default_value(DEFAULT_MAX_ITER)},
		{"strategy", "Strategy to be used", cxxopts::value<uint>()->default_value(DEFAULT_STRATEGY)},
		{"inputFile", "Input graph file path", cxxopts::value<std::string>()->default_value("/scratch/input_graphs/roadNet-CA")},});

	auto cl_options = options.parse(argc, argv);
	uint strategy = cl_options["strategy"].as<uint>();
	uint max_iterations = cl_options["nIterations"].as<uint>();
	std::string input_file_path = cl_options["inputFile"].as<std::string>();

	// Initialize the MPI environment
	MPI_Init(NULL, NULL);

	Graph g;
	g.readGraphFromBinary<int>(input_file_path);

	// Find out rank, size
	int process_rank, n_processes;
	MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &n_processes);

	if (process_rank == 0) {
	#ifdef USE_INT
		std::printf("Using INT\n");
	#else
		std::printf("Using FLOAT\n");
	#endif
		// Get the world size and print it out here
		std::printf("World size : %d\n", n_processes);
		std::printf("Communication strategy : %d\n", strategy);
		std::printf("Iterations : %d\n", max_iterations);
		std::printf("rank, num_edges, communication_time\n");
	}

	// Distributing work at this stage makes the program run faster on RMAT Graph
	uintV n = g.n_;
	uintV* verticesArray = (uintV*) calloc(n_processes, sizeof(uintV));
	uintV* countsArray = (uintV*) calloc(n_processes, sizeof(uintV));
	distributeWork(g, verticesArray, countsArray, n, n_processes);
	
	timer overall_timer;
	overall_timer.start();
	
	pageRankEdge(g, max_iterations, verticesArray, countsArray, strategy);

	if (process_rank == 0) {
		double overall_time = overall_timer.stop();
		std::printf("Time taken (in seconds) : %f\n", overall_time);
	}

	free(verticesArray);
	free(countsArray);
	MPI_Finalize();
	return 0;
}