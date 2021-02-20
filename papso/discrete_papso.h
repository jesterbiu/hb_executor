#pragma once 
#include <random>
#include <vector>
#include <mutex>
#include <algorithm>
#include <numeric>
#include <memory>
#include <optional>
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

// Constraint F is an object function type for particle type P
template <
	typename P, 
	typename F>
concept is_object_function =
	is_particle<P> &&
	std::is_invocable_r_v<typename P::value_type, F, const typename P::position_type&>;

// Class of parallel discrete asynchronous PSO
template <
	typename F,
	typename P = discrete_particle,	
	typename PosMinus = dpso_position_minus_perm,
	typename VelMult  = dpso_velocity_multiplies_perm,
	typename PosPlus  = dpso_position_plus_perm>
requires 
	is_object_function<P, F> && 
	position_minus<P, PosMinus> && 
	velocity_multiplies<P, VelMult> &&
	position_plus<P, PosPlus>
class par_async_dpso
{
public:
	// Type alias
	using particle_t	= P;
	using value_type	= typename particle_t::value_type;
	using velocity_type = typename particle_t::velocity_type;
	using position_type = typename particle_t::position_type;	

private:
	// Nested classes
	
	// Class that contains state of the swarm
	struct dpso_state
	{
		// Data members
		size_t NeighborSize = 4u;
		size_t SwarmSize = 100u;
		size_t Iteration = 200u;
		const size_t Dimension;
		F m_func;
		const value_type m_goal;
		const value_type m_Xmin; 
		const value_type m_Xmax;
		PosMinus m_pos_minus;
		VelMult	 m_vel_multiplies;
		PosPlus	 m_pos_plus;
		std::vector<particle_t> m_particles;
		std::mutex m_mtx;
		std::atomic<particle_t*> m_gbest{ nullptr };

		position_type random_pos(std::default_random_engine& r)
		{
			position_type pos(Dimension);
			std::iota(pos.begin(), pos.end(), m_Xmin);
			std::shuffle(pos.begin(), pos.end(), r);
			return pos;
		};
		void update_global_best(particle_t& p)
		{
			std::lock_guard lk{ m_mtx };
			particle_t* gbest_particle = m_gbest.load();
			if (!gbest_particle) {
				m_gbest.store(&p);
				return;
			}

			const auto pbest_fitness = p.m_best_fitness.load();
			const auto gbest_fitness = gbest_particle->m_best_fitness.load();
			if (pbest_fitness < gbest_fitness) {
				m_gbest.store(&p);
			}
		}

		dpso_state(
			size_t dim,
			F&& obj_func, 
			value_type goal, value_type Xmin, value_type Xmax) :
			Dimension(dim),
			m_func(std::forward<F>(obj_func)),
			m_goal(goal),
			m_Xmin(Xmin), m_Xmax(Xmax),
			m_pos_minus(),
			m_vel_multiplies(),
			m_pos_plus(),
			m_particles() {}
	};
	
	// Class that splits the work of computation
	class par_async_dpso_sub
	{
		// Data members
		dpso_state* m_state;
		size_t m_beg, m_end;
		std::vector<particle_t>* m_particles;
		canonical_rng m_rng;
	
		//// Operator overloadings
		//// pos - pos -> vel
		//friend velocity_type operator-(const position_type& lhs, const position_type& rhs)
		//{
		//	return m_state->m_pos_minus(lhs, rhs);
		//}
		//
		//// pos + vel -> pos
		//friend position_type operator+(const position_type& pos, const velocity_type& vel)
		//{
		//	return m_state->m_pos_plus(pos, vel);
		//}
		//friend position_type operator+(position_type&& pos, const velocity_type& vel)
		//{
		//	return m_state->m_pos_plus(std::move(pos), vel);
		//}
		//
		//// real * vel -> vel
		//friend velocity_type operator*(double r, const velocity_type& vel)
		//{
		//	return m_state->m_vel_multiplies(r, vel);
		//}
		//friend velocity_type operator*(double r, velocity_type&& vel)
		//{
		//	return m_state->m_vel_multiplies(r, std::move(vel));
		//}
		
		// Member functions
		position_type best_neighbor_position(size_t idx)
		{
			// Find best
			particle_t* p_neighbor = nullptr;
			particle_t* p_best_neighbor = &(m_particles->at(idx));
			int underflow = idx - m_state->NeighborSize / 2;
			const size_t first_neighbor = (underflow > 0) ? underflow : underflow + m_state->SwarmSize;
			const size_t last_neighbor = first_neighbor + m_state->NeighborSize;

			for (size_t i = first_neighbor; i != last_neighbor; ++i) {
				if (i == idx) continue;

				p_neighbor = &m_particles->at(i % m_state->SwarmSize);
				if (p_neighbor->m_best_fitness.load() < p_best_neighbor->m_best_fitness.load()) {
					p_best_neighbor = p_neighbor;
				}
			}

			// Copy
			std::lock_guard lg{ p_best_neighbor->m_mtx };
			auto& pos = p_best_neighbor->m_best_position;
			return { pos.cbegin(), pos.cend() };
		}
		void update_position(particle_t& p, const position_type& gbest) // clamp pos
		{
			const position_type& pos = p.m_position;
			const position_type& pbest = p.m_best_position;
			const auto& plus = m_state->m_pos_plus;
			const auto& minus = m_state->m_pos_minus;
			const auto& multiplies = m_state->m_vel_multiplies;

			// Destination to personal best
			// dest_pbest = pos + r * (pbest - pos)

			/*const auto vel_pbest = calculate_velocity(p.m_best_position, p.m_position);
			const auto dst_pbest = m_pos_plus(p.m_position, vel_pbest);*/
			const position_type dest_pbest = plus(pos, multiplies(m_rng(), 
																 minus(pbest, pos)));

			// Destination to global best
			// dest_gbest = pos + r * (gbest - pos)
			const position_type dest_gbest = plus(pos, multiplies(m_rng(), 
																 minus(gbest, pos)));

			// Random velocity
			// rand_vel = r * (rand_pos - pos);
			const velocity_type vel_rand = multiplies(m_rng(), minus(m_state->random_pos(m_rng.generator_), 
																	 pos));

			// Velocity to mid point
			// vel_mid = 0.5 * (dest_pbest - dest_gbest)
			const velocity_type vel_midpoint = multiplies(0.5, minus(dest_pbest, dest_gbest));

			// Update position
			// new_pos = pos + vel_mid + vel_rand
			p.m_position = plus(plus(std::move(p.m_position), vel_midpoint), 
								vel_rand);
		}
		void evaluate_particle(particle_t& p) // may update global
		{
			const auto Dimension = m_state->Dimension;
			const auto fitness = m_state->m_func(p.m_position);
			p.m_fitness.store(fitness);
			if (fitness < p.m_best_fitness.load()) {
				// Update personal best
				std::lock_guard lg{ p.m_mtx };
				p.m_best_fitness.store(fitness);
				p.m_best_position = p.m_position;

				// Update global best
				m_state->update_global_best(p);
			}
		}

	public:
		par_async_dpso_sub(dpso_state* state, size_t beg, size_t end) :
			m_state(state), m_beg(beg), m_end(end),
			m_particles(&m_state->m_particles), m_rng() {}
		par_async_dpso_sub(par_async_dpso_sub&& oth) noexcept :
			m_state(std::exchange(oth.m_state, nullptr)),
			m_beg(oth.m_beg), m_end(oth.m_end),
			m_particles(std::exchange(oth.m_particles, nullptr)),
			m_rng() { }
		~par_async_dpso_sub() {}

		par_async_dpso_sub(const par_async_dpso_sub&) = delete;
		par_async_dpso_sub& operator=(const par_async_dpso_sub&) = delete;
		
		void operator()()
		{
			for (size_t itr = 0; itr < m_state->Iteration; ++itr) {
				// Check result so far
				particle_t* gbest = m_state->m_gbest.load();
				if (gbest->m_best_fitness.load() <= m_state->m_goal) return;
				//printf_s("\n(current best: %d)\n", gbest->m_best_fitness.load());

				for (size_t idx = 0; idx < m_state->SwarmSize; ++idx) {
					particle_t& p = (*m_particles)[idx];
					const position_type gbest = best_neighbor_position(idx);
					update_position(p, gbest);
					evaluate_particle(p);
					//printf_s("%llu[%llu]: %d\n", itr, idx, p.m_fitness.load());
				} // end of per-particle loop				
			} // end of per-iteration loop

		}
	};
	
	// Data members
	std::unique_ptr<dpso_state> m_state;		
	
	void initialize_particles()
	{		
		// Create particles
		m_state->m_particles = std::vector<particle_t>(m_state->SwarmSize);
		
		// Random number engine for generating random position
		std::default_random_engine rng;

		// Prep args
		const size_t Dimension = m_state->Dimension;
		auto& particles = m_state->m_particles;
		particle_t* global_best_particle = &particles[0];
	
		// Initialize every particle
		for (auto& p : particles) {
			// Init position to a random pos
			p.m_position = m_state->random_pos(rng);

			// Init best pos to current initial pos
			p.m_best_position = p.m_position;

			// Init fitness
			const value_type fitness = m_state->m_func(p.m_position);
			p.m_fitness.store(fitness);
			p.m_best_fitness.store(fitness);

			// Init global fitness
			m_state->update_global_best(p);
		}
	}
	
public:
	template <typename U>
	par_async_dpso(
		size_t dim, 
		U&& obj_func, 
		value_type goal, value_type Xmin, value_type Xmax) : m_state(nullptr)
	{
		// May throw std::bad_alloc
		m_state = std::make_unique<dpso_state>(
			dim,
			std::forward<std::decay_t<U>>(obj_func),
			goal, Xmin, Xmax
		);
	}
	par_async_dpso(par_async_dpso&& oth) : m_state(std::move(oth.m_state)) {}
	~par_async_dpso() {}
	par_async_dpso(const par_async_dpso&) = delete;

	using opt_sz = std::optional<size_t>;
	void set_params(opt_sz neighbor_sz, opt_sz swarm_sz, opt_sz iter_sz)
	{
		if (neighbor_sz) m_state->NeighborSize = neighbor_sz.value();
		if (neighbor_sz) m_state->SwarmSize = swarm_sz.value();
		if (neighbor_sz) m_state->Iteration = iter_sz.value();
	}
	bool run_serialized(double& best_fitness, position_type& best_position)
	{
		initialize_particles();
		par_async_dpso_sub{ m_state.get(), 0, m_state->SwarmSize }();
		particle_t& p = *m_state->m_gbest.load();
		best_fitness = p.m_best_fitness.load();
		best_position = p.m_best_position;
		return  p.m_best_fitness.load() <= m_state->m_goal ?
			true :
			false;
	}
};