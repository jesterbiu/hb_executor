#include "simple_pso_test.hpp"
#include "test_functions.hpp"
#include <iostream>


template <size_t dim>
void benchmark_sync(double(*object_function)(const double*, size_t), double Xmin, double  Xmax, const std::string msg)
{
	auto tpl = simple_pso<dim>(object_function, Xmin, Xmax);
	std::cout << msg << ": " << std::get<1>(tpl) << '\n';
}

template <size_t dim>
void benchmark_async(double(*object_function)(const double*, size_t), double Xmin, double  Xmax, const std::string msg)
{
	auto tpl = simple_async_pso<dim>(object_function, Xmin, Xmax);
	std::cout << msg << ": " << std::get<1>(tpl) << '\n';
}

template <size_t dim>
void benchmark_random_async(double(*object_function)(const double*, size_t), double Xmin, double  Xmax, const std::string msg)
{
	auto tpl = random_async_pso<dim>(object_function, Xmin, Xmax);
	std::cout << msg << ": " << std::get<1>(tpl) << '\n';
}

void simple_pso_test()
{
	printf_s("simple pso\n");
	benchmark_sync<2>(ackley, -100.,100., "ackley");
	benchmark_sync<30>(sphere, -100., 100., "sphere");
	benchmark_sync<30>(rosenbrock, -2.048, 2.048, "rosenbrock");
	benchmark_sync<30>(griewank, -600., 600., "griewank");

	printf_s("\nasync pso\n");
	benchmark_async<2>(ackley, -100., 100., "ackley");
	benchmark_async<30>(sphere, -100., 100., "sphere");
	benchmark_async<30>(rosenbrock, -2.048, 2.048, "rosenbrock");
	benchmark_async<30>(griewank, -600., 600., "griewank");

	printf_s("\nrandom async pso\n");
	benchmark_random_async<2>(ackley, -100., 100., "ackley");
	benchmark_random_async<30>(sphere, -100., 100., "sphere");
	benchmark_random_async<30>(rosenbrock, -2.048, 2.048, "rosenbrock");
	benchmark_random_async<30>(griewank, -600., 600., "griewank");
}
