#pragma once
#include <cmath>


double ackley(const double* vec, size_t dim);
double sphere(const double* vec, size_t dim);
double rosenbrock(const double* vec, size_t dim);
double griewank(const double* vec, size_t dim);

double ackley_function(double* vec, int dim, void* param);
double pso_sphere(double* vec, int dim, void* params);
double pso_rosenbrock(double* vec, int dim, void* params);
double pso_griewank(double* vec, int dim, void* params);
