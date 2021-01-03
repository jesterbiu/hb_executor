#pragma once 
#include <random>
#include <vector>
#include <mutex>
#include <algorithm>
#include <numeric>
#include <cstdio>
#include "discrete_particle.h"
#include "canonical_rng.h"

/*
* A parallel async discrete PSO implementation for sovling 
* Travling Sales man problem
* 
*
* Hoffmann, M., M¨¹hlenthaler, M., Helwig, S., & Wanka, R. \
* Discrete particle swarm optimization for TSP: theoretical results and experimental evaluations. \
* http://wwwmayr.informatik.tu-muenchen.de/konferenzen/Ferienakademie14/literature/HMHW11.pdf
*/
template <size_t Dimension, typename F>
struct par_async_dpso
{
	using particle_t	= discrete_particle<Dimension>;
	using value_type	= typename particle_t::value_type;
	using velocity_type = typename particle_t::velocity_type;
	using position_type = typename particle_t::position_type;
	static constexpr size_t Iteration = 1000u;
	static constexpr size_t NeighborSize = 12u;
	static constexpr size_t SwarmSize = 80u;

	F m_object_function;
	const value_type m_goal;
	const value_type m_Xmin; 
	const value_type m_Xmax;
	dpso_position_minus_perm<Dimension>		    m_pos_minus;
	dpso_velocity_multiplies_perm<Dimension>	m_vel_multiplies;
	dpso_position_plus_perm<Dimension>			m_pos_plus;
	canonical_rng m_rng;
	std::vector<particle_t> m_particles;
	std::mutex m_mtx;
	std::atomic<particle_t*> m_gbest_particle{ nullptr };

	// Generate a random permutation of sequence [min, max)
	auto random_pos () 
	{
		static std::default_random_engine g(std::random_device{}());
		assert(m_Xmin + Dimension <= m_Xmax + 1);

		vec_t<value_type> pos(Dimension);
		std::iota(pos.begin(), pos.end(), m_Xmin);
		std::shuffle(pos.begin(), pos.end(), g);
		return pos;
	};
	void initialize_particles()
	{		
		particle_t* global_best_particle = &m_particles[0];
		for (auto& p : m_particles) {
			// Init position to random pos
			p.m_position = random_pos();

			// Init best pos
			p.m_best_position = p.m_position;

			// Init vel: this formula doesn't use velocity
			// p.m_velocity = m_pos_minus(random_pos(m_Xmin, m_Xmax), p.m_posiiton);

			// Init fitness
			const value_type fitness = m_object_function(p.m_position.data(), Dimension);
			p.m_fitness.store(fitness);
			p.m_best_fitness.store(fitness);

			// Init global fitness
			update_global_best(p);
		}
	}
	void update_global_best(particle_t& p)
	{
		std::lock_guard lk{ m_mtx };
		particle_t* gbest_particle = m_gbest_particle.load();
		if (!gbest_particle) {
			m_gbest_particle.store(&p);
			return;
		}

		const auto pbest_fitness = p.m_best_fitness.load();
		const auto gbest_fitness = gbest_particle->m_best_fitness.load();
		if (pbest_fitness < gbest_fitness) {
			m_gbest_particle.store(&p);
		}
	}

	struct par_async_dpso_sub
	{
		par_async_dpso* m_parent;
		size_t m_beg, m_end;
		std::vector<particle_t>* m_ptr_particles;
		canonical_rng m_rng;
		dpso_position_minus_perm<Dimension>		    m_pos_minus;
		dpso_velocity_multiplies_perm<Dimension>	m_vel_multiplies;
		dpso_position_plus_perm<Dimension>			m_pos_plus;

		par_async_dpso_sub(par_async_dpso& parent, size_t beg, size_t end) :
			m_parent(&parent), m_beg(beg), m_end(end),
			m_ptr_particles(&parent.m_particles), m_rng() {}
		par_async_dpso_sub(const par_async_dpso_sub&) = default;
		par_async_dpso_sub& operator=(const par_async_dpso_sub&) = default;
		~par_async_dpso_sub() {}

		// Calculate velocity with accelerator
		vec_t<velocity_type> calculate_velocity(const vec_t<position_type>& pos_dst,
												const vec_t<position_type>& pos_src) 
		{
			return m_vel_multiplies(m_rng(), m_pos_minus(pos_dst, pos_src));
		};
		vec_t<position_type> best_neighbor_position(size_t idx)
		{
			// Find best
			particle_t* p_neighbor = nullptr;
			particle_t* p_best_neighbor = &(m_ptr_particles->at(idx));
			int underflow = idx - NeighborSize / 2;
			const size_t first_neighbor = (underflow > 0) ? underflow : underflow + SwarmSize;
			const size_t last_neighbor = first_neighbor + NeighborSize;

			for (size_t i = first_neighbor; i != last_neighbor; ++i) {
				if (i == idx) continue;

				p_neighbor = &m_ptr_particles->at(i % SwarmSize);
				if (p_neighbor->m_best_fitness.load() < p_best_neighbor->m_best_fitness.load()) {
					p_best_neighbor = p_neighbor;
				}
			}

			// Copy
			std::lock_guard lg{ p_best_neighbor->m_mtx };
			auto& pos = p_best_neighbor->m_best_position;
			return { pos.cbegin(), pos.cend() };
		}
		void update_velocity(particle_t& p, const std::vector<double>& nbest)
		{
			static_assert(false, "This formula doesn't use velocity!");
		}
		void update_position(particle_t& p, const vec_t<position_type>& gbest) // clamp pos
		{
			// Destination to personal best
			const auto vel_pbest = calculate_velocity(p.m_best_position, p.m_position);
			const auto dst_pbest = m_pos_plus(p.m_position, vel_pbest);

			// Destination to global best
			const auto vel_gbest = calculate_velocity(gbest, p.m_position);
			const auto dst_gbest = m_pos_plus(p.m_position, vel_gbest);

			// Random velocity
			const auto vel_rand = calculate_velocity(m_parent->random_pos(), p.m_position);

			// Velocity to mid point
			auto vel_midpoint = m_vel_multiplies(0.5, m_pos_minus(dst_pbest, dst_gbest));

			// Update position
			p.m_position = m_pos_plus(m_pos_plus(p.m_position, vel_midpoint), 
									  vel_midpoint);
		}
		void evaluate_particle(particle_t& p) // may update global
		{
			const auto fitness = m_parent->m_object_function(p.m_position.data(), Dimension);
			p.m_fitness.store(fitness);
			if (fitness < p.m_best_fitness.load()) {
				// Update personal best
				std::lock_guard lg{ p.m_mtx };
				p.m_best_fitness.store(fitness);
				p.m_best_position = p.m_position;

				// Update global best
				m_parent->update_global_best(p);
			}
		}
		void operator()()
		{
			for (size_t itr = 0; itr < Iteration; ++itr) {
				// Check result so far
				particle_t* gbest = m_parent->m_gbest_particle.load();
				if (gbest->m_best_fitness.load() <= m_parent->m_goal) return;
				printf_s("\n(current best: %d)\n", gbest->m_best_fitness.load());

				for (size_t idx = 0; idx < SwarmSize; ++idx) {
					particle_t& p = (*m_ptr_particles)[idx];
					const auto gbest = best_neighbor_position(idx);
					update_position(p, gbest);
					evaluate_particle(p);
					printf_s("%lu[%lu]: %d\n", itr, idx, p.m_fitness.load());
				} // end of per-particle loop

				
			} // end of per-iteration loop

		}
	};

	template <typename Func>
	par_async_dpso(Func&& f, value_type goal, value_type Xmin, value_type Xmax) :
		m_object_function(std::forward<std::decay_t<Func>>(f)), 
		m_goal(goal), m_Xmin(Xmin), m_Xmax(Xmax),
		m_particles(SwarmSize) {}
	par_async_dpso(const par_async_dpso&) = delete;

	bool operator()(double& best_fitness, std::vector<int>& best_position)
	{
		initialize_particles();
		par_async_dpso_sub{ *this, 0, SwarmSize }();
		particle_t& p = *m_gbest_particle.load();
		best_fitness = p.m_best_fitness.load();
		best_position.assign(p.m_best_position.cbegin(), p.m_best_position.cend());
		return  p.m_best_fitness.load() <= m_goal ?
			true :
			false;
	}

};