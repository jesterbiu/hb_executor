#include "simple_pso_test.hpp"
#include "test_functions.hpp"
int main()
{
	printf_s("\nasync pso\n");
	benchmark_async<2>(ackley, -100., 100., "ackley");
	benchmark_async<30>(sphere, -100., 100., "sphere");
	benchmark_async<30>(rosenbrock, -2.048, 2.048, "rosenbrock");
	benchmark_async<30>(griewank, -600., 600., "griewank");
}