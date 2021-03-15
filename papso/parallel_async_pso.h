#pragma once
#include <tuple>
#include <utility>
#include <functional>
#include <atomic>
#include <cstddef>
#include <numeric>
#include <memory>
#include <array>
#include <type_traits>
#include <cassert>
#include <random>
#include <mutex>
#include <shared_mutex>
#include <span>
#include <condition_variable>
#include "spmc_buffer.h"
#include "expression.h"
#include "canonical_rng.h"
#include "../hb_executor/executor.h"
#define PRINT
#undef PRINT
#ifdef PRINT
#include <cstdio>
#endif
namespace hungbiu {
	template <typename V, typename Vel, typename Pos>
	struct alignas(64) basic_particle
	{
		using value_type = V;
		using velocity_type = std::vector<Vel>;
		using position_type = std::vector<Pos>;

		V value{ std::numeric_limits<V>::max() };
		velocity_type velocity;
		position_type position;
		position_type best_position;
	};

	// double * velocity -> velocity
	template <typename Lhs, typename Rhs>
	auto operator*(const Lhs& lhs, const Rhs& rhs) {
		using times = std::multiplies<double>;
		return expression{ times{}, lhs, rhs };
	}

	// velocity / double -> velocity
	template <typename Lhs, typename Rhs>
	auto operator/(const Lhs& lhs, const Rhs& rhs) {
		using divides = std::divides<double>;
		return expression{ divides{}, lhs, rhs };
	}

	// position - position -> velocity
	template <typename Lhs, typename Rhs>
	auto operator-(const Lhs& lhs, const Rhs& rhs) {
		using minus = std::minus<double>;
		return expression{ minus{}, lhs, rhs };
	}

	// velocity + velocity -> velocity
	// position + velocity -> position
	template <typename Lhs, typename Rhs>
	auto operator+(const Lhs& lhs, const Rhs& rhs) {
		using plus = std::plus<double>;
		return expression{ plus{}, lhs, rhs };
	}

	template <typename T> requires std::movable<T>
	struct alignas(64) movable_aligned_wrapper {
		T val;

		movable_aligned_wrapper() : val() {}
		template <typename... Args>
		movable_aligned_wrapper(Args...args) :
			val(std::forward<Args>(args)...) {}
		movable_aligned_wrapper(movable_aligned_wrapper&& oth) noexcept : 
			val(std::move(oth.val)) {}
		~movable_aligned_wrapper() {}
	};

	template <typename T> 
	struct alignas(64) atomic_aligned_wrapper {
		std::atomic<T> val;

		atomic_aligned_wrapper(T desired = 0) : val(desired) {}
		atomic_aligned_wrapper(atomic_aligned_wrapper&& oth) noexcept : 
			val(oth.val.exchange(0, std::memory_order_acq_rel)) {}
		~atomic_aligned_wrapper() {}
	};

	// Storage for PAPSO states
	template <template<typename> typename BufferT >
	struct pso_state {
		// Defaultable settings
		size_t iteration = 100;
		size_t neighbor_size = 2;
		size_t swarm_size = 40;

		// Instance settings
		size_t dimension = 0;
		double min_x = 0;
		double max_x = 0;
		std::function<double(const double*, size_t)> object_function;

		// Swarm
		using particle_type = basic_particle<double, double, double>;
		using aligend_value = atomic_aligned_wrapper<double>;
		using buffer_type = BufferT<particle_type::position_type>;
		using rng_type = canonical_rng;
		using aligned_rng = movable_aligned_wrapper<rng_type>;

		std::vector<particle_type> particles = {};
		std::vector<aligend_value> best_values = {};
		std::vector<buffer_type> best_positions = {};
		std::vector<aligned_rng> random_number_generators = {};

		// Read write lock
		std::mutex mtx;
		std::atomic<double> global_best_value = { std::numeric_limits<double>::max() };
		const particle_type* global_best_particle = { nullptr };

		std::mutex done_mtx;
		std::condition_variable done_cv;
		size_t done_count = 0;
		size_t fork_count = 0;

		template <typename F>
		pso_state(F&& f, size_t dim, double min, double max) :
			dimension(dim)
			, min_x(min), max_x(max)
			, object_function(std::forward<F>(f)) {}
	};

	template <typename T>
	auto span_of(std::vector<T>& vec) {
		return std::span{ vec.front(), vec.back() };
	}

	class parallel_async_pso {
		template <typename T>
		using buffer_type = naive_spmc_buffer<T>;
		using pso_state_type = pso_state<buffer_type>;
		using particle_type = pso_state_type::particle_type;
		using position_type = particle_type::position_type;
		using velocity_type = particle_type::velocity_type;
		using value_type = particle_type::value_type;
		using worker_handle = hungbiu::hb_executor::worker_handle;
		using rng_type = pso_state_type::rng_type;
		using aligned_rng = pso_state_type::aligned_rng;

		// Return true if particle improve from its pbest
		bool evaluate(size_t idx) const {
			particle_type& p = state_->particles[idx];
			p.value = state_->object_function(p.position.data(), p.position.size());
			return update_pbest(idx);
		}

		// Return true if particle is new gbest
		// Alternaltive: Traverse the pbests vector ??
		bool update_global(const particle_type* particle) const noexcept {
			
			return true;
		}

		// Return if there is improvement from previous pbest
		bool update_pbest(size_t idx) const {
			// Return if no improvement
			std::atomic<double>& pbest = state_->best_values[idx].val;
			particle_type& p = state_->particles[idx];

			if (p.value >= pbest.load(std::memory_order_acquire)) {
				return false;
			}


			// In-place update
			p.best_position = p.position; 

			// Publish
			pbest.store(p.value);
			// state_->best_positions[idx].put(p.best_position); (parallel only)

			return true;
		}		

		// Traverse the neighborhood to find the best neighbor
		// No special treatment if best neighbor is particle[index] itself
		auto get_lbest_view(size_t index) const noexcept { // Ring topology
			size_t swarm_sz = state_->swarm_size;
			size_t best_idx = index;
			auto& best_vals = state_->best_values;
			double best_v = best_vals[index].val.load(std::memory_order_acquire);

			const int offset = state_->neighbor_size / 2;
			for (int i = -offset; i <= offset; ++i) {
				// In case idx is negative, (... + swarm_sz) 
				size_t idx = (index + swarm_sz + i) % swarm_sz;
				double v = best_vals[idx].val.load(std::memory_order_acquire);
				if (v < best_v) {
					best_v = v;
					best_idx = idx;
				}
			}
			return state_->best_positions[best_idx].get();
		}

		value_type best_value_of(size_t idx) const noexcept {
			return state_->best_values[idx].val.load(std::memory_order_acquire);
		}

		const position_type& get_lbest_unsafe(int idx) const noexcept {
			const int offset = state_->neighbor_size / 2; // Positive
			const size_t swarm_size = state_->swarm_size;

			value_type best_v = best_value_of(idx);
			particle_type* best_ptr = &state_->particles[idx];

			for (int off = idx - offset; off <= idx + offset; ++off) {
				// `+ swarm_size` in case that `(idx + off) < 0`
				size_t neighbor_idx = (idx + off + swarm_size) % swarm_size;
				value_type neighbor_val = best_value_of(neighbor_idx);
				if (neighbor_val < best_v) {
					best_v = neighbor_val;
					best_ptr = &state_->particles[neighbor_idx];
				}
			}

			return best_ptr->best_position;
		}
		
		void update_vel_pos(size_t idx, rng_type& rng) const {
			static constexpr double W = 0.72984;	// Inertia weight
			static constexpr double Phi = 1.49618;	// Constriction, no need for velocity clamping

			const size_t dim = state_->dimension;
			particle_type& p = state_->particles[idx];
			const position_type& pbest = p.best_position;
			const position_type& lbest = get_lbest_unsafe(idx);

			const auto min = state_->min_x;
			const auto max = state_->max_x;

			for (size_t i = 0; i < dim; ++i) {
				p.velocity[i] =
					W * p.velocity[i]
					+ Phi * rng() * (pbest[i] - p.position[i])
					+ Phi * rng() * (lbest[i] - p.position[i]);
				p.position[i] += p.velocity[i];

				// Confinement by
				// Maurice Clerc, Standard PSO 2006
				auto& xi = p.position[i];
				if (min <= xi && xi <= max) {
					continue;
				}
				else if (xi < min) {
					xi = min;
				}
				else if (xi > max) {
					xi = max;
				}
				else {} // nothing

				p.velocity[i] = 0;
			}
		}

		void pso_main_loop(size_t beg, size_t end, size_t iteration, rng_type& rng) const {
			pso_state_type& state = *state_;

			// Calculate
			auto iter_per_task = state.iteration; // For canonical version only

			particle_type* ibest = &state.particles.front();

			for (size_t iter = iteration; iter < iteration + iter_per_task; iter++) {
				for (size_t i = beg; i < end; ++i) {			
					particle_type& p = state.particles[i];
					
					// Evaluation
					evaluate(i);
					if (p.value < ibest->value) {
						ibest = &p;
					}


					// Update: velocity, position
					update_vel_pos(i, rng);
				}

				// Update gbest after iteration of subswarm [beg, end)
#ifdef PRINT
				printf("best of the round: %.6lf\n", ibest->value);
#endif

				update_global(ibest);
			}


			// Terminate condition
			if (iteration + iter_per_task < state_->iteration) { // Fork
				//handle.execute( fork_subswarm(beg, end, iteration + iter_per_task, rng) );
			}
			else { 
				std::unique_lock ulock{ state_->done_mtx };
				state_->done_count += 1;
				if (state_->fork_count == state_->done_count) {
					ulock.unlock();
					state_->done_cv.notify_one();
				}
			}
		}

		// `region_scaling`: 
		// In order to avoid initialization bias, initialize particle in subspace of the search space 
		// given the `region_scaling`, forcing the swarm to expand.
		// 1.0 -> the whole search space, 0.25 -> 1/4 of the search space
		position_type random_position(rng_type& rng, float region_scaling = 1.0) const {
			const size_t dim = state_->dimension;
			position_type pos(dim);

			const auto min = state_->min_x;
			const auto max = state_->max_x;
			const auto range = region_scaling * (max - min);

			for (auto& x : pos) {
				x = min + range * rng();
			}

			return pos;
		}

		// Uniform distributed in 1/4 of the search space
		void initialize_swarm(size_t beg, size_t end, rng_type& rng) const {
			const size_t dim = state_->dimension;
			std::vector<particle_type>& particles = state_->particles;

			for (size_t i = beg; i < end; ++i) {
				particle_type& p = particles[i];

				// Random position
				p.position = random_position(rng);

				// Random velocity
				const position_type rand_pos = random_position(rng);
				p.velocity.resize(dim);
				for (size_t i = 0; i < dim; ++i) {
					p.velocity[i] = (rand_pos[i] - p.position[i]) / 2.0;
				}
			}
		}

		void entry() const {
			pso_state_type& state = *state_;

			// Pre-calculation for initializations
			const size_t swarm_size = state.swarm_size;

			// Swarm
			state.particles.resize(swarm_size);

			// Pbests: values, 
			// positions (parallel only)
			state.best_values.reserve(swarm_size);
			for (size_t i = 0; i < swarm_size; ++i) {
				state.best_values.emplace_back(std::numeric_limits<double>::max());
			}

			// Rngs for each subswarm (parallel only)
			std::unique_ptr<rng_type> rng_uptr = std::make_unique<rng_type>();

			// My initialization
			initialize_swarm(0, swarm_size, *rng_uptr);

			// Fork main loop
			pso_main_loop(0, swarm_size, 0, *rng_uptr);			
		}

		std::unique_ptr<pso_state_type> state_ = {};

	public:
		template <typename F>
		parallel_async_pso(F&& f, size_t dim, double min, double max) {
			static_assert(is_object_function_type<F>::value
						, "Object function should has the signature of: `double(const double*, size_t)`!");
			state_ = std::make_unique<pso_state_type>(std::forward<F>(f), dim, min, max);		
		}
		parallel_async_pso(parallel_async_pso&& oth) noexcept :
			state_(std::move(oth.state_)) {}
		~parallel_async_pso() {}
		parallel_async_pso& operator=(parallel_async_pso&& rhs) noexcept {
			state_ = std::move(rhs.state_);
			return *this;
		}

		parallel_async_pso(const parallel_async_pso&) = delete;
		parallel_async_pso& operator=(const parallel_async_pso&) = delete;

		void set_iteration(size_t i) const noexcept {
			state_->iteration = i;
		}
		void set_neighbor_size(size_t nb_size) const noexcept {
			state_->neighbor_size = nb_size;
		}
		void set_swarm_size(size_t swm_size) const noexcept {
			state_->swarm_size = swm_size;
		}

		template <typename F>
		struct is_object_function_type {
			static constexpr bool value = std::is_invocable_r_v<double, F, const double*, size_t>;
		};
		template <typename F>
		void reset_object_function(F&& f, size_t dim, double min, double max) {
			static_assert(is_object_function_type<F>::value
						, "Object function should has the signature of: `double(const double*, size_t)`!");

			state_->object_function = std::forward<F>(f);
			state_->dimension = dim;
			state_->min_x = min;
			state_->max_x = max;
		}

		using return_type = std::tuple<double, position_type>;
		return_type evolve() {			
			entry();
			
			double best = std::numeric_limits<double>::max();
			particle_type* best_ptr = &state_->particles[0];
			for (size_t i = 0; i < state_->swarm_size; ++i) {
				double v = best_value_of(i);
				if (v < best) {
					best = v;
					best_ptr = &state_->particles[i];
				}
			}
			return { best
					, std::move(best_ptr->best_position) };
		}
	};
}
