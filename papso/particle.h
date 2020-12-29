#pragma once
#include <mutex>
#include <atomic>
#include <limits>
#include <array>
#include <vector>
#include <type_traits>
#include <concepts>
#include "../../concurrent_data_structures/concurrent_data_structures/spinlock.h"

#define ALIGN_REQ alignas(64)

template <size_t D>
struct ALIGN_REQ particle
{
	using cont_t = std::array<double, D>;
	using lock_t = std::mutex;

	std::mutex mtx;
	std::atomic<double> best_fitness{ std::numeric_limits<double>::max() };
	std::atomic<double> fitness{ std::numeric_limits<double>::max() };
	cont_t velocity, position, best_position;
};

// position
// velocity
// pos - pos -> velocity
// real_number * velocity -> velocity
// pos + vel -> pos
template <typename P>
concept particle_represent = requires {
	typename P::position_type;
	typename P::velocity_type;
	typename P::factor_type;
	typename P::result_type; 
};
template <typename P, typename PosType, typename VelType, typename Factor>
concept particle_calc = requires(PosType pos, VelType vel, Factor ftr) {
	{ P::position_minus(pos, pos) } -> std::same_as<VelType>;
	P::position_plus(pos, vel);
	P::velocity_multiply(vel, ftr);
};

template <typename P>
concept is_particle = particle_represent<P> && 
						particle_calc<P, typename P::position_type, typename P::velocity_type,
	                                  typename P::factor_type>;

template <size_t D>
struct ALIGN_REQ canonical_particle
{
	static constexpr size_t Dimension = D;
	using position_type = std::vector<double>;
	using velocity_type = std::vector<double>;
	using factor_type = std::vector<double>;
	using result_type = double;

	static velocity_type position_minus(const position_type&, const position_type&);
	static void position_plus(position_type&, const velocity_type&);
};