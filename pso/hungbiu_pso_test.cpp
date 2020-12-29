#include <cmath>
#include <cstdio>
#include <string>
#include "hungbiu_pso_test.hpp"
using namespace hungbiu;
struct ackley_function
{
	double operator()(const double* vec, size_t dim)
	{
		// 16 digits Pi
		static constexpr double PI = 245850922.0 / 78256779.0;

		auto x = vec[0], y = vec[1];
		return -20 * std::exp(-0.2 * std::sqrt(0.5 * (x * x + y * y)))
			- std::exp(0.5 * (std::cos(2 * PI * x)
				+ std::cos(2 * PI * y))) + 22.718282L;
	}
};

struct sphere
{
	double operator()(const double* vec, size_t dim)
	{
		double sum = 0;
		int i;
		for (i = 0; i < dim; i++) {
			sum += pow(vec[i], 2);
		}
		return sum;
	}
};

struct rosenbrock
{
	double operator()(const double* vec, size_t dim)
	{
		static constexpr auto a = 1, b = 100;
		double sum = 0.;
		for (size_t i = 0; i < dim - 1; ++i) {
			const auto x = vec[i], y = vec[i + 1];
			sum += std::pow((a - x), 2)
				+ b * std::pow((y - std::pow(x, 2)), 2);
		}

		return sum;
	}
};

struct griewank
{ 
	double operator()(const double* vec, size_t dim)
	{
		double sum = 0.;
		double prod = 1.;
		for (size_t i = 0; i < dim; i++) {
			sum += std::pow(vec[i], 2);
			prod *= std::cos(vec[i] / std::sqrt(i + 1));
		}

		return sum / 4000. - prod + 1.;
	}
};
template <typename F, size_t dim, typename Cont>
void benchmark(F&& f, const Cont& Xmin, const Cont& Xmax, const std::string msg)
{
	pso<F,
		dim,	// Dimensions
		80,		// Swarm size 
		24,		// Neighbor size
		1000	// Iteration size
	> p{
		f, 1e-5,
		std::cbegin(Xmin), std::cend(Xmin),
		std::cbegin(Xmax), std::cend(Xmax) };
	p();
	printf_s("%s: %lf\n", msg.c_str(), p.global_best.load());
}

void hungbiu_pso_test()
{
	printf_s("hungbiu random async pso\n");
	benchmark<ackley_function, 2>(
		ackley_function{},
		std::vector<double>(2, -100.), 
		std::vector<double>(2, 100.), 
		"ackley");
	benchmark<sphere, 30>(
		sphere{},
		std::vector<double>(30, -100.),
		std::vector<double>(30, 100.),
		"sphere");
	benchmark<rosenbrock, 30>(
		rosenbrock{},
		std::vector<double>(30, -2.048),
		std::vector<double>(30, 2.048),
		"rosenbrock");
	benchmark<griewank, 30>(
		griewank{},
		std::vector<double>(30, -600.),
		std::vector<double>(30, 600.),
		"griewank");
}