#include <cstdio>
#include "papso2.h"
#include "papso_test.h"
#include "test_functions.h"
void parallel_async_pso_benchmark(hungbiu::hb_executor& etor, const func_t f, bound_t bounds, const char* const msg) {	
	auto [v, pos] = apso(etor, f, bounds);
	printf_s("par async pso @%s: %lf\n", msg, v);
	printf("\n");
}

void par_async_pso_test(hungbiu::hb_executor& etor, std::size_t i) {
	parallel_async_pso_benchmark(
		etor
		, test_functions::functions[i]
		, test_functions::bounds[i]
		, test_functions::function_names[i]);
}

int main() {
	hungbiu::hb_executor etor(fork_count);
	for (std::size_t i = 0; i < test_functions::functions.size(); ++i) {
		//hungbiu_pso_test(i);
		par_async_pso_test(etor, i);
	}
	etor.done();
}


