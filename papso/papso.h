/*
* A Parallel Asynchronous Standard PSO Implementations
*/
#pragma once
#include <random>
#include <vector>
#include <mutex>
#include "particle.h"
#include "canonical_rng.h"
#include "../hb_executor/executor.h"

template <
	size_t Dimension, 
	size_t Iteration = 10000u,	
	size_t NeighborSize = 12u, 
	size_t SwarmSize = 40u
>
struct par_async_dpso
{
	using particle_t = particle<Dimension>;	

	double(*m_object_function)(const double*, size_t);
	const double m_goal;
	const double m_Xmin; const double m_Xmax;
	canonical_rng m_rng;
	std::vector<particle_t> m_particles;
	std::mutex m_mtx;
	std::atomic<particle_t*> m_gbest_particle{ nullptr };	

	void initialize_particles()
	{
		auto random_position = [this](double min, double max) {
			return min + this->m_rng() * (max - min);
		};

		particle_t* global_best_particle = &m_particles[0];
		for (auto& p : m_particles) {
			for (size_t i = 0; i < Dimension; ++i) {
				// Init position to random pos
				p.position[i] = random_position(m_Xmin, m_Xmax);

				// Init best position to initial pos
				p.best_position[i] = p.position[i];

				// Init velocity
				p.velocity[i] = (random_position(m_Xmin, m_Xmax) - p.position[i]) / 2.;
			}

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

		par_async_dpso_sub(par_async_dpso& parent, size_t beg, size_t end) :
			m_parent(&parent), m_beg(beg), m_end(end),
			m_ptr_particles(&parent.m_particles), m_rng() {}
		par_async_dpso_sub(const par_async_dpso_sub&) = default;
		par_async_dpso_sub& operator=(const par_async_dpso_sub&) = default;
		~par_async_dpso_sub() {}

		double calculate_velocity(double vel, double x, double pbest_x, double nbest_x) {
			static constexpr double INERTIA = 0.7298;
			static constexpr double ACCELERATOR = 1.49618;
			return INERTIA * vel
				+ ACCELERATOR * m_rng.operator()() * (pbest_x - x)
				+ ACCELERATOR * m_rng.operator()() * (nbest_x - x);
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
			for (size_t i = 0; i < Dimension; ++i) {
				p.velocity[i]
					= calculate_velocity(p.velocity[i], p.position[i], p.best_position[i], nbest[i]);
			}
		}
		void update_position(particle_t& p) // clamp pos
		{
			const auto Xmin = m_parent->m_Xmin;
			const auto Xmax = m_parent->m_Xmax;
			for (size_t i = 0; i < Dimension; ++i) {
				p.position[i] += p.velocity[i];

				// Clamp position
				if (p.position[i] < Xmin) {
					p.position[i] = Xmin;
					p.velocity[i] = 0;
				}
				else if (p.position[i] > Xmax) {
					p.position[i] = Xmax;
					p.velocity[i] = 0;
				}
				else;
			} // end of for loop			
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
	
	par_async_dpso(double(*object_function)(const double*, size_t), double goal, double Xmin, double Xmax) :
		m_object_function(object_function), m_goal(goal), m_Xmin(Xmin), m_Xmax(Xmax),
		m_particles(SwarmSize){}
	bool operator()(double best_fitness, std::vector<double>& best_position)
	{
		initialize_particles();
		par_async_dpso_sub{ *this, 0, SwarmSize }();
		auto& p = *m_gbest_particle.load();
		best_fitness = p.best_fitness.load();
		best_position.assign(p.best_position.cbegin(), p.best_position.cend());
		return  p.best_fitness.load() <= m_goal ?
				true :
				false;
	}

};

