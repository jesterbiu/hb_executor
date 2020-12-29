#include <cmath>
#include <memory>
#include <iostream>
#include <cassert>
#include <vector>
#include <chrono>
#include <tuple>
#include <algorithm>

#include "time_counter.hpp"
#include "pso.h"

namespace kkentzo
{
	class particle_swarm_optimization
	{
		// Type def & factory function
		using settings_ptr = std::unique_ptr<pso_settings_t, void(*)(pso_settings_t*)>;
		static settings_ptr make_pso_settings(int dimensions, double beg, double end)
		{
			return settings_ptr{ pso_settings_new(dimensions, beg, end),
						  pso_settings_free
			};
		}
		static void default_settings(pso_settings_t* ps)
		{
			ps->goal = 1e-5;
			ps->size = 40;
			ps->nhood_strategy = PSO_NHOOD_GLOBAL;
			ps->w_strategy = PSO_W_CONST;
		}
		// Data members
		pso_obj_fun_t func_;
		int dimensions_;
		settings_ptr psettings_;
		std::unique_ptr<double[]> buffer_;
		pso_result_t result_;

	public:
		particle_swarm_optimization(pso_obj_fun_t func, int dimensions, double beg, double end) :
			func_(func),
			dimensions_(dimensions),
			psettings_(make_pso_settings(dimensions, beg, end)),
			buffer_(std::make_unique<double[]>(dimensions))
		{
			assert(dimensions > 0);
			assert(psettings_.operator bool());
			default_settings(psettings_.get());
			result_.gbest = buffer_.get();
		}
		particle_swarm_optimization(particle_swarm_optimization&& oth) noexcept :
			func_(oth.func_),
			dimensions_(oth.dimensions_),
			psettings_(std::move(oth.psettings_)),
			buffer_(std::move(oth.buffer_)),
			result_(oth.result_) {}
		particle_swarm_optimization& operator=(particle_swarm_optimization&& rhs) noexcept
		{
			if (this != &rhs) {
				func_ = rhs.func_;
				dimensions_ = rhs.dimensions_;
				psettings_ = std::move(rhs.psettings_);
				buffer_ = std::move(rhs.buffer_);
			}
			return *this;
		}

		pso_settings_t& settings() noexcept
		{
			return *psettings_;
		}

		using counter_t = hungbiu::time_counter<std::chrono::microseconds>;
		using duration_t = counter_t::duration_t;
		std::tuple<double, std::vector<double>, duration_t> operator()(void* param)
		{

			counter_t tc;
			pso_solve(func_, param, &result_, psettings_.get());
			auto te = tc.get_time_elapsed();
			std::vector<double> result;
			result.reserve(dimensions_);
			auto pbuf = buffer_.get();
			for (auto i = 0; i < dimensions_; ++i) {
				result.push_back(pbuf[i]);
			}			
			return std::make_tuple(result_.error, std::move(result), te);
		}
	};

};

void kkentzo_pso_test();