#pragma once
#include "papso.h"
#include "test_functions.h"

template <size_t D>
void papso_benchmark(const object_function_type f, const double Xmin, const double Xmax, const char* const msg)
{
	par_async_dpso<D> p{ f, 1e-5,Xmin, Xmax };
	std::vector<double> best_position(D);
	double best_fitness = std::numeric_limits<double>::max();
	best_fitness = p(best_position);
	printf("%s: %lf\n", msg, best_fitness);
	printf("\n");
}

void hungbiu_pso_test(size_t i);
