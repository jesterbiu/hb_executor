#pragma once
#include <type_traits>
#include <future>
#include <memory>
#include <vector>
#include <cstddef>
#include <random>
#include <cassert>
#include <concepts>
#include "concurrent_std_deque.h"
#include "../../concurrent_data_structures/concurrent_data_structures/array_blocking_queue.h"
namespace hungbiu
{	
	class worker_handle;
	template <typename F>
	concept ForkJoinTask = std::is_invocable<F, worker_handle&>::value;
	class hb_executor
	{	
	public:
		template <typename T>
		using promise_t = std::promise<T>;
		template <typename T>
		using future_t = std::future<T>;
		template <typename T>

		friend bool future_ready(std::future<T>& fut)
		{
			auto status = fut.wait_for(std::chrono::nanoseconds(1)));
			return std::future_status::ready == status;
		}
	private:
		class worker_handle;
		template <typename R>
		using task_t = std::packaged_task<R(worker_handle&)>;
		template <typename F, typename R>
		static task_t<R> make_task(F&& func)
		{
			using F_decay = std::decay_t<F>;
			return task_t<R>(std::forward<F_decay>(func));
		}
		struct task_wrapper_concept
		{
			virtual ~task_wrapper_concept() {}
			virtual void run(worker_handle&) = 0;
		};
		template <typename T>
		class task_wrapper_model : task_wrapper_concept
		{
			T task_;
		public:
			~task_wrapper_model() {}			
			task_wrapper_model(T&& f) :
				task_(std::move(T)) {}
			task_wrapper_model(task_wrapper_model&& oth) :
				task_(std::move(oth.task_)) {}
			task_wrapper_model& operator=(task_wrapper_model&& rhs)
			{
				if (this != &oth) {
					task_ = std::move(rhs.task_);
				}
				return *this;
			}
			task_wrapper_model(const task_wrapper_model&) = delete;
			task_wrapper_model& operator=(const task_wrapper_model&) = delete;

			void run(worker_handle& h) override
			{
				task_.operator()(h);
			}
		};
		class task_wrapper
		{
			std::unique_ptr<task_wrapper_concept> p_task_;
		public:
			task_wrapper() : p_task_(nullptr) {}
			template <typename R>
			task_wrapper(task_t<R>&& t)
			{
				using model_t = task_wrapper_model<task_t<R>>;
				p_task_ = std::make_unique<model_t>(std::move(t));
			}
			task_wrapper(task_wrapper&& oth) :
				p_task_(std::move(oth.p_task_))	{}
			~task_wrapper() {}
			task_wrapper& operator=(task_wrapper&& rhs)
			{
				if (this != &rhs) {
					p_task_ = std::move(rhs.p_task_);
				}
				return *this;
			}
			
			bool valid() const noexcept
			{
				return p_task_.get();
			}
			explicit operator bool() const noexcept
			{
				return p_task_.get();
			}
			void run(worker_handle& h)
			{
				assert(valid());
				p_task_->run(h);
			}
		};
		class worker
		{
			friend class worker_handle;
			template <typename T>
			using queue_t = array_blocking_queue<T>;
			template <typename T>
			using deque_t = concurrent_std_deque<T>;

			std::atomic<bool> associated_{ false };
			hb_executor& etor_;
			const std::size_t index_;
			deque_t<task_wrapper> run_stack_;
				
			void push_stack(task_wrapper tw)
			{
				run_stack_.push_back(tw);
			}
			[[nodiscard]] bool pop_stack(task_wrapper& tw) noexcept
			{
				return run_stack_.pop_back(tw);
			}
			void enqueue(task_wrapper tw)
			{
				run_stack_.push_front(tw);
			}			
		public:
			//static constexpr auto RUN_QUEUE_SIZE = 256u;
			worker(hb_executor& etor, std::size_t idx) :
				etor_(etor), index_(idx) {}
			~worker() {}
			worker(const worker&) = delete;
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
				while (!etor_.is_done()) {
					// get work from local stack 
					if (pop_stack(tw)) {
						tw.run(h);
						continue;
					}
					// steal from others
					if (etor_.steal(tw, index_)) {
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

		mutable std::atomic<bool> is_done_{ false };
		std::vector<worker> workers_;
		std::vector<std::thread> threads_;
	public:
		bool done() const noexcept
		{
			bool done = false;
			return is_done_.compare_exchange_strong(done, true, std::memory_order_acq_rel);
		}
		bool is_done() const noexcept
		{
			return is_done_.load(std::memory_order_acquire);
		}
	private:
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

			const auto sz = workers_.size();
			// Randomized stealing for a couple times
			for (auto i = 0; i != MAX_RAND_STEAL_ATTEMPTS; ++i) {
				auto victim_idx = random_idx() % sz;
				if (victim_idx == idx) {
					continue;
				}					
				if (workers_[victim_idx].try_steal(tw)) {
					return true;
				}
			}
			// If task not found, decay to traversal
			for (auto i = 0; i != sz; ++i) {
				if (workers_[i].try_steal(tw)) {
					return true;
				}
			}
			return false;
		}		
		static void thread_main(hb_executor* this_, std::size_t init_idx)
		{
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
	public:		
		
		class worker_handle
		{
			worker* worker_;
		public:
			worker_handle(worker* worker) noexcept :
				worker_(worker) {}
			worker_handle(const worker_handle& oth) noexcept :
				worker_(oth.worker_) {}
			worker_handle& operator=(const worker_handle& rhs) noexcept
			{
				if (this != &rhs) worker_ = rhs.worker_;
				return *this;
			}
			
			template <
				typename F,
				typename R = std::invoke_result_t<F, worker_handle&>>
				requires ForkJoinTask<F>
				[[nodiscard]] future_t<R> run_later(F&& func) const
			{
				if (worker_->etor_.is_done()) {
					return future_t<R>();
				}
				auto t = make_task<F, R>(std::forward<F>(f));
				auto fut = t.get_future();
				worker_->enqueue(task_wrapper{ std::move(t) });
				return fut;
			}
			template <
				typename F,
				typename R = std::invoke_result_t<F, worker_handle&>,
				std::enable_if_t<std::is_base_of_v<runnable_base<F>, F>> = 0>
				[[nodiscard]] future_t<R> run_next(F&& func) const
			{
				if (worker_->etor_.is_done()) {
					return future_t<R>();
				}
				auto t = make_task<F, R>(std::forward<F>(f));
				auto fut = t.get_future();				;
				worker_->push_stack(task_wrapper{ std::move(t) });
				return fut;
			}
		}; // end of class worker handle
	
		hb_executor(std::size_t parallelism = std::thread::hardware_concurrency())
		{
			for (auto i = 0u; i < parallelism; ++i) {
				workers_.emplace_back(*this, i);
				threads_.emplace_back(thread_main, this);
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

		template <
			typename F, 
			typename R = std::invoke_result_t<F, worker_handle&>>
		[[nodiscard]] future_t<R> submit(F&& func)
		{
			if (is_done.load(std::memory_order_acquire)) {
				return future_t<R>();
			}
			auto t = make_task<F, R>(std::forward<F>(func));
			auto fut = t.get_future();
			dispatch(task_wrapper{ std::move(t) });
			return fut;
		}
		
	};

	/*
	* // allocate continuation object
	* auto cont = h.get_continuation(continuation_func);
	* 
	* // fork arbitrary amount of child task if needed
	* h.summit(cont, task);
	* 
	* return;
	*/

	/*
	* class continuation
	* {
	*     struct ctrl_blk
	*	  {
	*         std::atomic<size_t> ref_count_;
	*         task_wrapper task;
	*     };
	*     std::shared_ptr<ctrl_blk> ctrl_;
	* public:
	*	  task_wrapper add_child(F&& func)
	*     {
	*         ctrl_->ref_count_.fetch_add(1);
	*		  return task_wrapper {
	*		  [&](F&& func) {
	*		      func();
	*			  
	*     }
	* };
	*/
}
