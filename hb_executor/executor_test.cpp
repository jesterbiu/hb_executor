#include "executor_test.h"
#include <cassert>
#include <vector>
#include <iostream>
#include <mutex>
using namespace hungbiu;
std::mutex gmtx;
struct fib_calc
{
	static constexpr int FIB_LOWER[] = { 0, 1, 1 };

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
			return h.get(fut_1) + h.get(fut_2);	
		}
		else {
			return FIB_LOWER[n];
		}
	}
};

void executor_test()
{
	static constexpr std::pair<int, int> FIB_TAB[] = {
		{6, 8},
		{10, 55},
		{20, 6765},
		{30, 832040}
	};

	hb_executor e{4};
	for (auto& f : FIB_TAB) {
		auto fut = e.submit(fib_calc{ f.first });
		try {
			assert(fut.get() == f.second);
			std::cout << "fib(" << f.first << ") succeed\n";
		}
		catch (std::exception& e) {
			std::cout << e.what() << '\n';
		}
	}	
	e.done();
}