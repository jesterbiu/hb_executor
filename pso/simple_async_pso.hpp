/*
* A Synchronous Standard PSO Implementation
*/

#pragma once
#include <random>
#include <vector>
#include "particle.hpp"
#include "canonical_rng.hpp"

template <size_t D>
std::tuple<bool, double, std::vector<double>>
simple_async_pso(double(*object_function)(const double*, size_t), double Xmin, double Xmax)
{
	auto random_position = [](double min, double max) {
		return min + canonical_rng() * (max - min);
	};
	static constexpr double INERTIA = 0.7298;
	static constexpr double ACCELERATOR = 1.49618;
	auto calculate_velocity = [&](double v, double x, double px, double nx) {
		return INERTIA * v
			+ ACCELERATOR * canonical_rng() * (px - x)
			+ ACCELERATOR * canonical_rng() * (nx - x);
	};

	// Initalize particles
	static constexpr auto SWARMSZ = 40u;
	std::vector<particle<D>> particles(SWARMSZ);
	particle<D>* global_best_particle = &particles[0];
	for (auto& p : particles) {
		for (size_t i = 0; i < D; ++i) {
			// Init position to random pos
			p.position[i] = random_position(Xmin, Xmax);

			// Init best position to initial pos
			p.best_position[i] = p.position[i];

			// Init velocity
			p.velocity[i] = (random_position(Xmin, Xmax) - p.position[i]) / 2.;
		}

		// Init fitness
		p.best_fitness = p.fitness = object_function(p.position.data(), D);

		// Init global fitness
		if (p.best_fitness <= global_best_particle->best_fitness) {
			global_best_particle = &p;
		}
	}

	// Main loop
	static constexpr auto ITERATION = 10000u;
	static constexpr auto GOAL = 1e-5;
	static constexpr auto NEIGHBORSZ = 18u;
	for (size_t i_itr = 0; i_itr < ITERATION; ++i_itr) {
		//// Check goal
		//if (global_best_particle->best_fitness <= GOAL) {
		//	break;
		//}

		// Update each particle
		for (size_t i_p = 0; i_p < SWARMSZ; ++i_p) {
			auto& p = particles[i_p];

			// Update best neighbor
			particle<D>* p_neighbor = nullptr;
			particle<D>* p_best_neighbor = &p;
			int underflow = i_p - NEIGHBORSZ / 2;
			const size_t first_neighbor = (underflow > 0) ? underflow : underflow + SWARMSZ;
			const size_t last_neighbor = first_neighbor + NEIGHBORSZ;
			// Check each neighbor to find out the best one
			for (size_t i_neig = first_neighbor; i_neig !=  last_neighbor; ++i_neig) {
				if (i_neig == i_p) continue;
				p_neighbor = &particles[i_neig % SWARMSZ];
				if (p_neighbor->best_fitness < p_best_neighbor->best_fitness) {
					p_best_neighbor = p_neighbor;
				}
			}

			// Update velocity and position, for each dimension
			for (size_t i_dim = 0; i_dim < D; ++i_dim) {
				p.velocity[i_dim] = calculate_velocity(p.velocity[i_dim],
					p.position[i_dim],
					p.best_position[i_dim],
					p_best_neighbor->best_position[i_dim]);

				p.position[i_dim] += p.velocity[i_dim];

				// Clamp position
				if (p.position[i_dim] < Xmin) {
					p.position[i_dim] = Xmin;
					p.velocity[i_dim] = 0;
				}
				else if (p.position[i_dim] > Xmax) {
					p.position[i_dim] = Xmax;
					p.velocity[i_dim] = 0;
				}
				else;
			} // end of velocity update loop

			// Update fitness
			p.fitness = object_function(p.position.data(), D);
			if (p.fitness < p.best_fitness) {
				// Update personal best
				p.best_fitness = p.fitness;
				for (size_t i = 0; i < D; ++i) p.best_position[i] = p.position[i];

				// Update global best
				if (p.best_fitness < global_best_particle->best_fitness) {
					global_best_particle = &p;
				}
			}
		} // end of particle update loop

	} // end of pso iteration loop

	bool success = global_best_particle->best_fitness <= GOAL;
	auto best_pos = std::vector<double>(global_best_particle->best_position.cbegin(),
		global_best_particle->best_position.cend());
	return std::make_tuple(success,
		global_best_particle->best_fitness,
		std::move(best_pos));
}

template <size_t D>
std::tuple<bool, double, std::vector<double>>
random_async_pso(double(*object_function)(const double*, size_t), double Xmin, double Xmax)
{
	auto random_position = [](double min, double max) {
		return min + canonical_rng() * (max - min);
	};
	static constexpr double INERTIA = 0.7298;
	static constexpr double ACCELERATOR = 1.49618;
	auto calculate_velocity = [&](double v, double x, double px, double nx) {
		return INERTIA * v
			+ ACCELERATOR * canonical_rng() * (px - x)
			+ ACCELERATOR * canonical_rng() * (nx - x);
	};

	// Initalize particles
	static constexpr auto SWARMSZ = 40u;
	std::vector<particle<D>> particles(SWARMSZ);
	particle<D>* global_best_particle = &particles[0];
	for (auto& p : particles) {
		for (size_t i = 0; i < D; ++i) {
			// Init position to random pos
			p.position[i] = random_position(Xmin, Xmax);

			// Init best position to initial pos
			p.best_position[i] = p.position[i];

			// Init velocity
			p.velocity[i] = (random_position(Xmin, Xmax) - p.position[i]) / 2.;
		}

		// Init fitness
		p.best_fitness = p.fitness = object_function(p.position.data(), D);

		// Init global fitness
		if (p.best_fitness <= global_best_particle->best_fitness) {
			global_best_particle = &p;
		}
	}

	// Main loop
	static constexpr auto ITERATION = 10000u;
	static constexpr auto GOAL = 1e-5;
	static constexpr auto NEIGHBORSZ = 16u;
	for (size_t i_itr = 0; i_itr < ITERATION; ++i_itr) {
		//// Check goal
		//if (global_best_particle->best_fitness <= GOAL) {
		//	break;
		//}

		// Update each particle
		for (size_t i_p = 0; i_p < SWARMSZ; ++i_p) {
			const size_t idx = canonical_rng() * SWARMSZ;
			auto& p = particles[idx];

			// Update best neighbor
			particle<D>* p_neighbor = nullptr;
			particle<D>* p_best_neighbor = &p;
			int underflow = i_p - NEIGHBORSZ / 2;
			const size_t first_neighbor = (underflow > 0) ? underflow : underflow + SWARMSZ;
			const size_t last_neighbor = first_neighbor + NEIGHBORSZ;
			// Check each neighbor to find out the best one
			for (size_t i_neig = first_neighbor; i_neig != last_neighbor; ++i_neig) {
				if (i_neig == i_p) continue;
				p_neighbor = &particles[i_neig % SWARMSZ];
				if (p_neighbor->best_fitness < p_best_neighbor->best_fitness) {
					p_best_neighbor = p_neighbor;
				}
			}

			// Update velocity and position, for each dimension
			for (size_t i_dim = 0; i_dim < D; ++i_dim) {
				p.velocity[i_dim] = calculate_velocity(p.velocity[i_dim],
					p.position[i_dim],
					p.best_position[i_dim],
					p_best_neighbor->best_position[i_dim]);

				p.position[i_dim] += p.velocity[i_dim];

				// Clamp position
				if (p.position[i_dim] < Xmin) {
					p.position[i_dim] = Xmin;
					p.velocity[i_dim] = 0;
				}
				else if (p.position[i_dim] > Xmax) {
					p.position[i_dim] = Xmax;
					p.velocity[i_dim] = 0;
				}
				else;
			} // end of velocity update loop

			// Update fitness
			p.fitness = object_function(p.position.data(), D);
			if (p.fitness < p.best_fitness) {
				// Update personal best
				p.best_fitness = p.fitness;
				for (size_t i = 0; i < D; ++i) p.best_position[i] = p.position[i];

				// Update global best
				if (p.best_fitness < global_best_particle->best_fitness) {
					global_best_particle = &p;
				}
			}
		} // end of particle update loop

	} // end of pso iteration loop

	bool success = global_best_particle->best_fitness <= GOAL;
	auto best_pos = std::vector<double>(global_best_particle->best_position.cbegin(),
		global_best_particle->best_position.cend());
	return std::make_tuple(success,
		global_best_particle->best_fitness,
		std::move(best_pos));
}