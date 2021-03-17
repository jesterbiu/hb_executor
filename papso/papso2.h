#pragma once
#include <vector>
#include <tuple>
#include <atomic>
#include <memory>
#include <iostream>
#include "particle.h"
#include "canonical_rng.h"

using vec_t = std::vector<double>;

// particle
struct my_particle {
	double value;
	double best_value;
	vec_t velocity;
	vec_t position;
	vec_t best_position;
};

using iter = vec_t::const_iterator;
using func_t = double(*)(iter, iter);

class pso_state {
	static constexpr auto swarm_size = 40u;
	static constexpr auto iteration = 500u;
	static constexpr auto neighbor_size = 4u;

	using particle = my_particle; 
	const func_t f;
	size_t dimension;
	double min, max;
	particle* gbest = nullptr;
	std::vector<particle> particles;
	canonical_rng rng;

public:
	pso_state(const func_t f, size_t dim, double min, double max) :
		f(f),
		dimension(dim), min(min), max(max) {}
	pso_state(const pso_state&) = delete;

	void initialize_state() {
		particles.resize(swarm_size);

		for (particle& p : particles) {
			p.position.resize(dimension);
			p.best_position.resize(dimension);
			p.velocity.resize(dimension);
		}
	}
	void initialize_swarm() { // rng() may throw?
		auto random_xi = [&]() {
			return min + rng() * (max - min);
		};

		for (particle& p : particles) {
			for (size_t i = 0; i < dimension; ++i) {
				p.position[i] = random_xi();
				p.best_position[i] = p.position[i];
				p.velocity[i] = (random_xi() - p.position[i]) / 2.0;
			}

			p.value = std::numeric_limits<double>::max();//f(p.position.cbegin(), p.position.cend());
			p.best_value = p.value;
		}
	}
	auto calculate_velocity (double vi, double xi, double pbest, double lbest) { // rng may throw?
		static constexpr double INERTIA = 0.7298;
		static constexpr double ACCELERATOR = 1.49618;

		return INERTIA * vi
			+ ACCELERATOR * rng() * (pbest - xi)
			+ ACCELERATOR * rng() * (lbest - xi);
	}
	particle& update_gbest() noexcept {
		auto tmp = &particles.front();
		for (auto& p : particles) {
			if (p.best_value < tmp->best_value) {
				tmp = &p;
			}
		}
		gbest = tmp;
		return *gbest;
	}

	void evaluate_particle(particle& p) noexcept {
		// Evaluate
		p.value = f(p.position.cbegin(), p.position.cend());
		if (p.value < p.best_value) {
			p.best_value = p.value;
			p.best_position = p.position;
		}
	}

	const vec_t& get_lbest_unsafe(int idx) const noexcept {
		const particle* lbest_ptr = &particles[idx]; // !!Middle of neighbor
		const int max_offset = neighbor_size / 2; // Always positive
		// offset: [-max_offset, +max_offset]
		for (int offset = -max_offset; offset <= max_offset; ++offset) {
			size_t neighbor = (idx + swarm_size + offset) % swarm_size; // in case (idx + offset) < 0
			if (particles[neighbor].best_value < lbest_ptr->best_value) {
				lbest_ptr = &particles[neighbor];
			}
		}
		return lbest_ptr->best_position;
	}

	void move_particle(particle& p, const vec_t& lbest) {		
		for (size_t d = 0; d < dimension; ++d) {
			double& vi = p.velocity[d];
			double& xi = p.position[d];
			vi = calculate_velocity(vi, xi, p.best_position[d], lbest[d]);
			xi += vi;

			// Confinement
			if (xi < min) {
				xi = min;
				vi = 0;
			}
			else if (xi > max) {
				xi = max;
				vi = 0;
			}
			else {
				// nothing happens
			}
		}
	}

	void pso_main_loop() {
		for (size_t i = 0; i < iteration; ++i) {
			for (size_t j = 0; j < swarm_size; ++j) { // The order of evaluation & update!
				particle& p = particles[j];

				evaluate_particle(p);

				// Lbest				
				const vec_t& lbest = get_lbest_unsafe(j);

				// Update velocity, position				
				move_particle(p, lbest);

			} // end of particle

			if ((i + 1) % 100 == 0) {
				printf("%lf ", update_gbest().best_value);
			}
		} // end of iteration
	}
};

std::tuple<double, vec_t>
apso(const func_t f, double min, double max, size_t dim = 30) {
	auto pso_state_uptr = std::make_unique<pso_state>(f, dim, min, max);
	auto& state = *pso_state_uptr;

	// Initialize
	state.initialize_state();
	state.initialize_swarm();

	// Update
	state.pso_main_loop();

	// Get result
	auto& gbest = state.update_gbest();
	return { gbest.best_value, std::move(gbest.best_position) };
}