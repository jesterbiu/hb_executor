#pragma once
#include <random>
#include <numeric>
#include <functional>
#include "discrete_particle.h"
#include "tsp.h"

class alt_dpso_tsp {
	using particle_t = discrete_particle;
	using value_type = typename particle_t::value_type;
	using velocity_type = typename particle_t::velocity_type;
	using position_type = typename particle_t::position_type;

	size_t swarm_sz_;
	size_t iteration_sz_;
	size_t neighborhood_sz_;

	int gbest_ = std::numeric_limits<int>::max();
	std::vector<particle_t> particles_ = {};
	std::default_random_engine rng_ = {};
	size_t dimension_ = 0;

	dpso_position_minus_perm		pos_minus = {};
	dpso_position_plus_perm			pos_plus = {};
	dpso_velocity_multiplies_perm	vel_multiplies = {};
	dpso_velocity_plus_perm			vel_plus = {};

	std::function<double(const position_type&)> object_func_ = {};

	position_type random_pos(std::default_random_engine& r) {
		position_type pos(dimension_);
		std::iota(pos.begin(), pos.end(), 0);
		std::shuffle(pos.begin(), pos.end(), r);
		return pos;
	};

	void update_gbest(int f) {
		if (f < gbest_) {
			gbest_ = f;
		}
	}

	int evaluate(particle_t& p) {
		int fitness = static_cast<int>(object_func_(p.m_position));

		if (fitness < p.m_best_fitness) {
			p.m_best_fitness.store(fitness);
			update_gbest(fitness);
		}
	}

	void initialize_swarm() {
		// Initialize every particle
		for (auto& p : particles_) {
			// Init position to a random pos
			p.m_position = random_pos(rng_);

			// Init best pos to current initial pos
			p.m_best_position = p.m_position;

			// Init velocity
			position_type rand_pos = random_pos(rng_);
			p.m_velocity = pos_minus(p.m_position, rand_pos);

			// Init fitness
			evaluate(p);
		}
	}

	const position_type& best_neighbor(size_t idx) const {
		size_t half = neighborhood_sz_ / 2;
		size_t first_neighbor = 0;
		if (idx >= half) {
			first_neighbor = idx - half;
		}
		else {
			first_neighbor = swarm_sz_ - (half - idx);
		}

		int best = particles_[first_neighbor].m_best_fitness.load();
		size_t best_idx = first_neighbor;
		for (size_t i = first_neighbor + 1; 
			i <= first_neighbor + neighborhood_sz_; ++i) {
			
			if (i == idx) {
				continue;
			}

			int fitness = particles_[i % swarm_sz_].m_best_fitness.load();
			if (fitness < best) {
				best = fitness;
				best_idx = i % swarm_sz_;
			}
		}

		return particles_[best_idx].m_best_position;
	}

	void update_velocity(size_t idx) {	
		particle_t& p = particles_[idx];

		// v(t+1) = c1 * v(t) + c2(pbest - x(t)) + c2(gbest - x(t))

		// c1 * v(t)
		static constexpr double c1 = 0.999;
		velocity_type prev_vel = vel_multiplies(c1, std::move(p.m_velocity));

		// 0 <= c2 < 2
		double c2 = 2 * std::generate_canonical<double, 16>(rng_);
		
		// c2(pbest - x(t))
		velocity_type pbest_vel = vel_multiplies(
			c2, 
			pos_minus(p.m_best_position, p.m_position));

		// c2(gbest - x(t))
		auto& hood_best = best_neighbor(idx);
		velocity_type hoodbest_vel = vel_multiplies(
			c2,
			pos_minus(hood_best, p.m_position));

		p.m_velocity = vel_plus(prev_vel, vel_plus(pbest_vel, hoodbest_vel));
	}

	void update_position(size_t idx) {
		particle_t& p = particles_[idx];
		p.m_position = pos_plus(std::move(p.m_position), p.m_velocity);
	}

public:
	alt_dpso_tsp(size_t ssz = 32, size_t isz = 200, size_t nbsz = 4) :
		swarm_sz_(ssz)
		, iteration_sz_(isz)
		, neighborhood_sz_(nbsz)
		, particles_(swarm_sz_) { }
	
	int evolve(const coord* map, size_t sz) {
		dimension_ = sz;
		object_func_ = std::bind(tsp, std::placeholders::_1, map);
		initialize_swarm();

		for (size_t i = 0; i < iteration_sz_; ++i) {
			for (size_t j = 0; j < swarm_sz_; ++j) {
				// Update velocity
				update_velocity(j);

				// Update position
				update_position(j);
			}			

			// Update fitness
			for (auto& p : particles_) {
				evaluate(p);
			}
		}
	}
};