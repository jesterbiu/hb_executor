#include <cstdio>
#include <string>
#include "papso2.h"
#include "papso_test.h"
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

int main(int argc, const char* argv[]) {
	/*size_t fork_count = 0;
	if (argc > 1) {
		fork_count = std::stoul(std::string{ argv[1] });
	}
	else {
		printf("enter forks: ");
		std::cin >> fork_count;
	}*/

	size_t fork_counts[] = { 1, 2, 4, 8 };

	for (auto i = 0; i < 8; ++i) {
		for (auto fork_count : fork_counts) {
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


