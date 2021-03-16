#pragma once
#define PAPSO_TEST 1
#if PAPSO_TEST
#include "papso_test.h"
#include <cstdio>

void hungbiu_pso_test(size_t i){
	papso_benchmark<30>(
		test_functions::functions[i]
		, test_functions::bounds[i].first, test_functions::bounds[i].second
		, test_functions::function_names[i]);
}
#endif