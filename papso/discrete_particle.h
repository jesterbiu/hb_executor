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

template <typename T>
using vec_t = std::vector<T>;

template <size_t D>
struct alignas(64) discrete_particle
{
	using value_type = int;
	using velocity_type = std::pair<size_t, size_t>;
	using position_type = value_type;
	using lock_t = hungbiu::spinlock;

	lock_t m_mtx;
	std::atomic<value_type> m_best_fitness{ std::numeric_limits<value_type>::max() };
	std::atomic<value_type> m_fitness{ std::numeric_limits<value_type>::max() };
	vec_t<velocity_type> m_velocity; 
	vec_t<value_type> m_position; 
	vec_t<value_type> m_best_position;
};

template <size_t D>
struct dpso_position_minus_perm 
{
	using particle_t = discrete_particle<D>;
	using value_type	= typename particle_t::value_type;
	using velocity_type = typename particle_t::velocity_type;
	using position_type = typename particle_t::position_type;

	vec_t<velocity_type> operator()(
		const vec_t<position_type>& pos_dst, 
		const vec_t<position_type>& pos_src)
	{
		assert(D == pos_dst.size() && D == pos_src.size());

		std::unordered_map<value_type, size_t> map_src_idx;
		for (size_t i = 0; i < D; ++i) {
			map_src_idx[pos_src[i]] = i;
		}

		std::vector<position_type> src_mutable = pos_src;
		std::vector<velocity_type> vel;
		vel.reserve(D);
		for (size_t i = 0; i < D; ++i) {
			const auto dst_val = pos_dst[i];
			if (src_mutable[i] == dst_val) continue;
			else {
				// Update velocity
				const auto src_idx = map_src_idx[dst_val];
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
	using velocity_type = typename particle_t::velocity_type;

	vec_t<velocity_type> operator()(double r, const vec_t<velocity_type>& vel)
	{
		assert(0 <= r && r <= 1);
		const auto size_truncated = r * vel.size();
		return { vel.cbegin(), vel.cbegin() + static_cast<size_t>(size_truncated) };
	}
};

template <size_t D>
struct dpso_position_plus_perm
{
	using particle_t = discrete_particle<D>;
	using velocity_type = typename particle_t::velocity_type;
	using position_type = typename particle_t::position_type;

	vec_t<position_type> operator()(const vec_t<position_type>& pos, const vec_t<velocity_type>& vel)
	{
		vec_t<position_type> new_pos = pos;
		for (const auto& v : vel) {
			std::swap(new_pos[v.first], new_pos[v.second]);
		}
		return new_pos;
	}
};