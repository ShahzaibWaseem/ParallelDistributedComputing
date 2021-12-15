#include "core/graph.h"
#include "core/utils.h"
#include <future>
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <thread>

uint n_workers;
std::atomic<long> triangle_count (n_workers), curr_vertex;

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
			i++;
			j++;
		} else if (array1[i] < array2[j]) {
			i++;
		} else {
			j++;
		}
	}
	return count;
}

long getNextVertexToBeProcessed(){
	return curr_vertex.fetch_add(1, std::memory_order_relaxed);
}

void triangleCountDynamic(Graph &g, uint thread_id){
	uintV n = g.n_;
	long local_triangle_count = 0, edge_count = 0, vertex_count = 0;
	timer parallel_timer;
	double time_taken = 0.0;

	parallel_timer.start();

	while(1){
		uintV u = getNextVertexToBeProcessed();
		if(u >= n)
			break;

		// For each outNeighbor v, find the intersection of inNeighbor(u) and
		// outNeighbor(v)
		uintE out_degree = g.vertices_[u].getOutDegree();
		for (uintE i = 0; i < out_degree; i++) {
			uintV v = g.vertices_[u].getOutNeighbor(i);
			local_triangle_count += countTriangles(g.vertices_[u].getInNeighbors(),
				g.vertices_[u].getInDegree(),
				g.vertices_[v].getOutNeighbors(),
				g.vertices_[v].getOutDegree(), u, v);
			edge_count++;
		}
		vertex_count++;
	}

	std::atomic_fetch_add(&triangle_count, local_triangle_count);

	time_taken = parallel_timer.stop();
	printf("%d, %ld, %ld, %ld, %f\n", thread_id, vertex_count, edge_count, local_triangle_count, time_taken);
}

void dynamicThreadSpawn(Graph &g){
	timer t1;
	double time_taken = 0.0;
	std::thread threads[n_workers];
	
	t1.start();

	for(uint th = 0; th < n_workers; th++){
		threads[th] = std::thread(triangleCountDynamic, std::ref(g), th);
	}
	for(uint th = 0; th < n_workers; th++){
		threads[th].join();
	}

	time_taken = t1.stop();

	std::cout << "Number of triangles : " << triangle_count << "\n";
	std::cout << "Number of unique triangles : " << triangle_count / 3 << "\n";
	std::cout << "Time taken (in seconds) : " << std::setprecision(TIME_PRECISION) << time_taken << "\n";
}

int main(int argc, char *argv[]) {
	cxxopts::Options options(
		"triangle_counting_serial",
		"Count the number of triangles using serial and parallel execution");
	options.add_options(
		"custom",
		{
			{"nWorkers", "Number of workers",
				cxxopts::value<uint>()->default_value(DEFAULT_NUMBER_OF_WORKERS)},
			{"inputFile", "Input graph file path",
				cxxopts::value<std::string>()->default_value("/scratch/assignment1/input_graphs/roadNet-CA")},
		}
	);

	auto cl_options = options.parse(argc, argv);
	n_workers = cl_options["nWorkers"].as<uint>();
	std::string input_file_path = cl_options["inputFile"].as<std::string>();
	std::cout << std::fixed;
	std::cout << "Number of workers : " << n_workers << "\n";

	Graph g;
	std::cout << "Reading graph\n";
	g.readGraphFromBinary<int>(input_file_path);
	std::cout << "Created graph\n";
	std::cout << "thread_id, num_vertices, num_edges, triangle_count, time_taken\n";

	dynamicThreadSpawn(g);
	return 0;
}