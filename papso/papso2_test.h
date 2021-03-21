#ifndef _PAPSO_TEST
#define _PAPSO_TEST
#include "papso2.h"
#include "test_functions.h"

void parallel_async_pso_benchmark(hungbiu::hb_executor& etor, std::size_t fork_count, const func_t f, bound_t bounds, const char* const msg) {
	auto result = papso::parallel_async_pso(etor, fork_count, f, bounds);
	auto [v, pos] = result.get();
	printf_s("\npar async pso @%s: %lf\n", msg, v);
	printf("\n");
}

void par_async_pso_test(hungbiu::hb_executor& etor, std::size_t fork_count, std::size_t i) {
	parallel_async_pso_benchmark(
		etor
		, fork_count
		, test_functions::functions[i]
		, test_functions::bounds[i]
		, test_functions::function_names[i]);
}

void robust_test() {
	static constexpr size_t forks[] = { 1, 2, 4, 8 };
	static constexpr size_t loop_size = 4;
	for (size_t fork_count : forks) {
		for (auto i = 0; i < loop_size; ++i) {
			// Create thread pool
			hungbiu::hb_executor etor(fork_count);

			// Run test functions
			for (std::size_t j = 0; j < 4; ++j) {
				par_async_pso_test(etor, fork_count, j);
			}

			// End thread pool
			etor.done();
		}
	}
}

void speedup_benchmark(size_t fork_count, size_t loop_size = 4) {
	hungbiu::hb_executor etor(fork_count);
	for (size_t i = 0; i < loop_size; ++i) {
		for (size_t j = 0; j < test_functions::functions.size(); ++j) {
			par_async_pso_test(etor, fork_count, j);
		}
	}
	etor.done();
}

#endif