#pragma once
#include <array>
#include <limits>
template <size_t D>
struct particle
{
	using array_t = std::array<double, D>;
	double best_fitness{ std::numeric_limits<double>::max_x() };
	double fitness{ std::numeric_limits<double>::max_x() };
	array_t velocity, position, best_position;
};