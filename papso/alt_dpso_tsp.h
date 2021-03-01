#pragma once
#include <random>
#include <numeric>
#include <functional>
#include <cstdio>
#include "discrete_particle.h"
#include "tsp.h"

#define PRINT_FOR_TEST 1

class alt_dpso_tsp {
	using particle_t = basic_particle<double, std::pair<size_t, size_t>, int>;
	using value_type = typename particle_t::value_type;
	using velocity_type = typename particle_t::velocity_type;
	using position_type = typename particle_t::position_type;

	size_t swarm_sz_;
	size_t iteration_sz_;
	size_t neighborhood_sz_;

	int gbest_ = std::numeric_limits<int>::max();
	const particle_t* gbest_p_ = nullptr;
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

	// Return true if gbest is improved
	bool update_gbest(const particle_t& p) {
		int f = p.m_fitness.load();

		if (f < gbest_) {
			gbest_ = f;
			gbest_p_ = &p;
			return true;
		}
		else {
			return false;
		}
	}

	// Return true if particle improve itself
	bool evaluate(particle_t& p) {
		double fitness = object_func_(p.m_position);
		p.m_fitness.store(fitness);

		if (fitness < p.m_best_fitness) {
			p.m_best_fitness.store(fitness);
			p.m_best_position = p.m_position;
			return true;
		}
		else {
			return false;
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
			update_gbest(p);
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

	// Fallback tatics trying to improve gbest
	bool rehope_lazy_descent_method() {
		size_t max_steps = std::max(1ull, dimension_ / 1);
		bool gbest_improved = false;

		for (size_t i = 0; i < swarm_sz_; ++i) {
			particle_t& p = particles_[i];

			// Go back to personal best
			p.m_position = p.m_best_position;

			// Try to move it 
			// to a better position within given steps
			for (int j = 0; j < max_steps; ++j) {
				// Move slowly by |v| == 1

				// Generate random velocity 
				size_t first = rng_() % dimension_;
				size_t second = rng_() % dimension_;
				while (first == second) {
					second = rng_() % dimension_;
				}
				velocity_type rand_vel = { {first, second} };

				// Update position
				// Go back to personal best and move
				p.m_position = pos_plus(std::move(p.m_position), rand_vel);
				
				// Evaluate
				if (evaluate(p)) {
					gbest_improved = update_gbest(p);

					// Stop as soon as finds a better position
					break;
				}
			} // end of max steps

			if (gbest_improved) {
				return true;
			}
		}
		return false;
	}
	
	bool rehope_energetic_descent_method() {
		size_t max_steps = std::max(1ull, dimension_ / 1);
		bool gbest_improved = false;

		for (size_t i = 0; i < swarm_sz_; ++i) {
			particle_t& p = particles_[i];

			// Go back to personal best
			p.m_position = p.m_best_position;

			// Try to move it 
			// to a better position within given steps
			for (int j = 0; j < max_steps; ++j) {
				// Move slowly by |v| == 1

				// Generate random velocity 
				size_t first = rng_() % dimension_;
				size_t second = rng_() % dimension_;
				while (first == second) {
					second = rng_() % dimension_;
				}
				velocity_type rand_vel = { {first, second} };

				// Update position
				p.m_position = pos_plus(std::move(p.m_position), rand_vel);

				// Evaluate				
				if (evaluate(p)) {
					gbest_improved = update_gbest(p);

					// Move as long as it finds better position 
					// by reset steps
					j = 0;
				}
			} // end of max steps

			if (gbest_improved) {
				return true;
			}
		}
		return false;
	}

	bool rehope_local_iterative_levelling() {
		bool gbest_improved = false;

		for (size_t i = 0; i < swarm_sz_; ++i) {
			// For each particle, find the best position  
			// at 1 distance away from it
			particle_t& p = particles_[i];

			// Find the best from every position in 1 distance
			double best_f = std::numeric_limits<double>::max();
			velocity_type best_vel(1); // 1 distance

			// Every position
			for (size_t first = 0; first < dimension_ - 1; ++first) {
				for (size_t second = first + 1; second < dimension_; ++second) {
					velocity_type vel = { { first, second } };
					position_type pos = pos_plus(p.m_position, vel);
					
					double f = object_func_(pos);

					if (f < best_f) {
						best_f = f;
						best_vel = vel;
					}
				} // end of second
			} // end of first 		

			// Evaluation
			if (best_f < p.m_best_fitness.load()) {
				p.m_position = pos_plus(std::move(p.m_position), best_vel);
				if (evaluate(p) && update_gbest(p)) {
					return true;
				}
			}
		}
		return false;
	}

public:
	alt_dpso_tsp(size_t ssz = 32, size_t isz = 200, size_t nbsz = 4) :
		swarm_sz_(ssz)
		, iteration_sz_(isz)
		, neighborhood_sz_(nbsz)
		, particles_(swarm_sz_) { }
	
	int evolve(const coord* map, size_t sz, position_type& best_pos) {
		dimension_ = sz;
		object_func_ = std::bind(tsp, std::placeholders::_1, map);
		initialize_swarm();

#if PRINT_FOR_TEST
		FILE* fp = nullptr;
		if (fopen_s(&fp, "dpso_tsp_log.txt", "a") != 0
			|| !fp) {
			throw std::exception{ "can't open log file!\n" };
		}
		fprintf(fp, "\n\niteration\tbest results\tgbest\n");
#endif		
		size_t LDM_thresh = 2;				// no rehope: [0, 1]
											// ldm: [2, 3]
		size_t EDM_thresh = LDM_thresh + 2; // edm: [4, ...]		
		size_t last_improved = 0;		
		size_t i = 0; // For iteration
		auto update_thresh = [&](bool improved) {
			if (improved) {
				last_improved = i;
				LDM_thresh = i + 2;
				EDM_thresh = LDM_thresh + 2;
			}
		};

		for (; i < iteration_sz_; ++i) {
			for (size_t j = 0; j < swarm_sz_; ++j) {
				// Update velocity
				update_velocity(j);

				// Update position
				update_position(j);
			} // end of update loop
			
			// Update fitness
#if PRINT_FOR_TEST
			int best_of_iteration = std::numeric_limits<int>::max();
#endif
			bool improved = false;
			for (auto& p : particles_) {
				evaluate(p);
				improved = update_gbest(p);

#if PRINT_FOR_TEST
				int f = p.m_best_fitness.load();
				if (f < best_of_iteration) {
					best_of_iteration = f;
				}
#endif
			} // end of evaluation loop

			if (i < LDM_thresh) {
				// No rehope
			}
			else if (i < EDM_thresh) { // LDM	
				improved = rehope_lazy_descent_method();				
			}	
			else if (EDM_thresh <= i) { // EDM
				improved = rehope_energetic_descent_method();
			}
			else { // LIL				
				//improved = rehope_local_iterative_levelling();				
			}
			
			update_thresh(improved);

#if PRINT_FOR_TEST
			const auto& p0 = particles_[0];
			printf("%llu\nfitness: %lf, best fitness:%lf\n", i, p0.m_fitness.load(), p0.m_best_fitness.load());
			printf("\nposition:\n");
			for (int i : p0.m_position) {
				printf("%d, ", i);
			}
			printf("\n\n\n\n");

			fprintf(fp, "%llu\t\t%d\t%d\n", i, best_of_iteration, gbest_);
#endif
		}

#if PRINT_FOR_TEST
		fprintf(fp, "best position:\n");
		auto& gbest = *gbest_p_;
		for (size_t i = 0; i < gbest.m_best_position.size(); ++i) {
			fprintf(fp, "%d, ", gbest.m_best_position[i]);
		}
		fprintf(fp, "\n");
		fclose(fp);
#endif

		best_pos = gbest_p_->m_best_position;
		return std::exchange(gbest_, std::numeric_limits<int>::max());
	}
};