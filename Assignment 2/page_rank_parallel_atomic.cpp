#include "core/graph.h"
#include "core/utils.h"
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <thread>
#include <stdexcept>
#include <atomic>

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
PageRankType *pr_curr, *thread_sums;
std::atomic <PageRankType> *pr_next;

void partialSum(uint thread_id, uintV start_n, uintV subset_n, PageRankType *pr_curr, PageRankType *thread_sums){
	for (uintV u = start_n; u < subset_n; u++) {
		thread_sums[thread_id] += pr_curr[u];
	}
}

void addAtomic(std::atomic <PageRankType> &pr_next, PageRankType value){
	PageRankType prev_pr = pr_next.load(std::memory_order_consume);
	PageRankType desired_pr = prev_pr + value;

	while(!pr_next.compare_exchange_weak(prev_pr, desired_pr, std:: memory_order_release, std::memory_order_consume)){
		desired_pr = prev_pr + value;
	}
}

void pageRankCalculation(Graph &g, CustomBarrier &barrier, uint max_iters, uintV start_n, uintV subset_n, uint thread_id, PageRankType *thread_sums){
	timer thread_timer;
	double time_taken = 0.0;
	thread_timer.start();

	for (uint iter = 0; iter < max_iters; iter++) {
		// for each vertex 'u', process all its outNeighbors 'v'
		for (uintV u = start_n; u < subset_n; u++) {
			uintE out_degree = g.vertices_[u].getOutDegree();
			for (uintE i = 0; i < out_degree; i++) {
				uintV v = g.vertices_[u].getOutNeighbor(i);
				addAtomic(pr_next[v], pr_curr[u] / out_degree);
			}
		}

		barrier.wait();

		for (uintV v = start_n; v < subset_n; v++) {
			pr_next[v] = PAGE_RANK(pr_next[v]);
			// reset pr_curr for the next iteration
			pr_curr[v] = pr_next[v];
			pr_next[v] = 0.0;
		}
		barrier.wait();
	}
	time_taken = thread_timer.stop();
	printf("%d, %f\n", thread_id, time_taken);
	partialSum(thread_id, start_n, subset_n, pr_curr, thread_sums);
}

void pageRankParallel(Graph &g, uint max_iters, uint n_workers) {
	uintV n = g.n_;
	uintV extra_n = (uintV) n%n_workers;
	uintV start_n = 0, end_n = (n/n_workers);
	bool offset = false;

	CustomBarrier barrier(n_workers);

	thread_sums = new PageRankType[n];
	pr_curr = new PageRankType[n];
	pr_next = new std::atomic <PageRankType> [n];

	for (uintV i = 0; i < n; i++) {
		pr_curr[i] = INIT_PAGE_RANK;
		pr_next[i] = 0.0;
	}
	
	// Push based pagerank
	std::thread threads[n_workers];
	timer t1;
	double time_taken = 0.0;

	t1.start();

	for(uint th = 0; th < n_workers; th++){
		// spawning threads (data evenly distributed)
		if (th == n_workers - 1)
			end_n += extra_n;
		threads[th] = std::thread(pageRankCalculation, std::ref(g), std::ref(barrier), max_iters, start_n, end_n, th, thread_sums);
		start_n = end_n;
		end_n = start_n+(n/n_workers);
	}
	
	for(uint th = 0; th < n_workers; th++){
		threads[th].join();
	}
	
	time_taken = t1.stop();
	
	PageRankType sum_of_page_ranks = 0;

	for (uintV th = 0; th < n_workers; th++) {
		sum_of_page_ranks += thread_sums[th];
	}

	std::cout << "Sum of page rank : " << sum_of_page_ranks << "\n";
	std::cout << "Time taken (in seconds) : " << time_taken << "\n";
	delete[] pr_curr;
	delete[] pr_next;
	delete[] thread_sums;
}

int main(int argc, char *argv[]) {
	cxxopts::Options options("page_rank_push", "Calculate page_rank using serial and parallel execution");
	options.add_options("",{
		{"nWorkers", "Number of workers",
			cxxopts::value<uint>()->default_value(DEFAULT_NUMBER_OF_WORKERS)},
		{"nIterations", "Maximum number of iterations",
			cxxopts::value<uint>()->default_value(DEFAULT_MAX_ITER)},
		{"inputFile", "Input graph file path",
			cxxopts::value<std::string>()->default_value("/scratch/input_graphs/roadNet-CA")},
	});

	auto cl_options = options.parse(argc, argv);
	n_workers = cl_options["nWorkers"].as<uint>();
	uint max_iterations = cl_options["nIterations"].as<uint>();
	std::string input_file_path = cl_options["inputFile"].as<std::string>();

#ifdef USE_INT
	std::cout << "Using INT\n";
#else
	std::cout << "Using FLOAT\n";
#endif
	std::cout << std::fixed;
	std::cout << "Number of workers : " << n_workers << "\n";

	Graph g;
	std::cout << "Reading graph\n";
	g.readGraphFromBinary<int>(input_file_path);
	std::cout << "Created graph\n";
	std::cout << "thread_id, time_taken\n";
	pageRankParallel(g, max_iterations, n_workers);

	return 0;
}