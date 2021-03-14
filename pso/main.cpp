#include "kkentzo_test.hpp"
#include "test_functions.hpp"
#include <tuple>
int main()
{
	auto [fitness, pos, duration] = kkentzo::particle_swarm_optimization(pso_rosenbrock, 30, -30, 30)(nullptr);
	std::cout << "fitness: " << fitness << "\nbest postion: ";
	for (auto x : pos) {
		std::cout << x << '\n';
	}
}