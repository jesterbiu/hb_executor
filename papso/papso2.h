#pragma once
#include <vector>
#include <tuple>
#include <variant>
#include <atomic>
#include <memory>
#include <iostream>
#include <numeric>
#include <type_traits>
#include <mutex>
#include <condition_variable>
#include "../hb_executor/executor.h"
#include "spmc_buffer.h"
#include "canonical_rng.h"

// change get_lbest 


using vec_t = std::vector<double>;
using iter = vec_t::const_iterator;
using func_t = double(*)(iter, iter);
using bound_t = std::pair<double, double>;

class pso_state {
	template <typename T>
	class alignas(64) aligned_atomic_t {
		std::atomic<T> value_;
	public:
		aligned_atomic_t(T desired = std::numeric_limits<double>::max()) 
			: value_(desired) {}
		aligned_atomic_t(aligned_atomic_t&& oth) noexcept 
			: value_(oth.load()) {}
		~aligned_atomic_t() {}

		T load() const noexcept {
			return value_.load(std::memory_order_acquire);
		}
		void store(T desired) noexcept {
			value_.store(desired, std::memory_order_release);
		}
	};

	// particle
	struct my_particle {
		double value;
		double best_value;
		vec_t velocity;
		vec_t position;
		vec_t best_position;
	};
public:
	static constexpr auto swarm_size = 40u;
	static constexpr auto iteration = 500u;
	static constexpr auto neighbor_size = 4u;

	using particle = my_particle;
	using atomic_double = aligned_atomic_t<double>;
	using buffer_t = hungbiu::naive_spmc_buffer<vec_t>;
	using size_t = std::size_t;
	using range_t = std::pair<size_t, size_t>;
	using worker_handle = hungbiu::hb_executor::worker_handle;

private:
	const func_t f;
	size_t dimension;
	double min, max;
	std::atomic<particle*> gbest = { nullptr };
	std::vector<particle> particles;
		
	//--------------------------------
	// Synchronization
	std::vector<atomic_double> best_values;
	std::vector<buffer_t> best_positions;	
	std::vector<canonical_rng> rngs;
	//--------------------------------

	std::mutex completion_mtx;
	std::condition_variable completion_cv;
	size_t forks = 0 ;

	class fork_tracer {
		pso_state* state_ptr;
	public:
		fork_tracer(pso_state* p) 
			: state_ptr(p) {
			std::lock_guard guard{ p->completion_mtx };
			p->forks++;
			printf("forks++: %llu\n", p->forks);
		}
		fork_tracer(fork_tracer&& oth) noexcept
			: state_ptr(std::exchange(oth.state_ptr, nullptr)) {}
		~fork_tracer() {
			if (!state_ptr) {
				return;
			}

			bool is_completed = false;
			{ // Critical section
				std::lock_guard guard{ state_ptr->completion_mtx };
				state_ptr->forks--;
				printf("forks--: %llu\n", state_ptr->forks);
				if (0 == state_ptr->forks) {
					is_completed = true;
				}
			}

			if (is_completed) {
				state_ptr->completion_cv.notify_one();
			}
		}
	};

	friend std::tuple<double, vec_t>
		apso(hungbiu::hb_executor&, const func_t, const bound_t&, size_t dim);
public:
	pso_state(const func_t f, const bound_t& bounds, size_t dim) :
		f(f),
		dimension(dim), min(bounds.first), max(bounds.second) {}
	pso_state(const pso_state&) = delete;

	void initialize_state(size_t fork_count) { // Tons of allocations
		particles.resize(swarm_size);
		best_values.resize(swarm_size);
		best_positions.resize(swarm_size);
		rngs.resize(fork_count);

		for (size_t i = 0; i < swarm_size; ++i) {
			particle& p = particles[i];
			p.position.resize(dimension);
			p.best_position.resize(dimension);
			p.velocity.resize(dimension);
			evaluate_particle(i);
		}
	}
	
	void initialize_swarm(canonical_rng& rng) { // rng() may throw?
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
	
	particle& update_gbest() noexcept { // Thread safe!
		particle* best_ptr = &particles.front();
		double best_val = best_values.front().load();

		for (size_t i = 0; i < swarm_size; ++i) {
			double v = best_values[i].load();
			if (v < best_val) {				
				best_ptr = &particles[i]; 
				best_val = v;
			}
		}
		gbest.store(best_ptr, std::memory_order_release);
		return *best_ptr;
	}

	void evaluate_particle(size_t i) noexcept {
		// Evaluate
		particle& p = particles[i];
		p.value = f(p.position.cbegin(), p.position.cend());

		// Update pbest
		if (p.value < p.best_value) {
			p.best_value = p.value;
			p.best_position = p.position;

			best_values[i].store(p.value);
			best_positions[i].put(p.best_position);
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

	using var_t = std::variant<const vec_t*, buffer_t::viewer>;
	var_t get_lbest(int idx, const range_t range) noexcept { // Thread safe!
		size_t lbest_idx = idx;	// !!Middle of neighbor
		double lbest_val = particles[idx].best_value;
		const int max_offset = neighbor_size / 2; // Always positive

		auto in_range = [&](size_t i) -> bool {
			return range.first <= i && i < range.second;
		};

		// Traverse neighborhood
		// offset: [-max_offset, +max_offset]
		for (int offset = -max_offset; offset <= max_offset; ++offset) {
			size_t neighbor = (idx + swarm_size + offset) % swarm_size; // in case (idx + offset) < 0

			double v = in_range(neighbor)
				? particles[neighbor].best_value
				: best_values[neighbor].load();		

			if (v < lbest_val) {
				lbest_val = v;
				lbest_idx = neighbor;
			}
		}

		// Return
		if (in_range(lbest_idx)) {
			return &particles[lbest_idx].best_position;
		}
		else {			
			return best_positions[lbest_idx].get();
		}
	}

	void move_particle(size_t idx, var_t lbest_var, canonical_rng* rng_ptr) {
		auto calculate_velocity = [rng_ptr](double vi, double xi, double pbest, double lbest) {
			static constexpr double INERTIA = 0.7298;
			static constexpr double ACCELERATOR = 1.49618;

			canonical_rng& rng = *rng_ptr;
			return INERTIA * vi
				+ ACCELERATOR * rng() * (pbest - xi)
				+ ACCELERATOR * rng() * (lbest - xi);
		};

		particle& p = particles[idx];
		const vec_t& lbest =
			(0 == lbest_var.index())
			? *std::get<0>(lbest_var) // variant holds `const vec_t*`
			: *std::get<1>(lbest_var); // variant holds `buffer_t::viewer`

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

	void pso_main_loop(range_t range, canonical_rng* rng_ptr, worker_handle& wh) {
		// Loop
		for (size_t i = 0; i < iteration; ++i) {
			for (size_t j = range.first; j < range.second; ++j) {
				// Lbest				
				// const vec_t& lbest = get_lbest_unsafe(j);
				var_t lbest_var = get_lbest(j, range);

				// Update velocity, position				
				move_particle(j, std::move(lbest_var), rng_ptr); // Sink

				evaluate_particle(j);

			} // end of particle

			if ((i + 1) % 100 == 0) {
				printf("%lf ", update_gbest().best_value);
			}
		} // end of iteration
	}

	auto fork(range_t range, canonical_rng* rng_ptr) {
		return [tracer = fork_tracer(this), this, &range, rng_ptr](worker_handle& wh) {
			pso_main_loop(range, rng_ptr, wh);
		};
	}
};


static constexpr size_t fork_size = 20;
static constexpr size_t fork_count = pso_state::swarm_size / fork_size 
									+ (pso_state::swarm_size % fork_size > 0);
std::tuple<double, vec_t>
apso(hungbiu::hb_executor& etor, const func_t f, const bound_t& bounds, size_t dim = 30) {
	auto pso_state_uptr = std::make_unique<pso_state>(f, bounds, dim);
	auto& state = *pso_state_uptr;

	using worker_handle = hungbiu::hb_executor::worker_handle;

	// Initialize
	state.initialize_state(fork_count);
	state.initialize_swarm(state.rngs[0]);

	// Main
	for (size_t i = 0; i < fork_count; ++i) {
		size_t left = fork_size * i;
		size_t right =  (i + 1) * fork_size; 
		right = std::min<size_t>(right, pso_state::swarm_size);
		etor.execute( state.fork({ left, right }, &state.rngs[i]) );
	}

	// Wait for finish
	{
		std::unique_lock lock{ state.completion_mtx };
		state.completion_cv.wait(lock, [&]() {
			bool is_completed = (0 == state.forks);
			printf("woke up to see that forks = %llu\n", state.forks);
			return is_completed;
			});
	}

	// Get result
	auto& gbest = state.update_gbest();
	return { gbest.best_value, std::move(gbest.best_position) };
}