#include "executor_test.h"
#include <atomic>
#include <vector>
using namespace hungbiu;
struct parallel_travel
{
	struct parallel_travel_task : hb_executor::runnable_base<parallel_travel>
	{
		static size_t left_child(size_t idx)
		{
			return  idx * 2 + 1;
		}
		static size_t right_child(size_t idx)
		{
			return  idx * 2 + 2;
		}
		std::vector<int>* pvec;
		size_t idx;

		int operator()(hb_executor::worker_handle& h)
		{
			int sum = 0;
			size_t root = idx;
			sum += pvec->at(root);

			if (pvec->size() > left_child(idx)) {
				h.run_next(parallel_travel_task{ pvec, left_child(idx) });
			}
		}
	};
	
	
	int operator()(hb_executor& e)
	{
		
	}
};
void executor_test()
{

}

struct my
{
	void myfunc()
	{
		my m();
	}
};