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
#include "../hb_executor/executor.h"
#define PRINT 1
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

		atomic_aligned_wrapper() : val() {}
		atomic_aligned_wrapper(atomic_aligned_wrapper&& oth) noexcept : 
			val(oth.val.exchange(0, std::memory_order_acq_rel)) {}
		~atomic_aligned_wrapper() {}
	};

	// Storage for PAPSO states
	template <template<typename> typename BufferT>
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
		using rng_type = std::default_random_engine;
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

	template <template<typename> typename BufferT = hungbiu::naive_spmc_buffer>
	class parallel_async_pso {
		using pso_state_type = pso_state<BufferT>;
		using particle_type = pso_state_type::particle_type;
		using position_type = particle_type::position_type;
		using velocity_type = particle_type::velocity_type;
		using value_type = particle_type::value_type;
		using buffer_type = pso_state_type::buffer_type;
		using worker_handle = hungbiu::hb_executor::worker_handle;
		using rng_type = pso_state_type::rng_type;
		using aligned_rng = pso_state_type::aligned_rng;

		// Return random real number from [0, 1)
		static double my_generate_random(rng_type& rng) {
			return std::generate_canonical<double, std::numeric_limits<double>::digits>(rng);
		}

		double evaluate(const position_type& pos) const {
			return state_->object_function(&pos[0], pos.size());
		}

		// Return true if particle is new gbest
		// Alternaltive: Traverse the pbests vector ??
		bool update_gbest(const particle_type* particle) const noexcept {
			const double v = particle->value;
			if (v >= state_->global_best_value.load(std::memory_order_relaxed)) {
				return false;
			}

			// Update
			std::lock_guard guard{ state_->mtx };
			if (v < state_->global_best_value.load(std::memory_order_relaxed)) { // Double check
				state_->global_best_value.store(v);
				state_->global_best_particle = particle;
			}
			return true;
		}

		// Return if there is improvement from previous pbest
		bool update_pbest(size_t idx) const {
			particle_type& p = state_->particles[idx];
			std::atomic<double>& pbest = state_->best_values[idx].val;

			// Return if no improvement
			if (p.value >= pbest.load(std::memory_order_acquire)) {
				return false;
			}

			// Update
			p.best_position = p.position;
			
			state_->best_positions[idx].put(p.position); // May be contended
			pbest.store(p.value);

			return true;
		}

		auto get_lbest_view(size_t index) const noexcept { // Ring topology
			size_t swarm_sz = state_->swarm_size;
			auto& best_vals = state_->best_values;
			double best_v = best_vals[index].val.load();
			size_t best_idx = index;
			for (int i = -1; i < 2; ++i) {
				// In case (idx - 1) is negative, (... + swarm_sz) 
				size_t idx = (index + swarm_sz + i) % swarm_sz;
				double v = best_vals[idx].val.load();
				if (v < best_v) {
					best_v = v;
					best_idx = idx;
				}
			}
			return state_->best_positions[best_idx].get();
		}

		bool is_within_bondary(const position_type& pos) const noexcept {
			const double min = state_->min_x;
			const double max = state_->max_x;

			for (auto& x : pos) {
				if (x < min || x > max) {
					return false;
				}
			}
			return true;
		}

		void pso_main_loop(size_t beg, size_t end, size_t iteration, rng_type& rng, worker_handle& handle) const {
			static constexpr double W = 0.72984;	// Inertia weight
			static constexpr double Phi = 1.49618;	// Constriction, no need for velocity clamping

			auto& particles = state_->particles;
			const particle_type* ibest = &particles[0]; // best particle of this iteration

			// Calculate
			static constexpr auto iter_per_task = 100;
			for (size_t iter = iteration; iter < iteration + iter_per_task; iter++) {
				for (size_t i = beg; i < end; ++i) { // Update each particle
					particle_type& p = particles[i];

					// Evaluation
					// Applies "let the particle fly" method: does not evaluate particle out of bound, 
					// wait for it to be drawn back by pbest and lbest
					if (is_within_bondary(p.position)) {
						p.value = evaluate(p.position);
						if (p.value < ibest->value) {
							ibest = &p;
						}
						update_pbest(i);
					}

					// Update
					const position_type& pbest = p.best_position;	// Particle's personal best in history
					auto lbest_view = get_lbest_view(i);			// Best in local ring neighborhood
					const position_type& lbest = *lbest_view;

					auto next_vel = W * p.velocity
						+ Phi * my_generate_random(rng) * (pbest - p.position)
						+ Phi * my_generate_random(rng) * (lbest - p.position);
					evaluate_expression(p.velocity, next_vel);

					lbest_view.unlock(); // Reduce shared time

					auto next_pos = p.position + p.velocity;
					evaluate_expression(p.position, next_pos);
				}
			}

			// Update gbest after iteration of subswarm [beg, end)
#ifdef PRINT
			printf("best of the round: %.6lf\n", ibest->value);
#endif
			update_gbest(ibest);

			// Terminate condition
			if (iteration + iter_per_task < state_->iteration) { // Fork
				handle.execute( fork_subswarm(beg, end, iteration + iter_per_task, rng) );
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
			position_type pos(state_->dimension, 0); // Allocation; may throw
			const double min = state_->min_x;
			double diff = (state_->max_x - state_->min_x) * region_scaling;
			
			for (double& x : pos) {
				x = min + diff * my_generate_random(rng);				
			}
			return pos;
		}

		// Uniform distributed in 1/4 of the search space
		void initialize_swarm(size_t beg, size_t end, rng_type& rng, worker_handle& handle) const {
#ifdef PRINT
			printf("initialize_swarm [%llu, %llu)\n", beg, end);
#endif
			std::vector<particle_type>& particles = state_->particles;
			for (size_t i = beg; i < end; ++i) {
				particle_type& particle = particles[i];

				// Init position to random pos
				particle.position = random_position(rng);

				// Init velocity
				position_type rand_pos = random_position(rng);
				auto init_vel = (rand_pos - particle.position) / 2.;
				particle.velocity.resize(state_->dimension);
				evaluate_expression(particle.velocity, init_vel);
			}
		}

		// Return a lambda that evolves a subswarm given [beg, end)
		auto fork_subswarm(size_t beg, size_t end, size_t iteration, rng_type& rng) const noexcept {
#ifdef PRINT
			printf("\n**fork_subswarm [%llu, %llu) : %llu**\n"
				, beg, end, iteration);
#endif
			return [this, beg, end, iteration, &rng](worker_handle& handle) {
				pso_main_loop(beg, end, iteration, rng, handle);
			};
		}

		void entry(size_t partition_thresh, worker_handle& handle) const {
			// Pre-calculation for initializations
			const size_t swarm_sz = state_->swarm_size;
			size_t fork_count = swarm_sz / partition_thresh;
			if (swarm_sz % partition_thresh) {
				fork_count += 1;
			}
			{
				std::lock_guard guard{ state_->done_mtx };
				state_->fork_count = fork_count;
			}

			// Swarm
			state_->particles.resize(swarm_sz);

			// Pbests: values, positions
			state_->best_values.resize(swarm_sz);
			for (auto& v : state_->best_values) {
				v.val = std::numeric_limits<double>::max();
			}

			state_->best_positions.resize(swarm_sz);

			// Rngs for each subswarm	
			state_->random_number_generators.resize(fork_count);

			// Fork initialization
			/*static constexpr size_t Init_Partition_Threshold = 40;
			for (size_t i = Init_Partition_Threshold; i < swarm_size; ) {
				size_t beg = i;
				i += Init_Partition_Threshold;
				size_t end = std::min(swarm_sz, i);
				handle.execute(initialize_swarm(beg, end, handle);
			}*/

			// My initialization
			auto rng = std::make_unique<rng_type>();
			initialize_swarm(0, swarm_sz, *rng, handle);

			// when_all(initalizations);

			// Fork main loop
			auto rngs_iter = state_->random_number_generators.begin();
			for (size_t i = 0; i < swarm_sz; ) {
				// Range
				size_t beg = i;
				i += partition_thresh;
				size_t end = std::min(swarm_sz, i);

				handle.execute( fork_subswarm(beg, end, 0, rngs_iter->val) );
				++rngs_iter;
			}
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
			state_(std::move(oth)) {}
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
		return_type evolve(hungbiu::hb_executor& etor, size_t partition_thresh) {
			auto f = [this, partition_thresh](worker_handle& handle) {
				entry(partition_thresh, handle);
			};
			etor.execute(f);
#ifdef PRINT
			printf("execute(entry)\n");
#endif
			// Wait for calculation to complete
			auto predicate_done = [this]() -> bool {
				return state_->fork_count != 0
					&& state_->fork_count == state_->done_count; 
			};
			{
				std::unique_lock ulock(state_->done_mtx);
				state_->done_cv.wait(ulock, predicate_done);

			}
			
			std::lock_guard guard{ state_->mtx };
			return { state_->global_best_value.load()
					, std::move(state_->global_best_particle->best_position) };
		}
	};
}
