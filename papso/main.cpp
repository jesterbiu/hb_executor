#include <cstdio>
#include "parallel_async_pso.h"
#include "papso_test.h"
int main() {
	hungbiu::parallel_async_pso papso(rosenbrock, 30, -30, 30);
	papso.set_iteration(10000);
	hungbiu::hb_executor etor(4);
	auto [v, pos] = papso.evolve(etor, 40);
	etor.done();

	printf("best value: %lf\n", v);
	printf("best postion:\n");
	for (auto x : pos) {
		printf("\t%.4lf\n", x);
	}
}


