#include "discrete_particle_test.h"
#include "tsp_test.h"
#include "alt_dpso_tsp.h"
#include <iostream>
int main() {
	alt_dpso_tsp pso{ 100, 100, 4 };
	std::vector<std::vector<int>> best_pos(3);
	std::cout << "bl52: " << pso.evolve(tsp_data::berlin52, 52, best_pos[0]) << '\n';
	//std::cout << "pr76: " << pso.evolve(tsp_data::pr76, 76, best_pos[1]) << '\n';
	//std::cout << "gr96: " << pso.evolve(tsp_data::gr96, 96, best_pos[2]) << '\n';  // 2835??
}


