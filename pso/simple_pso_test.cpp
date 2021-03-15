#include "simple_pso_test.hpp"
#include "test_functions.hpp"


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
