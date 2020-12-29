#pragma once
#pragma once
#include <mutex>
#include <atomic>
#include <limits>
#include <array>
#include <vector>
#include <type_traits>
#include <concepts>
#include <unordered_map>
#include <cassert>
#include "../../concurrent_data_structures/concurrent_data_structures/spinlock.h"

#define ALIGN_REQ alignas(64)

template <size_t D>
struct ALIGN_REQ discrete_particle
{
	using value_type = int;
	template <typename T>
	using cont_t = std::vector;
	using velocity_type = std::pair<size_t, size_t>;
	using position_type = value_type;
	using lock_t = hungbiu::spinlock;

	lock_t m_mtx;
	std::atomic<value_type> m_best_fitness{ std::numeric_limits<value_type>::max() };
	std::atomic<value_type> m_fitness{ std::numeric_limits<value_type>::max() };
	cont_t<velocity_type> m_velocity; 
	cont_t<value_type> m_position; 
	cont_t<value_type> m_best_position;
};

template <size_t D>
struct discrete_particle_algorithm_type_info
{
	using particle_t = discrete_particle<D>;
	template <typename T>
	using cont_t = particle_t::cont_t;
	using value_type = particle_t::value_type;
	using velocity_type = particle_t::velocity_type;
	using position_type = particle_t::position_type;
};

template <size_t D>
struct dpso_position_minus_perm 
{
	using particle_t = discrete_particle<D>;
	template <typename T>
	using cont_t = particle_t::cont_t;
	using value_type = particle_t::value_type;
	using velocity_type = particle_t::velocity_type;
	using position_type = particle_t::position_type;

	cont_t<velocity_type> operator()(
		const cont_t<position_type>& pos_dst, 
		const cont_t<position_type>& pos_src)
	{
		std::unordered_map<value_type, size_t> map_src_idx;
		for (size_t i = 0; i < D; ++i) {
			map_idx_src[pos_dst[i]] = i;
		}

		cont_t<position_type> src_mutable = pos_dst;
		cont_t<velocity_type> vel;
		vel.reserve(D);
		for (size_t i = 0; i < D; ++i) {
			const auto dst_val = pos_src[i];
			if (src_mutable[i] == dst_val) continue;
			else {
				// Update velocity
				const src_idx = map_src_idx[dst_val];
				vel.emplace_back(i, src_idx);

				// Swap element to the right position
				std::swap(src_mutable[i], src_mutable[src_idx]);
				
				// Update lookup table
				map_src_idx[src_mutable[i]] = i;
				map_src_idx[src_mutable[src_idx]] = src_idx;
			}			
		} // end of for
		return vel;
	}
};

template <size_t D>
struct dpso_velocity_multiplies_perm
{
	using particle_t = discrete_particle<D>;
	template <typename T>
	using cont_t = particle_t::cont_t;
	using velocity_type = particle_t::velocity_type;

	void operator()(double r, cont_t<velocity_type>& vel)
	{
		assert(0 <= r && r < 1);
		const size_t size_truncated = r * vel.size();
		r.resize(size_truncated);
	}
};

template <size_t D>
struct dpso_position_plus_perm
{
	using particle_t = discrete_particle<D>;
	template <typename T>
	using cont_t = particle_t::cont_t;
	using velocity_type = particle_t::velocity_type;
	using position_type = particle_t::position_type;

	void operator()(cont_t<position_type>& pos, const cont_t<velocity_type>& vel)
	{
		for (const auto& v : vel) {
			std::swap(pos[v.first], pos[v.second]);
		}
	}
};