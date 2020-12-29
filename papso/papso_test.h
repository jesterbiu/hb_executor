#pragma once
#include "papso.h"
double sphere(const double* vec, size_t dim);
double rosenbrock(const double* vec, size_t dim);
double griewank(const double* vec, size_t dim);

template <size_t D>
void benchmark(double(*f)(const double*, size_t), double Xmin, double Xmax, const std::string msg)
{
	par_async_dpso<D> p{ f, 1e-5,Xmin, Xmax };
	double best_fitness{ 0. };
	std::vector<double> best_position(D);
	p(best_fitness, best_position);
	printf_s("%s: %lf\n", msg.c_str(), best_fitness);
}

void hungbiu_pso_test();
