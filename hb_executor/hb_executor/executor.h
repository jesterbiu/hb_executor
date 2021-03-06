/*
 * Thread pool with work stealing
 * 
 * - thread count is static, no management.
 * - type alias `task_wrapper`: two type-erasure task types implementation to choose
 * - `worker_handle`: to fork task recursively * 
*/

#pragma once
#include <type_traits>
#include <memory>
#include <vector>
#include <cstddef>
#include <random>
#include <cassert>
#include <concepts>
#include <future>
#include "concurrent_std_deque.h"

namespace hungbiu
{		
	class hb_executor
	{			
	public:	// Template aliases used by hb_executor
		template <typename T>
		using promise_t = std::promise<T>;
		template <typename T>
		using future_t = std::future<T>;
		class worker_handle; // Forward declaration

	private:
		// --------------------------------------------------------------------------------
		// task_t
		// Erase function object's type but leaves return type and arg type(s)
		// --------------------------------------------------------------------------------
		template <typename R>
		using task_t = std::packaged_task<R(worker_handle&)>;
		template <typename F, typename R>
		static task_t<R> make_task(F&& func)
		{
			using F_decay = std::decay_t<F>;
			return task_t<R>(std::forward<F_decay>(func));
		}

		// --------------------------------------------------------------------------------
		// task_wrapper
		// Erase all type info
		// --------------------------------------------------------------------------------    

		// Provide task interface
		struct task_wrapper_alloc_concept
		{
			virtual ~task_wrapper_alloc_concept() = default; // Require a dtor definition for
													   // the derived class to instantiate
			virtual void run(worker_handle&) = 0;
		};

		// Provide storage for an actual task
		template <typename T>
		struct task_wrapper_alloc_model : public task_wrapper_alloc_concept
		{
			T task_;
		public:
			~task_wrapper_alloc_model() {}
			task_wrapper_alloc_model(T&& f) :
				task_(std::forward<T>(f)) {}
			task_wrapper_alloc_model(task_wrapper_alloc_model&& oth) :
				task_(std::move(oth.task_)) {}
			task_wrapper_alloc_model& operator=(task_wrapper_alloc_model&& rhs)
			{
				if (this != &rhs) {
					task_ = std::move(rhs.task_);
				}
				return *this;
			}

			task_wrapper_alloc_model(const task_wrapper_alloc_model&) = delete;
			task_wrapper_alloc_model& operator=(const task_wrapper_alloc_model&) = delete;

			void run(worker_handle& h) override
			{
				task_.operator()(h);
			}
		};

		// Provide total type erasure to support heterogeneous tasks
		// Always heap-allocated
		class task_wrapper_alloc
		{
			std::unique_ptr<task_wrapper_alloc_concept> task_vtable_;
		public:
			task_wrapper_alloc() : task_vtable_(nullptr) {}
			template <typename T>
			task_wrapper_alloc(T&& t)
			{
				using model_t = task_wrapper_alloc_model<T>;
				task_vtable_ = std::make_unique<model_t>(std::move(t));
			}
			task_wrapper_alloc(task_wrapper_alloc&& oth) noexcept :
				task_vtable_(std::move(oth.task_vtable_)) {}
			~task_wrapper_alloc() {}
			task_wrapper_alloc& operator=(task_wrapper_alloc&& rhs) noexcept
			{
				if (this != &rhs) {
					task_vtable_ = std::move(rhs.task_vtable_);
				}
				return *this;
			}

			bool valid() const noexcept
			{
				return task_vtable_.get();
			}
			explicit operator bool() const noexcept
			{
				return task_vtable_.get();
			}
			void run(worker_handle& h)
			{
				assert(valid());
				task_vtable_->run(h);
			}
		};

		// --------------------------------------------------------------------------------
		// task_wrapper with small object opitimization
		// --------------------------------------------------------------------------------
		struct task_wrapper_concept_sso
		{
			void (*_destructor)(void*) noexcept;
			void (*_move)(void*, void*) noexcept;
			void (*_run)(void*, worker_handle&);
		};

		static constexpr auto small_size = sizeof(void*) * 4;
		template <typename T>
		static constexpr bool is_small_object() { return sizeof(T) <= small_size; }

		template <typename T, bool Small = is_small_object<T>()>
		struct task_wrapper_model_sso;
		template <typename F>
		struct task_wrapper_model_sso<F, true>
		{
			F task_;

			template <typename U>
			task_wrapper_model_sso(U&& func) :
				task_(std::forward<F>(func)) {}
			task_wrapper_model_sso(task_wrapper_model_sso&& oth) :
				task_(std::move(oth.task_)) {}
			~task_wrapper_model_sso() = default;

			static void _destructor(void* p) noexcept
			{
				static_cast<task_wrapper_model_sso*>(p)->~task_wrapper_model_sso();
			}
			static void _move(void* lhs, void* rhs) noexcept
			{
				auto& model = *static_cast<task_wrapper_model_sso*>(rhs);
				new (lhs) task_wrapper_model_sso(std::move(model));
			}
			static void _run(void* p, worker_handle& h)
			{
				auto& t = static_cast<task_wrapper_model_sso*>(p)->task_;
				std::invoke(t, h);
			}
			static constexpr task_wrapper_concept_sso vtable_ = { _destructor, _move, _run };
		};
		template <typename F>
		struct task_wrapper_model_sso<F, false>
		{
			std::unique_ptr<F> ptask_;

			template <typename U>
			task_wrapper_model_sso(U&& func) :
				ptask_(std::make_unique<F>(std::forward<F>(func))) {}
			task_wrapper_model_sso(task_wrapper_model_sso&& oth) :
				ptask_(std::move(oth.ptask_)) {}
			~task_wrapper_model_sso() = default;

			static void _destructor(void* p) noexcept
			{
				static_cast<task_wrapper_model_sso*>(p)->~task_wrapper_model_sso();
			}
			static void _move(void* lhs, void* rhs) noexcept
			{
				auto& model = *static_cast<task_wrapper_model_sso*>(rhs);
				new (lhs) task_wrapper_model_sso(std::move(model));
			}
			static void _run(void* p, worker_handle& h)
			{
				auto pt = static_cast<task_wrapper_model_sso*>(p)->ptask_.get();
				if (pt) std::invoke(*pt, h);
			}
			static constexpr task_wrapper_concept_sso vtable_ = { _destructor, _move, _run };
		};

		class task_wrapper_sso
		{
			// Constness: the vtable is a const object!
			const task_wrapper_concept_sso* task_vtable_{ nullptr };
			std::aligned_storage_t<small_size> task_;
		public:
			task_wrapper_sso() : task_() {}
			template <typename T>
			task_wrapper_sso(T&& t)
			{
				using DecayT = std::decay_t<T>;
				using model_t = task_wrapper_model_sso<DecayT>;
				task_vtable_ = &model_t::vtable_;
				new (&task_) model_t(std::forward<DecayT>(t));
			}
			task_wrapper_sso(task_wrapper_sso&& oth) noexcept :
				task_vtable_(oth.task_vtable_)
			{
				task_vtable_->_move(&task_, &oth.task_);
			}
			~task_wrapper_sso()
			{
				if (task_vtable_) task_vtable_->_destructor(&task_);
			}
			task_wrapper_sso& operator=(task_wrapper_sso&& rhs) noexcept
			{
				if (this != &rhs) {
					// Destroy current task if there is one
					if (task_vtable_) task_vtable_->_destructor(&task_);

					// Copy rhs's vtable BEFORE moving the rhs's task
					// because rhs's task has an arbitrary type
					task_vtable_ = std::exchange(rhs.task_vtable_, nullptr);
					task_vtable_->_move(&task_, &rhs.task_);
				}
				return *this;
			}

			bool valid() const noexcept
			{
				return task_vtable_;
			}
			explicit operator bool() const noexcept
			{
				return valid();
			}
			void run(worker_handle& h)
			{
				if (task_vtable_) task_vtable_->_run(&task_, h);
			}
		};
				
		// Choose a flavor
		using task_wrapper = task_wrapper_sso;

		class worker; // Forward declaration because worker::get_handle() return a woker_handle object;
					  // Have to be PRIVATE
	public:		
		// --------------------------------------------------------------------------------
		// worker_handler
		// used by task object to submit work
		// --------------------------------------------------------------------------------		
		class worker_handle
		{
			worker* ptr_worker_;
		public:
			worker_handle(worker* worker) noexcept :
				ptr_worker_(worker) {}
			worker_handle(const worker_handle& oth) noexcept :
				ptr_worker_(oth.ptr_worker_) {}
			worker_handle& operator=(const worker_handle& rhs) noexcept
			{
				if (this != &rhs) ptr_worker_ = rhs.ptr_worker_;
				return *this;
			}

			template <typename T>
			static bool future_ready(const std::future<T>& fut)
			{
				assert(fut.valid());
				const auto status = fut.wait_for(std::chrono::nanoseconds{ 0 });
				return std::future_status::ready == status;
			}

			// Suspend current task to execute others
			template <typename R>
			R get(future_t<R>& fut)
			{
				while (!future_ready(fut)) {
					task_wrapper tw{};
					if (ptr_worker_->_pop(tw) ||
						ptr_worker_->_steal(tw)) {
						tw.run(*this);
					}
				}
				return fut.get();
			}

			// Submit a task to current thread, return future to obtain result
			template <
				typename F
				, typename R = std::invoke_result_t<F, worker_handle&> >
			requires std::invocable<F, hb_executor::worker_handle&>
				[[nodiscard]] future_t<R> execute_return(F&& func) const
			{
				auto t = make_task<F, R>(std::forward<F>(func));
				auto fut = t.get_future();
				ptr_worker_->_push(std::move(t));
				return fut;
			}

			// Submit a task to current thread and doesn't return result
			template <typename F>
			requires std::invocable<F, hb_executor::worker_handle&>
				void execute(F&& func) const
			{
				ptr_worker_->_push(std::move(func));
			}
		}; // end of class worker handle
	
	private:
		// --------------------------------------------------------------------------------
		// A worker object is the working context of an OS thread
		// --------------------------------------------------------------------------------
		class worker
		{
			friend class worker_handle;
			template <typename T>
			using deque_t = concurrent_std_deque<T>;

			std::atomic<bool> associated_{ false };
			hb_executor* etor_;
			std::size_t index_;
			deque_t<task_wrapper> run_stack_;

			// Push a forked task onto stack
			void _push(task_wrapper tw)
			{
				run_stack_.push_back(tw);
			}
			// Pop a task from stack for the worker itself to execute
			[[nodiscard]] bool _pop(task_wrapper& tw) noexcept
			{
				return run_stack_.pop_back(tw);
			}
			[[nodiscard]] bool _steal(task_wrapper& tw)
			{
				return etor_->steal(tw, index_);
			}
		public:
			//static constexpr auto RUN_QUEUE_SIZE = 256u;
			worker(hb_executor& etor, std::size_t idx) :
				etor_(&etor), index_(idx) {}
			~worker() {}
			worker(worker&& oth) noexcept :
				associated_(oth.associated_.load()),
				etor_(std::exchange(oth.etor_, nullptr)),
				index_(oth.index_),
				run_stack_(std::move(oth.run_stack_)) {}
			worker& operator=(const worker&) = delete;

			[[nodiscard]] bool is_associated() const noexcept
			{
				return associated_.load(std::memory_order_acquire);
			}
			void operator()()
			{
				if (associated_.exchange(true, std::memory_order_acq_rel)) {
					return;
				}
				auto h = get_handle();
				task_wrapper tw;
				while (!etor_->is_done()) {
					// get work from local stack 
					if (_pop(tw)) {
						tw.run(h);
						continue;
					}
					// steal from others
					if (etor_->steal(tw, index_)) {
						tw.run(h);
						continue;
					}
				}
				associated_.store(false);
			}
			[[nodiscard]] bool try_assign(task_wrapper& tw)
			{
				return run_stack_.try_push_front(tw);
			}
			[[nodiscard]] bool try_steal(task_wrapper& tw) noexcept
			{
				return run_stack_.try_pop_front(tw);
			}
			worker_handle get_handle() noexcept
			{
				return worker_handle{ this };
			}
		};
		// Worker thread's main function
		static void thread_main(hb_executor* this_, std::size_t init_idx)
		{
			// 
			this_->workers_[init_idx].operator()();
			for (;;) {
				if (this_->is_done()) {
					return;
				}
				for (auto& w : this_->workers_) {
					if (w.is_associated()) {
						continue;
					}
					else {
						w.operator()();
					}
				}
			}
		}
		
		// --------------------------------------------------------------------------------
		// Data members of executor
		// --------------------------------------------------------------------------------
		mutable std::atomic<bool> is_done_{ false };
		std::vector<worker> workers_;
		std::vector<std::thread> threads_;

	public:
		bool done() noexcept
		{
			bool done = false;
			return is_done_.compare_exchange_strong(done, true, std::memory_order_acq_rel);
		}
		bool is_done() const noexcept
		{
			return is_done_.load(std::memory_order_acquire);
		}

	private:
		// not thread-safe (single producer, multi consumers)
		std::size_t random_idx() noexcept
		{
			static std::mt19937_64 engine;
			return std::uniform_int_distribution<std::size_t>()(engine);
		}
		void dispatch(task_wrapper tw)
		{
			auto idx = random_idx();
			const auto sz = workers_.size();
			for (;;) {
				if (workers_[idx % sz].try_assign(tw)) {
					break;
				}
				else {
					++idx;
				}
			}
		} 
		[[nodiscard]] bool steal(task_wrapper& tw, const std::size_t idx)
		{
			static constexpr auto MAX_RAND_STEAL_ATTEMPTS = 64u;

			// Try stealing randomly 
			const auto sz = workers_.size();
			for (auto i = 0; i != MAX_RAND_STEAL_ATTEMPTS; ++i) {
				auto victim_idx = random_idx() % sz;
				if (victim_idx == idx) {
					continue;
				}					
				if (workers_[victim_idx].try_steal(tw)) {
					return true;
				}
			}

			// If failed to find a victim after pre-def attempts, 
			// decay to traversal
			for (auto i = 0; i != sz; ++i) {
				if (workers_[i].try_steal(tw)) {
					return true;
				}
			}
			return false;
		}		
			
	public:				
		hb_executor(std::size_t parallelism = std::thread::hardware_concurrency())
		{
			workers_.reserve(parallelism);
			threads_.reserve(parallelism);
			for (auto i = 0u; i < parallelism; ++i) {
				workers_.emplace_back(*this, i);
				threads_.emplace_back(thread_main, this, i);
			}
		}
		~hb_executor()
		{
			for (auto& t : threads_) {
				t.join();
			}
		}

		hb_executor(const hb_executor&) = delete;
		hb_executor& operator=(const hb_executor&) = delete;

		template <typename F, typename R = std::invoke_result_t<F, worker_handle&>>
		requires std::invocable<F, hb_executor::worker_handle&>
		[[nodiscard]] future_t<R> execute_return(F&& func)
		{
			if (is_done()) {
				return future_t<R>{};
			}
			auto t = make_task<F, R>(std::forward<F>(func));
			auto fut = t.get_future();
			dispatch( std::move(t) );
			return fut;
		}
		
		// Submit a task to current thread and doesn't return result
		template <typename F>
		requires std::invocable<F, hb_executor::worker_handle&>
		void execute(F&& func)
		{
			if (is_done()) { return; }
			dispatch( std::forward<F>(func) );
		}
	};

	template <typename F>
	concept is_hb_task = std::invocable<F, hb_executor::worker_handle&>;
}
