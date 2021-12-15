#include "core/graph.h"
#include "core/utils.h"
#include <future>
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <thread>
#define DEFAULT_STRATEGY "1"

uint n_workers;
std::atomic<long> triangle_count (n_workers);

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

void triangleCountEdge(Graph &g, uintV start_n, uintV end_n, uintV start_e, uintV end_e, uint thread_id){
	long local_triangle_count = 0, edge_count = 0;
	timer parallel_timer;
	double time_taken = 0.0;
	uint first_e = start_e, last_e = 0;

	parallel_timer.start();

	for (uintV u = start_n; u <= end_n; u++) {
		// For each outNeighbor v, find the intersection of inNeighbor(u) and
		// outNeighbor(v)
		uintE out_degree = g.vertices_[u].getOutDegree();
		last_e = out_degree;
		if(u != start_n)
			first_e = 0;
		if(u == end_n)
			last_e = end_e;
		for (uintE i = first_e; i < last_e; i++) {
			uintV v = g.vertices_[u].getOutNeighbor(i);
			local_triangle_count += countTriangles(g.vertices_[u].getInNeighbors(),
				g.vertices_[u].getInDegree(),
				g.vertices_[v].getOutNeighbors(),
				g.vertices_[v].getOutDegree(), u, v);
			edge_count++;
		}
	}

	std::atomic_fetch_add(&triangle_count, local_triangle_count);

	time_taken = parallel_timer.stop();
	printf("%d, %d, %ld, %ld, %f\n", thread_id, end_n - start_n, edge_count, local_triangle_count, time_taken);
}

void edgeThreadSpawn(Graph &g){
	uintV n = g.n_;
	uintV n_edges;
	uintV extra_n = (uintV) n%n_workers;
	uintV extra_e = 0;
	uintV start_n = 0, end_n = (n/n_workers), current_n=0;
	uintV start_e = 0, end_e = 0, thread_edges=0;
	double time_taken = 0.0, partitioning_time = 0.0;
	timer t1, t2;

	std::thread threads[n_workers];
	t1.start();
	// Counting the number of edges
	for(uint u = 0; u < n; u++)
		n_edges += g.vertices_[u].getOutDegree();

	end_e = n_edges/n_workers;
	extra_e = n_edges;
	uintV end_e_temp=0;

	for(uint th = 0; th < n_workers; th++){
		t2.start();
		if (th < n_workers - 1){
			end_e = n_edges/n_workers;
			extra_e -= end_e;
		}
		else
			end_e = extra_e;
		partitioning_time += t2.stop();
		thread_edges = 0;

		start_n = current_n;
		start_e = end_e_temp;
		while(1){
			t2.start();
			thread_edges += g.vertices_[current_n].getOutDegree() - end_e_temp;
			end_e_temp = 0;
			if(thread_edges >= end_e){
				end_e_temp = end_e - (thread_edges - g.vertices_[current_n].getOutDegree());
				partitioning_time += t2.stop();
				threads[th] = std::thread(triangleCountEdge, std::ref(g), start_n, current_n, start_e, end_e_temp, th);
				break;
			}
			current_n++;
		}
	}
	
	for(uint th = 0; th < n_workers; th++){
		threads[th].join();
	}

	time_taken = t1.stop();

	std::cout << "Number of triangles : " << triangle_count << "\n";
	std::cout << "Number of unique triangles : " << triangle_count / 3 << "\n";
	std::cout << "Partitioning time (in seconds) : " << std::setprecision(TIME_PRECISION) << partitioning_time << "\n";
	std::cout << "Time taken (in seconds) : " << std::setprecision(TIME_PRECISION) << time_taken << "\n";
}

void triangleCountVertex(Graph &g, uintV start_n, uintV end_n, uint thread_id){
	long local_triangle_count = 0, edge_count = 0;
	timer parallel_timer;
	double time_taken = 0.0;

	parallel_timer.start();

	for (uintV u = start_n; u < end_n; u++) {
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
	}
	std::atomic_fetch_add(&triangle_count, local_triangle_count);

	time_taken = parallel_timer.stop();
	printf("%d, %d, %ld, %ld, %f\n", thread_id, end_n - start_n, edge_count, local_triangle_count, time_taken);
}

void vertexThreadSpawn(Graph &g) {
	uintV n = g.n_;
	double time_taken = 0.0, partitioning_time = 0.0;
	uintV extra_n = (uintV) n%n_workers;
	uintV start_n = 0, end_n = (n/n_workers);
	timer t1, t2;

	// Create threads and distribute the work across T threads
	std::thread threads[n_workers];
	t1.start();

	for(uint th = 0; th < n_workers; th++){
		t2.start();
		if (th == n_workers - 1)
			end_n += extra_n;
		partitioning_time += t2.stop();
		// spawning threads (data evenly distributed)
		threads[th] = std::thread(triangleCountVertex, std::ref(g), start_n, end_n, th);
		start_n = end_n;
		end_n = start_n+(n/n_workers);
	}
	
	for(uint th = 0; th < n_workers; th++){
		threads[th].join();
	}

	time_taken = t1.stop();

	std::cout << "Number of triangles : " << triangle_count << "\n";
	std::cout << "Number of unique triangles : " << triangle_count / 3 << "\n";
	std::cout << "Partitioning time (in seconds) : " << std::setprecision(TIME_PRECISION) << partitioning_time << "\n";
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
			{"strategy", "Vertex (1) based task division or Edge (2) based task division",
				cxxopts::value<uint>()->default_value(DEFAULT_STRATEGY)},
			{"inputFile", "Input graph file path",
				cxxopts::value<std::string>()->default_value(
					"/scratch/assignment1/input_graphs/roadNet-CA")},
		}
	);

	auto cl_options = options.parse(argc, argv);
	n_workers = cl_options["nWorkers"].as<uint>();
	std::string input_file_path = cl_options["inputFile"].as<std::string>();
	std::cout << std::fixed;
	std::cout << "Number of workers : " << n_workers << "\n";

	uint strategy = cl_options["strategy"].as<uint>();
	std::cout << "Task decomposition strategy : " << strategy << "\n";

	Graph g;
	std::cout << "Reading graph\n";
	g.readGraphFromBinary<int>(input_file_path);
	std::cout << "Created graph\n";
	std::cout << "\nthread_id, num_vertices, num_edges, triangle_count, time_taken\n";

	switch (strategy){
	case 1:
		vertexThreadSpawn(g);
		break;
	case 2:
		edgeThreadSpawn(g);
		break;
	default:
		break;
	}

	return 0;
}