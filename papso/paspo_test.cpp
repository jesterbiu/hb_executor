#pragma once
#define PAPSO_TEST 1
#if PAPSO_TEST
#include "papso_test.h"
#include <cstdio>

template <size_t>
void hungbiu_pso_test()
{
	printf_s("hungbiu random async pso\n");
	benchmark<30>(sphere, -100., 100., "sphere");
	benchmark<30>(rosenbrock, -30, 30, "rosenbrock");
	benchmark<30>(griewank, -600., 600., "griewank");
}
#endif