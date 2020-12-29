#include "kkentzo_test.hpp"
#include "test_functions.hpp"
//void kkentzo_time_test()
//{
//	std::vector<long long> avg_runtimes;
//	auto benchmark = [&](auto f, int loop_sz) {
//		kkentzo::particle_swarm_optimization::duration_t d{ 0 };
//		for (auto i = 0; i < loop_sz; ++i) {
//			d += f();
//		}
//		avg_runtimes.push_back(d.count() / loop_sz);
//	};
//
//	auto ackley_bench = [](double beg, double end, double goal) {
//		kkentzo::particle_swarm_optimization ackley_pso(ackley_function, 2, beg, end);
//		ackley_pso.settings().goal = goal;
//		return ackley_pso(nullptr).second;
//	};
//
//	std::vector<std::pair<double, const std::string>> precesions{
//		{ 1e-6, "1e-6" },
//		{ 8 * 1e-7 ,"8*1e-7" }, { 6 * 1e-7 ,"6*1e-7" },  { 4 * 1e-6, "4 * 1e-6" },
//		{ 2 * 1e-7 ,"2*1e-7" },	{ 1e-7 ,"1e-7" }
//	};
//	for (const auto& t : precesions) {
//		benchmark([&]() {return ackley_bench(-100, 100, t.first); }, 5);
//	}
//
//	for (auto i = 0u; i < avg_runtimes.size(); ++i) {
//		std::cout << "ackley\t-100\t100\t" <<
//			precesions[i].second << ": " <<
//			avg_runtimes[i] << "us\n";
//	}
//}

void kkentzo_pso_test()
{
	auto recal_func = [](std::vector<double>&& vec, double(*f)(double*, int, void*)) {
		return f(vec.data(), vec.size(), nullptr);
	};

	std::cout << "kkentzo pso\n";

	auto tpl = kkentzo::particle_swarm_optimization(ackley_function, 2, -100, 100)(nullptr);
	std::cout << "ackley: " << recal_func(std::move(std::get<1>(tpl)), ackley_function) << '\n';

	tpl = kkentzo::particle_swarm_optimization(pso_sphere, 30, -100, 100)(nullptr);
	std::cout << "sphere: " << recal_func(std::move(std::get<1>(tpl)), pso_sphere) << '\n';

	tpl = kkentzo::particle_swarm_optimization(pso_rosenbrock, 30, -2.048, 2.048)(nullptr);
	std::cout << "rosenbrock: " << recal_func(std::move(std::get<1>(tpl)), pso_rosenbrock) << '\n';

	tpl = kkentzo::particle_swarm_optimization(pso_griewank, 30, -600, 600)(nullptr);
	std::cout << "griewank: " << recal_func(std::move(std::get<1>(tpl)), pso_griewank) << '\n';
}