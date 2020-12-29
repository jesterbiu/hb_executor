#include "function_evaluation_test.h"
#include "time_counter.hpp"
#include "canonical_rng.hpp"
#include "test_functions.hpp"
#include <cmath>
#include <vector>
#include <array>
#include <algorithm>
#include <cstdio>
#include <iostream>

void function_evaluation_test()
{

	printf_s("dimension\trosenbrock\tgriewank\n");

	// 2 fucntion * dimensions{ 30, 100, 250, 500, 1000 } * 20 runs per dimension
	using std::chrono::nanoseconds;
	static constexpr std::array Dimensions	{ 300u, 1000u, 2500u, 5000u, 10000u };
	static constexpr auto RunsPerDimension = 20u;
	std::vector<double> domain;
	auto get_time_elapsed = [&](auto f) {
		hungbiu::time_counter<nanoseconds> tc;
		rosenbrock(domain.data(), domain.size());
		return tc.get_time_elapsed();
	};

	for (const auto d : Dimensions) {
		printf_s("%lu\t\t", d);

		domain.resize(d);
		nanoseconds rosenbrock_time{ 0 }, griewank_time{ 0 };

		for (size_t i = 0; i < RunsPerDimension; ++i) {
			std::for_each(domain.begin(), domain.end(), [](auto& val) { 
				val = canonical_rng(); });
			rosenbrock_time += get_time_elapsed(rosenbrock);
			griewank_time += get_time_elapsed(griewank);
		}

		printf_s("%llu\t\t%llu\n", 
				rosenbrock_time.count() / RunsPerDimension, 
				griewank_time.count() / RunsPerDimension);
	}
}