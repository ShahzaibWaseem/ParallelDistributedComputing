#include "core/utils.h"
#include <iomanip>
#include <iostream>
#include <stdlib.h>
#include <thread>

#define sqr(x) ((x) * (x))
#define DEFAULT_NUMBER_OF_POINTS "12345678"

uint c_const = (uint)RAND_MAX + (uint)1;
uint n_workers;
std::atomic<uint> circle_points(n_workers);

inline double get_random_coordinate(uint *random_seed) {
	return ((double)rand_r(random_seed)) / c_const;
}

void get_points_in_circle(uint n, uint thread_id, uint random_seed) {
	timer parallel_timer;
	double time_taken = 0.0;
	uint circle_count = 0;
	double x_coord, y_coord;

	parallel_timer.start();

	for (uint i = 0; i < n; i++) {
		x_coord = (2.0 * get_random_coordinate(&random_seed)) - 1.0;
		y_coord = (2.0 * get_random_coordinate(&random_seed)) - 1.0;
		if ((sqr(x_coord) + sqr(y_coord)) <= 1.0)
			circle_count++;
	}

	time_taken = parallel_timer.stop();

	std::atomic_fetch_add(&circle_points, circle_count);
	printf("%d, %d, %d, %f\n", thread_id, n, circle_count, time_taken);
}

void piCalculation(uint n) {
	timer serial_timer;
	double time_taken = 0.0;
	uint extra_n = n%n_workers;
	uint random_seed = time(NULL);
	
	serial_timer.start();

	// Create threads and distribute the work across T threads
	std::thread threads[n_workers];

	for(uint th = 0; th < n_workers; th++){
		// spawning threads (data evenly distributed)
		threads[th] = std::thread(get_points_in_circle, n/n_workers + (extra_n+th)/(n_workers), th, random_seed + th);
	}

	for(uint th = 0; th < n_workers; th++){
		threads[th].join();
	}
	
	double pi_value = 4.0 * (double)circle_points / (double)n;

	time_taken = serial_timer.stop();

	std::cout << "Total points generated : " << n << "\n";
	std::cout << "Total points in circle : " << circle_points << "\n";
	std::cout << "Result : " << std::setprecision(VAL_PRECISION) << pi_value
		<< "\n";
	std::cout << "Time taken (in seconds) : " << std::setprecision(TIME_PRECISION)
		<< time_taken << "\n";
}

int main(int argc, char *argv[]) {
	srand(time(NULL));

	// Initialize command line arguments
	cxxopts::Options options("pi_calculation",
		"Calculate pi using serial and parallel execution");
	options.add_options(
		"custom",
		{
			{"nPoints", "Number of points",
				cxxopts::value<uint>()->default_value(DEFAULT_NUMBER_OF_POINTS)},
			{"nWorkers", "Number of workers",
				cxxopts::value<uint>()->default_value(DEFAULT_NUMBER_OF_WORKERS)},
		});

	auto cl_options = options.parse(argc, argv);
	uint n_points = cl_options["nPoints"].as<uint>();
	n_workers = cl_options["nWorkers"].as<uint>();
	std::cout << std::fixed;
	std::cout << "Number of points : " << n_points << "\n";
	std::cout << "Number of workers : " << n_workers << "\n";
	std::cout << "thread_id, points_generated, circle_points, time_taken\n";

	piCalculation(n_points);

	return 0;
}