#pragma once
#include <iostream>
#include "simple_pso.hpp"
#include "simple_async_pso.hpp"

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


void simple_pso_test();