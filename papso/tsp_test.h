#pragma once
#include "tsp.h"
#include "tsp_data.h"
#include "discrete_papso.h"
#include <array>

struct pso_params
{
	size_t NeighborSize;
	size_t SwarmSize;
	size_t Iteration;
};
static constexpr pso_params params[] = {
		{2, 40, 200},
		{2, 80, 200},
		{2, 160, 200},
		{12, 40, 200},
		{12, 80, 200},
		{12, 160, 200},
		{40, 40, 200},
		{40, 160, 200},
};
constexpr size_t neighbor_size(size_t i)
{
	return params[i].NeighborSize;
}
constexpr size_t swarm_size(size_t i)
{
	return params[i].NeighborSize;
}
constexpr size_t iteration_size(size_t i)
{
	return params[i].Iteration;
}
void run_tsp_pso(size_t i)
{
	static constexpr auto D = 52u;
	par_async_dpso<tsp_solve> dpso(
		D,									// Dimension
		tsp_solve{ tsp_data::berlin52 },	// Object function
		7542,								// Goal
		0, D - 1							// Min, Max
	); // Don't use initialize list
	dpso.set_params(
		params[i].NeighborSize, 
		params[i].SwarmSize, 
		params[i].Iteration);
	double best_fit{};
	std::vector<int> best_pos;
	bool b = dpso.run_serialized(best_fit, best_pos);
	printf_s("NEIGHBOR:%llu, SWARM: %llu, ITER: %llu, RESULT: %lf\n", 
		params[i].NeighborSize,
		params[i].SwarmSize,
		params[i].Iteration,
		best_fit);
}
void tsp_pso_test()
{
	for (auto i = 0u; i < sizeof(params) / sizeof(pso_params); ++i) {
		run_tsp_pso(i);
	}
}