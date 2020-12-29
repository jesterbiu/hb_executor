#include "test_functions.hpp"
double ackley(const double* vec, size_t dim)
{
	// 16 digits Pi
	static constexpr double PI = 245850922.0 / 78256779.0;

	auto x = vec[0], y = vec[1];
	return -20 * std::exp(-0.2 * std::sqrt(0.5 * (x * x + y * y)))
		- std::exp(0.5 * (std::cos(2 * PI * x) + std::cos(2 * PI * y)))
		+ 22.718282L;
}

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

double ackley_function(double* vec, int dim, void* param)
{
	// 16 digits Pi
	static constexpr double PI = 245850922.0 / 78256779.0;

	auto x = vec[0], y = vec[1];
	return -20 * std::exp(
		-0.2 * std::sqrt(0.5 * (x * x + y * y)))
		- std::exp(0.5 * (std::cos(2 * PI * x)
			+ std::cos(2 * PI * y)))
		+ 22.718282L;
}

double pso_sphere(double* vec, int dim, void* params) {

	double sum = 0;
	int i;
	for (i = 0; i < dim; i++)
		sum += pow(vec[i], 2);

	return sum;
}

double pso_rosenbrock(double* vec, int dim, void* params) {

	double sum = 0;
	int i;
	for (i = 0; i < dim - 1; i++)
		sum += 100 * pow((vec[i + 1] - pow(vec[i], 2)), 2) + \
		pow((1 - vec[i]), 2);

	return sum;

}

double pso_griewank(double* vec, int dim, void* params) {

	double sum = 0.;
	double prod = 1.;
	int i;
	for (i = 0; i < dim; i++) {
		sum += pow(vec[i], 2);
		prod *= cos(vec[i] / sqrt(i + 1));
	}

	return sum / 4000. - prod + 1.;

}