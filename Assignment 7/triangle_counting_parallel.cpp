#include <iostream>
#include <cstdio>
#include "core/utils.h"
#include "core/graph.h"
#include <mpi.h>

uintV countTriangles(uintV *array1, uintE len1, uintV *array2, uintE len2, uintV u, uintV v) {
	uintE i = 0, j = 0; // indexes for array1 and array2
	uintV count = 0;
	
	if (u == v)
		return count;

  	while ((i < len1) && (j < len2)) {
		if (array1[i] == array2[j]) {
			if ((array1[i] != u) && (array1[i] != v)) {
				count++;
			}
			else {
				// triangle with self-referential edge -> ignore
			}
			i++;
			j++;
		}
		else if (array1[i] < array2[j]) {
			i++;
		}
		else {
			j++;
		}
	}
	return count;
}

void distributeWork(Graph &g, uintV* verticesArray, uintV n, int n_workers){
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
}

void triangleCountEdge(Graph &g, uintV* verticesArray, uint strategy){
	uintV n = g.n_;
	long local_triangle_count = 0, global_triangle_count = 0, edge_count = 0;
	long *local_triangle_counts = NULL;
	timer communication_timer;
	double communication_time = 0.0;
	int process_rank, n_processes;
	
	// Find out rank, size
	MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &n_processes);

	if (process_rank == 0 && strategy == 1) {
		local_triangle_counts = (long *)malloc(sizeof(long) * n_processes);
	}

	for (uintV u = verticesArray[process_rank]; u < verticesArray[process_rank+1]; u++) {
		// For each outNeighbor v, find the intersection of inNeighbor(u) and outNeighbor(v)
		uintE out_degree = g.vertices_[u].getOutDegree();
		edge_count += out_degree;
		for (uintE i = 0; i < out_degree; i++) {
			uintV v = g.vertices_[u].getOutNeighbor(i);
			local_triangle_count += countTriangles(g.vertices_[u].getInNeighbors(),
				g.vertices_[u].getInDegree(),
				g.vertices_[v].getOutNeighbors(),
				g.vertices_[v].getOutDegree(), u, v);
		}
	}

	communication_timer.start();
	switch (strategy){
		case 1:
			MPI_Gather(&local_triangle_count, 1, MPI_LONG, local_triangle_counts, 1, MPI_LONG, 0, MPI_COMM_WORLD);
			break;
		case 2:
			MPI_Reduce(&local_triangle_count, &global_triangle_count, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
		default:
			break;
	}
	communication_time = communication_timer.stop();

	if (process_rank == 0 && strategy == 1) {
		for (int process = 0; process < n_processes; process++) {
			global_triangle_count += local_triangle_counts[process];
		}
	}

	std::printf("%d, %ld, %ld, %f\n", process_rank, edge_count, local_triangle_count, communication_time);

	if (process_rank == 0) {
		std::printf("Number of triangles : %ld\n", global_triangle_count);
		std::printf("Number of unique triangles : %ld\n", global_triangle_count/3);
	}
}

int main(int argc, char *argv[]) {
	cxxopts::Options options("triangle_counting_serial", "Count the number of triangles using serial and parallel execution");
	options.add_options("custom", {
		{"strategy", "Strategy to be used", cxxopts::value<uint>()->default_value(DEFAULT_STRATEGY)},
		{"inputFile", "Input graph file path", cxxopts::value<std::string>()->default_value("/scratch/input_graphs/roadNet-CA")},
	});

	auto cl_options = options.parse(argc, argv);
	uint strategy = cl_options["strategy"].as<uint>();
	std::string input_file_path = cl_options["inputFile"].as<std::string>();

	// Initialize the MPI environment
	MPI_Init(NULL, NULL);

	// Find out rank, size
	int process_rank, n_processes;
	MPI_Comm_rank(MPI_COMM_WORLD, &process_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &n_processes);

	Graph g;
	g.readGraphFromBinary<int>(input_file_path);

	if (process_rank == 0) {
		// Get the world size and print it out here
		std::printf("World size : %d\n", n_processes);
		std::printf("Communication strategy : %d\n", strategy);
		std::printf("rank, edges, triangle_count, communication_time\n");
	}

	// Distributing work at this stage makes the program run faster on RMAT Graph
	uintV n = g.n_;
	uintV* verticesArray = (uintV*) calloc(n_processes, sizeof(uintV));
	distributeWork(g, verticesArray, n, n_processes);

	timer overall_timer;
	overall_timer.start();

	triangleCountEdge(g, verticesArray, strategy);

	if (process_rank == 0) {
		double overall_time = overall_timer.stop();
		std::printf("Time taken (in seconds) : %f\n", overall_time);
	}
	
	free(verticesArray);
	MPI_Finalize();
	return 0;
}