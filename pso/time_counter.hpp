#include <chrono>
#include <vector>
namespace hungbiu
{
	template <typename TimeUnit = std::chrono::milliseconds>
	class time_counter
	{
	public:
		using clock_t = std::chrono::high_resolution_clock;
		using duration_t = TimeUnit;
	private:
		clock_t::time_point start_;
	public:
		time_counter() : 
			start_(clock_t::now()) 
		{}
		TimeUnit get_time_elapsed()
		{
			return std::chrono::duration_cast<TimeUnit>(clock_t::now() - start_);
		}
	};
}