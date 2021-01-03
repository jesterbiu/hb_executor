#pragma once
#if PAPSO_TEST
#include "papso_test.h"
#include <cstdio>
#include <cmath>
double sphere(const double* vec, size_t dim) {

	double sum = 0;
	for (size_t i = 0; i < dim; i++)
		sum += pow(vec[i], 2);
	return sum;
}

double rosenbrock(const double* vec, size_t dim) {
	double sum = 0;
	for (size_t i = 0; i < dim - 1; i++)
		sum += 100 * pow((vec[i + 1] - pow(vec[i], 2)), 2) + \
		pow((1 - vec[i]), 2);
	return sum;

}

double griewank(const double* vec, size_t dim) {

	double sum = 0.;
	double prod = 1.;
	for (size_t i = 0; i < dim; i++) {
		sum += pow(vec[i], 2);
		prod *= cos(vec[i] / sqrt(i + 1));
	}

	return sum / 4000. - prod + 1.;

}

void hungbiu_pso_test()
{
	printf_s("hungbiu random async pso\n");
	benchmark<30>(sphere, -100., 100., "sphere");
	benchmark<30>(rosenbrock, -2.048, 2.048, "rosenbrock");
	benchmark<30>(griewank, -600., 600., "griewank");
}
#endif