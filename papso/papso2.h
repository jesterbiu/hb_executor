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
std::tuple<double, vec_t>
apso(const func_t f, double min, double max, size_t dim = 30) {
	using particle = my_particle;

	static constexpr auto swarm_size = 40u;
	canonical_rng rng;
	std::vector<particle> particles;
	particles.resize(swarm_size);

	// Initialize
	auto random_xi = [&]() {
		return min + rng() * (max - min);
	};
	for (particle& p : particles) {
		p.position.resize(dim);
		p.best_position.resize(dim);
		p.velocity.resize(dim);

		for (size_t i = 0; i < dim; ++i) {
			p.position[i] = random_xi();
			p.best_position[i] = p.position[i];
			p.velocity[i] = (random_xi() - p.position[i]) / 2.0;
		}

		p.value = f(p.position.cbegin(), p.position.cend());
		p.best_value = p.value;
	}

	// Update
	static constexpr double INERTIA = 0.7298;
	static constexpr double ACCELERATOR = 1.49618;
	auto calculate_velocity = [&](double vi, double xi, double pbest, double lbest) {
		return INERTIA * vi
			+ ACCELERATOR * rng() * (pbest - xi)
			+ ACCELERATOR * rng() * (lbest - xi);
	};

	static constexpr auto iteration = 1000u;
	static constexpr auto neighbor_size = 12u; 
	particle* gbest = &particles[0];
	for (size_t i = 0; i < iteration; ++i) {
		for (size_t j = 0; j < swarm_size; ++j) { // The order of evaluation & update!
			particle& p = particles[j];

			// Lbest
			const particle* lbest_ptr = &p; // !!Middle of neighbor
			int underflow = j - neighbor_size / 2;
			size_t first = (underflow > 0) ? underflow : underflow + swarm_size;
			size_t last = first + neighbor_size;
			first %= swarm_size;
			last %= swarm_size;
			for (size_t neighbor = first; neighbor <= last; ++neighbor) {
				if (particles[neighbor].best_value < lbest_ptr->best_value) {
					lbest_ptr = &particles[neighbor];
				}
			}
			const vec_t& lbest = lbest_ptr->best_position;

			// Update
			for (size_t d = 0; d < dim; ++d) {
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
			} // end of dimension


			// Evaluate
			p.value = f(p.position.cbegin(), p.position.cend());
			if (p.value < p.best_value) {
				p.best_value = p.value;
				p.best_position = p.position;

				if (p.best_value < gbest->best_value) {
					gbest = &p;
				}
			}
		} // end of particle

		if ((i+1) % 100 == 0) {
			printf("%lf ", gbest->best_value);
		}

	} // end of iteration

	return { gbest->best_value
		, std::move(gbest->best_position) };
}