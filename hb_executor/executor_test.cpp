#include "executor_test.h"
#include <cassert>
#include <vector>
#include <iostream>
#include <mutex>
using namespace hungbiu;

std::mutex global_mtx;
struct fib_calc
{

	// Data member
	int n;

	// Ctor
	fib_calc(int param) : n(param) {}
	fib_calc(const fib_calc& oth) : n(oth.n) {}
	fib_calc& operator=(const fib_calc& rhs)
	{
		if (this != &rhs) n = rhs.n;
		return *this;
	}

	int operator()(hb_executor::worker_handle& h)
	{
		if (n > 2) {
			auto fut_1 = h.submit(fib_calc{ n - 1 });
			auto fut_2 = h.submit(fib_calc{ n - 2 });
			int n_1 = h.get(fut_1);
			int n_2 = h.get(fut_2);
			
			std::lock_guard lg{ global_mtx };
			std::cout << "fib(" << n - 1 << ") = " << n_1 << '\n';
			std::cout << "fib(" << n - 1 << ") = " << n_2 << '\n';

			return n_1 + n_2;
		}
		else {
			return n;
		}
	}
};

void executor_test()
{
	static constexpr std::pair<int, int> FIB_TAB[] = {
		{20, 6765},
		{30, 832040},
		{40, 102334155}
	};

	hb_executor e{2};
	for (auto& fib : FIB_TAB) {
		auto fut = e.submit(fib_calc{ fib.first });

		// BOOM!!!
		const int result = fut.get();

		assert(result == fib.second);
	}	
}