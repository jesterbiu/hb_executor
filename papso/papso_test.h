#pragma once
#include "papso.h"
#include "test_functions.h"

template <size_t D>
void benchmark(object_function_type f, double Xmin, double Xmax, const std::string msg)
{
	par_async_dpso<D> p{ f, 1e-5,Xmin, Xmax };
	double best_fitness{ 0. };
	std::vector<double> best_position(D);
	p(best_fitness, best_position);
	printf_s("%s: %lf\n", msg.c_str(), best_fitness);
	//printf("best postion: \n");
	//for (auto i : best_position) {
	//	printf("%lf, ", i);
	//}
	printf("\n");
}

void hungbiu_pso_test();
