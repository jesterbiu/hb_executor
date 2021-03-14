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


// --------------------------------------------------------------------------------
// Particle definition
// --------------------------------------------------------------------------------

template <typename P>
concept Particle = requires {
	typename P::value_type; 
	typename P::velocity_type;
	typename P::position_type;
};

template <typename P>
constexpr bool is_particle = Particle<P>;

template <typename V, typename Vel, typename Pos>
struct alignas(64) basic_particle
{
	using lock_type		= hungbiu::spinlock;
	using value_type	= V;
	using velocity_type = std::vector<Vel>;
	using position_type = std::vector<Pos>;

	lock_type m_mtx;
	std::atomic<V> m_best_fitness{ std::numeric_limits<V>::max() };
	std::atomic<V> fitness{ std::numeric_limits<V>::max() };
	velocity_type m_velocity;
	position_type m_position;
	position_type m_best_position;
};

using discrete_particle = basic_particle<int, std::pair<size_t, size_t>, int>;

// --------------------------------------------------------------------------------
// Function object templates that update a particle's state
// Should be stateless and copy constructible
// --------------------------------------------------------------------------------

// <pos> - <pos> -> <vel>
template <typename P, typename F>
concept position_minus =
	is_particle<P> &&
	std::is_default_constructible_v<F> &&
	std::is_copy_constructible_v<F> &&
	std::is_invocable_r<	// F(const pos&, const pos&) -> vel
		typename P::velocity_type, F,
		typename P::position_type const&,
		typename P::position_type const&>::value;

// <pos> + <vel> -> <pos>
template <typename P, typename F>
concept position_plus = 
	is_particle<P> &&
	std::is_default_constructible_v<F> &&
	std::is_copy_constructible_v<F> &&
	std::is_invocable_r<			// F(pos&&, const vel&) -> pos
		typename P::position_type, F,	
		typename P::position_type&&,		
		const typename P::velocity_type&>::value &&
	std::is_invocable_r<			// F(const pos&, const vel&) -> pos
		typename P::position_type, F,
		typename P::position_type const&,
		typename P::velocity_type const&>::value;

// <float point> * <vel> -> <vel>
template <typename P, typename F, typename T = double>
concept velocity_multiplies = 
	is_particle<P> &&
	std::is_floating_point_v<T> &&
	std::is_default_constructible_v<F> &&
	std::is_copy_constructible_v<F> &&
	std::is_invocable_r<				// F(Real, vel&&) -> vel
		typename P::velocity_type,	F,	
		T, typename P::velocity_type&>::value &&
	std::is_invocable_r<				// F(Real, const vel&) -> vel
		typename P::velocity_type, F,
		T, typename P::velocity_type const&>::value;

// --------------------------------------------------------------------------------
// Functor implementations for `discrete particle`
// --------------------------------------------------------------------------------
struct dpso_position_minus_perm 
{
	using particle_t	= discrete_particle;
	using value_type	= typename particle_t::value_type;
	using velocity_type = typename particle_t::velocity_type;
	using position_type = typename particle_t::position_type;

	velocity_type 
	operator()(const position_type& pos_dst, const position_type& pos_src) const
	{
		const size_t D = pos_dst.size();
		assert(D == pos_dst.size() && D == pos_src.size());

		std::unordered_map<value_type, size_t> map_src_idx;
		for (size_t i = 0; i < D; ++i) {
			map_src_idx[pos_src[i]] = i;
		}

		position_type src_mutable = pos_src;
		velocity_type vel;
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

struct dpso_velocity_multiplies_perm
{
	using particle_t = discrete_particle;
	using velocity_type = typename particle_t::velocity_type;

	velocity_type operator()(double r, const velocity_type& vel) const
	{
		velocity_type ret = vel;
		multiplies(r, ret);
		return ret;
	}
	velocity_type operator()(double r, velocity_type&& vel) const
	{
		multiplies(r, vel);
		return vel;		
	}

private:	
	void multiplies(double r, velocity_type& vel) const {
		size_t trunc_size = static_cast<size_t>(
			(r > 1.0 ? (r - 1.0) : r) * static_cast<double>(vel.size())
		);

		if (r > 1) {
			vel.insert(vel.cend(), vel.cbegin(), vel.cbegin() + trunc_size);
		}
		else { // 0 <= r <= 1
			vel.resize(trunc_size);
		}
	}
};

struct dpso_position_plus_perm
{
	using particle_t = discrete_particle;
	using velocity_type = typename particle_t::velocity_type;
	using position_type = typename particle_t::position_type;

	position_type operator()(const position_type& pos, const velocity_type& vel) const
	{
		position_type new_pos = pos;
		for (const auto& v : vel) {
			std::swap(new_pos[v.first], new_pos[v.second]);
		}
		return new_pos;
	}

	position_type operator()(position_type&& pos, const velocity_type& vel) const
	{
		for (const auto& v : vel) {
			std::swap(pos[v.first], pos[v.second]);
		}
		return pos;
	}
};

struct dpso_velocity_plus_perm {
	using particle_t = discrete_particle;
	using velocity_type = typename particle_t::velocity_type;

	velocity_type operator()(const velocity_type& lhs, const velocity_type& rhs) const {
		velocity_type vel;
		vel.reserve(lhs.size() + rhs.size());
		vel.insert(vel.cend(), lhs.cbegin(), lhs.cend());
		vel.insert(vel.cend(), rhs.cbegin(), rhs.cend());
		return vel;
	}
};