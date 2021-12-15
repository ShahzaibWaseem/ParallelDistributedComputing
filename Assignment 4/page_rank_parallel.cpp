#include "core/graph.h"
#include "core/utils.h"
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <thread>
#include <stdexcept>
#include <atomic>

#define DEFAULT_GRANULARITY "1"

#ifdef USE_INT
#define INIT_PAGE_RANK 100000
#define EPSILON 1000
#define PAGE_RANK(x) (15000 + (5 * x) / 6)
#define CHANGE_IN_PAGE_RANK(x, y) std::abs(x - y)
typedef int64_t PageRankType;
#else
#define INIT_PAGE_RANK 1.0
#define EPSILON 0.01
#define DAMPING 0.85
#define PAGE_RANK(x) (1 - DAMPING + DAMPING * x)
#define CHANGE_IN_PAGE_RANK(x, y) std::fabs(x - y)
typedef float PageRankType;
#endif

uint n_workers;
std::atomic <long> vertex1, vertex2;
double barrier_1_time = 0.0, barrier_2_time = 0.0, wait_vertex_time = 0.0;

void addAtomic(std::atomic <PageRankType> &pr, PageRankType value){
	PageRankType prev_pr = pr.load(std::memory_order_consume);
	PageRankType desired_pr = prev_pr + value;

	while(!pr.compare_exchange_weak(prev_pr, desired_pr, std::memory_order_release, std::memory_order_consume)){
		desired_pr = prev_pr + value;
	}
}

long getNextVertexToBeProcessed(std::atomic <long> &vertex, uint granularity){
	return vertex.fetch_add(granularity, std::memory_order_relaxed);
}

void pageRankDynamic(Graph &g, PageRankType *pr_curr, std::atomic <PageRankType> *pr_next, CustomBarrier &barrier, uint max_iters, uint granularity, uint thread_id){
	uintV n = g.n_;
	uint edge_count = 0, vertex_count = 0;
	double time_taken = 0.0;
	timer thread_timer, b1_timer, b2_timer, wait_vertex_timer;
	
	thread_timer.start();

	for (uint iter = 0; iter < max_iters; iter++) {
		// for each vertex 'u', process all its outNeighbors 'v'
		while(1){
			wait_vertex_timer.start();
			uintV u = getNextVertexToBeProcessed(vertex1, granularity);
			wait_vertex_time += wait_vertex_timer.stop();
			if (u == -1 || u >= n)
				break;
			
			for(uint k = 0; k < granularity; k++){
				uintE out_degree = g.vertices_[u].getOutDegree();
				for (uintE i = 0; i < out_degree; i++) {
					uintV v = g.vertices_[u].getOutNeighbor(i);
					addAtomic(pr_next[v], pr_curr[u]/out_degree);
				}
				edge_count += out_degree;
				u++;
				if (u >= n)
					break;
			}
		}
		b1_timer.start();
		barrier.wait();
		barrier_1_time += b1_timer.stop();
		vertex1 = 0;
		
		while(1){
			wait_vertex_timer.start();
			uintV v = getNextVertexToBeProcessed(vertex2, granularity);
			wait_vertex_time += wait_vertex_timer.stop();
			if (v == -1 || v >= n)
				break;
			
			for(uint k = 0; k < granularity; k++){
				pr_next[v] = PAGE_RANK(pr_next[v]);
				// reset pr_curr for the next iteration
				pr_curr[v] = pr_next[v];
				pr_next[v] = 0.0;
				vertex_count++;
				v++;
				if (v >= n)
					break;
			}
		}
		b2_timer.start();
		barrier.wait();
		barrier_2_time += b2_timer.stop();
		vertex2 = 0;
	}
	time_taken = thread_timer.stop();
	printf("%d, %d, %d, %f, %f, %f, %f\n", thread_id, vertex_count, edge_count, barrier_1_time, barrier_2_time, wait_vertex_time, time_taken);
}

void dynamicThreadSpawn(Graph &g, uint max_iters, uint n_workers, uint granularity) {
	uintV n = g.n_;
	timer t1;
	double time_taken = 0.0;
	CustomBarrier barrier(n_workers);
	std::thread threads[n_workers];

	PageRankType *pr_curr = new PageRankType[n];
	std::atomic <PageRankType> *pr_next = new std::atomic <PageRankType>[n];

	for (uintV i = 0; i < n; i++) {
		pr_curr[i] = INIT_PAGE_RANK;
		pr_next[i] = 0.0;
	}
	
	t1.start();
	for(uint th = 0; th < n_workers; th++){
		threads[th] = std::thread(pageRankDynamic, std::ref(g), pr_curr, pr_next, std::ref(barrier), max_iters, granularity, th);
	}

	for(uint th = 0; th < n_workers; th++){
		threads[th].join();
	}
	time_taken = t1.stop();
	
	PageRankType sum_of_page_ranks = 0;
	for (uintV u = 0; u < n; u++) {
		sum_of_page_ranks += pr_curr[u];
	}
	std::cout << "Sum of page rank : " << sum_of_page_ranks << "\n";
	std::cout << "Time taken (in seconds) : " << time_taken << "\n";
	delete[] pr_curr;
	delete[] pr_next;
}

int main(int argc, char *argv[]) {
	cxxopts::Options options("page_rank_push", "Calculate page_rank using serial and parallel execution");
	options.add_options("",{
		{"nWorkers", "Number of workers",
			cxxopts::value<uint>()->default_value(DEFAULT_NUMBER_OF_WORKERS)},
		{"nIterations", "Maximum number of iterations",
			cxxopts::value<uint>()->default_value(DEFAULT_MAX_ITER)},
		{"granularity", "Granularity of the division",
			cxxopts::value<uint>()->default_value(DEFAULT_GRANULARITY)},
		{"inputFile", "Input graph file path",
			cxxopts::value<std::string>()->default_value("/scratch/input_graphs/roadNet-CA")},
	});

	auto cl_options = options.parse(argc, argv);
	n_workers = cl_options["nWorkers"].as<uint>();
	uint max_iterations = cl_options["nIterations"].as<uint>();
	uint granularity = cl_options["granularity"].as<uint>();
	std::string input_file_path = cl_options["inputFile"].as<std::string>();

#ifdef USE_INT
	std::cout << "Using INT\n";
#else
	std::cout << "Using FLOAT\n";
#endif
	std::cout << std::fixed;
	std::cout << "Number of workers : " << n_workers << "\n";
	std::cout << "Granularity : " << granularity << "\n";
	std::cout << "Iterations : " << max_iterations << "\n";

	Graph g;
	std::cout << "Reading graph\n";
	g.readGraphFromBinary<int>(input_file_path);
	std::cout << "Created graph\n";
	std::cout << "thread_id, num_vertices, num_edges, barrier1_time, barrier2_time, getNextVertex_time, total_time\n";
	
	dynamicThreadSpawn(g, max_iterations, n_workers, granularity);
	return 0;
}