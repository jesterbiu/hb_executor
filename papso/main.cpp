#include "tsp.h"
#include "discrete_papso.h"
#include "tsp_data.h"

struct tsp_solve
{
	const coord* m_map;

	tsp_solve(const coord* ptr_map) : m_map(ptr_map) {}
	int operator()(const int* pos, size_t sz)
	{
		std::vector<int> pos_vec(sz);
		for (size_t i = 0; i < sz; ++i) pos_vec[i] = pos[i];

		// Implicityly convert float point to integer
		return static_cast<int>(tsp(pos_vec, m_map)); 
	}
};

int main() 
{
	constexpr auto D = 52u;
	par_async_dpso<D, tsp_solve> dpso(
		tsp_solve{tsp_data::berlin52},	// Object func
		7542,							// Goal
		0, D - 1						// Min, Max
	);
	double best_fit{};
	std::vector<int> best_pos;
	bool b = dpso(best_fit, best_pos);
	printf_s("%lf\n(", best_fit);
	for (size_t i = 0; i < D; ++i) {
		printf_s("%d", best_pos[i]);
		D - 1 == i ?
			printf_s(")\n") :
			printf_s(", ");
	}
}