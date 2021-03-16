#include <cstdio>
#include "papso2.h"
#include "papso_test.h"
#include "test_functions.h"
template <typename F>
void parallel_async_pso_benchmark(F&& f, double min, double max, const char* const msg) {	
	auto [v, pos] = apso(f, min, max);
	printf_s("par async pso @%s: %lf\n", msg, v);
	printf("\n");
}

void par_async_pso_test(size_t i) {
	parallel_async_pso_benchmark(
		test_functions::functions[i]
		, test_functions::bounds[i].first, test_functions::bounds[i].second
		, test_functions::function_names[i]);
}

int main() {
	for (size_t i = 0; i < test_functions::functions.size(); ++i) {
		hungbiu_pso_test(i);
		par_async_pso_test(i);
	}

}


