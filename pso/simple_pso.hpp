/*
* A Synchronous Standard PSO Implementation
*/

#pragma once
#include <random>
#include <vector>
#include "particle.hpp"
#include "canonical_rng.hpp"


// return (if goal is achieved, best result, best position)
template <size_t D>
std::tuple<bool, double, std::vector<double>> 
simple_pso(double(*object_function)(const double*, size_t), double Xmin, double Xmax)
{
	auto random_position = [](double min_x, double max_x) {
		return min_x + canonical_rng() * (max_x - min_x);
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
	static constexpr auto NEIGHBORSZ = 24u;
	for (size_t i = 0; i < ITERATION; ++i) {
		//// Check goal
		//if (global_best_particle->best_fitness <= GOAL) {
		//	break;
		//}
		
		// Update each particle
		for (size_t j = 0; j < SWARMSZ; ++j) {
			auto& p = particles[j];

			// Update best neighbor
			particle<D>* neighbor = nullptr;
			particle<D>* best_neighbor = &p;
			int underflow = j - NEIGHBORSZ / 2;
			size_t first_neighbor = ( underflow > 0)
									? underflow
									: underflow + SWARMSZ;
			for (size_t in = first_neighbor; in != first_neighbor + NEIGHBORSZ; ++in) {
				if (in == j) continue;
				neighbor = &particles[in % SWARMSZ];
				if (neighbor->best_fitness < best_neighbor->best_fitness) {
					best_neighbor = neighbor;
				}
			}

			// Update velocity, position
			for (size_t iv = 0; iv < D; ++iv) {
				p.velocity[iv] = calculate_velocity( p.velocity[iv],
													p.position[iv],
													p.best_position[iv],
													best_neighbor->best_position[iv] );

				p.position[iv] += p.velocity[iv];

				// Clamp position
				if (p.position[iv] < Xmin) {
					p.position[iv] = Xmin;
					p.velocity[iv] = 0;
				}
				else if (p.position[iv] > Xmax) {
					p.position[iv] = Xmax;
					p.velocity[iv] = 0;
				}
				else;
			} // end of velocity update loop
		} // end of particle update loop

		// Update fitness
		for (auto& p : particles) {			
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
		}
	} // end of pso iteration loop

	bool success = global_best_particle->best_fitness <= GOAL;
	auto best_pos = std::vector<double>( global_best_particle->best_position.cbegin(),
										 global_best_particle->best_position.cend() );
	return std::make_tuple( success,
							global_best_particle->best_fitness,
							std::move(best_pos) );
}