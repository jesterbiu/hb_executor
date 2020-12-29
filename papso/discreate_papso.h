#pragma once 
#include <random>
#include <vector>
#include <mutex>
#include <algorithm>
#include <numeric>
#include "discrete_particle.h"
#include "canonical_rng.h"
#include "../hb_executor/executor.h"

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
	using particle_t = discrete_particle<Dimension>;
	template <typename T>
	using cont_t = particle_t::cont_t;
	using value_type = particle_t::value_type;
	using velocity_type = particle_t::velocity_type;
	using position_type = particle_t::position_type;
	static constexpr size_t Iteration = 10000u;
	static constexpr size_t NeighborSize = 12u;
	static constexpr size_t SwarmSize = 40u;

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
		assert(m_Xmin + Dimension <= m_Xmax);

		cont_t<value_type> pos(Dimension);
		std::iota(pos.begin(), pos.end(), m_Xmin);
		std::shuffle(pos.begin(), pos.end(), g);
		return pos;
	};
	void initialize_particles()
	{		
		particle_t* global_best_particle = &m_particles[0];
		for (auto& p : m_particles) {
			// Init position to random pos
			p.m_position = random_pos(m_Xmin, m_Xmax);

			// Init best pos
			p.m_best_position = p.m_position;

			// Init vel: this formula doesn't use velocity
			// p.m_velocity = m_pos_minus(random_pos(m_Xmin, m_Xmax), p.m_posiiton);

			// Init fitness
			const auto fitness = m_object_function(p.position.data(), Dimension);
			p.fitness.store(fitness);
			p.best_fitness.store(fitness);

			// Init global fitness
			update_global_best(p);
		}
	}
	void update_global_best(particle_t& p)
	{
		std::lock_guard<std::mutex> lk{ m_mtx };
		auto gbest_particle = m_gbest_particle.load();
		if (!gbest_particle) {
			m_gbest_particle.store(&p);
			return;
		}

		const auto pbest_fitness = gbest_particle->best_fitness.load();
		const auto gbest_fitness = gbest_particle->best_fitness.load();
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

		cont_t<velocity_type> calculate_velocity(
			const cont_t<position_type>& pos_dst,
			const cont_t<position_type>& pos_src, 
			double acc) 
		{
			auto vel = m_pos_minus(pbest, pos);
			m_vel_multiplies(vel, acc* m_rng());
		};
		std::vector<double> best_neighbor_position(size_t idx)
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
				if (p_neighbor->best_fitness.load() < p_best_neighbor->best_fitness.load()) {
					p_best_neighbor = p_neighbor;
				}
			}

			// Copy
			std::lock_guard<std::mutex> lg{ p_best_neighbor->mtx };
			auto& pos = p_best_neighbor->best_position;
			return std::vector<double>(pos.cbegin(), pos.cend());
		}
		void update_velocity(particle_t& p, const std::vector<double>& nbest)
		{
			static_assert(false, "This formula doesn't use velocity!");
			for (size_t i = 0; i < Dimension; ++i) {
				p.velocity[i]
					= calculate_velocity(p.velocity[i], p.position[i], p.best_position[i], nbest[i]);
			}
		}
		void update_position(particle_t& p) // clamp pos
		{
			static constexpr double ACCELERATOR = 1.49618;

			// Destination to personal best
			const auto vel_pbest = calculate_velocity(pbest, pos, ACCELERATOR);
			const auto dst_pbest = m_pos_plus(pos, vel_pbest);

			// Destination to global best
			const auto vel_gbest = calculate_velocity(gbest, pos, ACCELERATOR);
			const auto dst_gbest = m_pos_plus(pos, vel_gbest);

			// Random velocity
			const auto vel_rand = calculate_velocity(m_parent->random_pos(), pos, m_rng());

			// Update position
			auto vel_midpoint = m_pos_minus(dst_pbest, dst_gbest);
			m_vel_multiplies(m_rng() * m_rng(), vel_midpoint);
		}
		void evaluate_particle(particle_t& p) // may update global
		{
			const auto fitness = m_parent->m_object_function(p.position.data(), Dimension);
			p.fitness.store(fitness);
			if (fitness < p.best_fitness.load()) {
				// Update personal best
				std::lock_guard<std::mutex> lg{ p.mtx };
				p.best_fitness.store(p.fitness);
				for (size_t i = 0; i < Dimension; ++i) {
					p.best_position[i] = p.position[i];
				}

				// Update global best
				m_parent->update_global_best(p);
			}
		}
		void operator()()
		{
			for (size_t itr = 0; itr < Iteration; ++itr) {
				// Check result so far
				auto gbest = m_parent->m_gbest_particle.load();
				if (gbest->best_fitness.load() <= m_parent->m_goal) return;

				for (size_t idx = 0; idx < SwarmSize; ++idx) {
					auto& p = m_ptr_particles->operator[](idx);
					auto best_neighbor = best_neighbor_position(idx);
					update_velocity(p, best_neighbor);
					update_position(p);
					evaluate_particle(p);
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

	bool operator()(double best_fitness, std::vector<double>& best_position)
	{
		initialize_particles();
		par_async_dpso_sub{ *this, 0, SwarmSize }();
		auto& p = *m_gbest_particle.load();
		best_fitness = p.m_best_fitness.load();
		best_position.assign(p.m_best_position.cbegin(), p.m_best_position.cend());
		return  p.m_best_fitness.load() <= m_goal ?
			true :
			false;
	}

};