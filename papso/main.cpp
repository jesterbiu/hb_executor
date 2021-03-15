#include <cstdio>
#include "parallel_async_pso.h"
#include "papso_test.h"

template <size_t Dim, size_t NSize, size_t ISize>
void parallel_async_pso_benchmark(double(*fp)(const double*, size_t), double min, double max, const char* msg) {
	hungbiu::parallel_async_pso papso(fp, Dim, min, max);
	papso.set_neighbor_size(NSize);
	papso.set_iteration(ISize);
	auto [v, pos] = papso.evolve();
	printf_s("%s: %lf\n", msg, v);
	/*printf("best postion: ");
	for (auto x : pos) {
		printf("%.4lf, ", x);
	}*/
	printf("\n");
}

void parallel_async_pso_test() {
	/*parallel_async_pso_benchmark<30, 4, 1000>(sphere, -100., 100., "sphere");
	parallel_async_pso_benchmark<30, 4, 1000>(rosenbrock, -30, 30, "rosenbrock");
	parallel_async_pso_benchmark<30, 4, 1000>(griewank, -600., 600., "griewank");*/
}


int main() {
	hungbiu_pso_test();
}


