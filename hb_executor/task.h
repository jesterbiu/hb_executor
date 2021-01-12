#include <type_traits>
#include <atomic>
#include <memory>
#include <exception>
#include <optional>
#include <functional>
#include <mutex>
#include <utility>
#include <cassert>
#include "../../concurrent_data_structures/concurrent_data_structures/spinlock.h"

namespace hungbiu
{
	template <typename>			
	class hb_inplace_function;	// this template is not defined

	template <typename R, typename... Args>
	class hb_inplace_function<R(Args...)>
	{
		struct inplace_function_concept
		{
			virtual ~inplace_function_concept() {}
			virtual void* move_to(void* buf) = 0;
			virtual R operator()(Args... args) = 0;
		};
		template <typename F>
		struct inplace_function_model : inplace_function_concept
		{
			F func_;

			template <typename U>
			inplace_function_model(U&& f) :
				func_(std::forward<std::decay_t<U>>(f)) {}
			inplace_function_model(inplace_function_model&& oth) :
				func_(std::move(oth.func_)) {}
			inplace_function_model& operator=(inplace_function_model&& rhs)
			{
				func_(std::move(oth.func_)) {}
				return *this;
			}
			void* move_to(void* buf) override
			{
				return new(buf) inplace_function_model(std::move(func_));
			}
		};
	};

	// --------------------------------------------------------------------------------
	// task_wrapper 
	// An analogy to std::packaged_task or std::function
	// Move-only
	// Cooperate with task_handle
	// --------------------------------------------------------------------------------

	class hb_task_state_base
	{
		std::atomic<std::int8_t> flag_{ false };
		spinlock mtx_;
		std::exception_ptr exception_;

		// Raw functions
		static constexpr auto ExecutedMask = 0b1;
		static constexpr auto ReadyMask = 0b10;
		static constexpr auto RetrievedMask = 0b100;
		static constexpr auto CanceledMask = 0b1000;

		void set_ready_() noexcept
		{
			flag_.fetch_or(ReadyMask);
		}
		bool is_ready_() const noexcept
		{
			return flag_.load() & ReadyMask;
		}
		bool set_retrived_() noexcept
		{
			auto old = flag_.load();
			const auto retrieved = old | RetrievedMask;
			return flag_.compare_exchange_strong(old, retrieved);
		}
		bool is_retrieved_() const noexcept
		{
			return flag_.load() & RetrievedMask;
		}
		bool set_executed_() noexcept
		{
			auto old = flag_.load();
			const auto executed = old | ExecutedMask;
			return flag_.compare_exchange_strong(old, executed);
		}
		bool is_executed_() const noexcept
		{
			return flag_.load() & ExecutedMask;
		}
		bool set_canceled_() noexcept
		{
			auto old = flag_.load();
			const auto executed = old | CanceledMask;
			return flag_.compare_exchange_strong(old, executed);
		}
		bool is_canceled_() const noexcept
		{
			return flag_.load() & CanceledMask;
		}
	};
		
	template <typename>		
	class hb_task_state;	// this template is not defined

	template <typename R, typename... Args> 
	class hb_task_state<R(Args...)> : public hb_task_state_base
	{
		// If the function return void then change it to void
		template <typename T>
		struct map_void_to_bool
		{
			using type = T;
		};
		template <>
		struct map_void_to_bool<void>
		{
			using type = bool;
		};
				
		std::optional<map_void_to_bool<R>::type> result_;
		std::function<R(Args...)> function_;

	public:
		template <typename F>
		hb_task_state(F&& f) :
			result_(), exception_(),
			function_(std::forward<F>(f)) {}
		~hb_task_state() {}
		hb_task_state(const hb_task_state&) = delete;
		hb_task_state& operator=(const hb_task_state&) = delete;

		bool result_ready() const noexcept
		{
			return is_ready_();
		}
		std::optional<R> try_get() noexcept
		{
			std::lock_guard lock{ mtx_ };

			if (is_retrieved_()) {
				throw std::exception{ "Result has been retrieved!" };
			}
			else if (exception_) {
				set_retrived_();
				std::rethrow_exception(exception_);
			}
			else if (!result_ready()) {
				return std::optional<R>{};
			}
			else { // All pre-conditions met
				set_retrived_();
				return std::move(result_);
			}
		}
		void invoke(Args&&... args)
		{
			if (is_executed_()) {
				return;
			}
			std::lock_guard lock{ mtx_ };
			try {
				if (!set_executed_()) {
					return;
				}
				if constexpr (std::is_same_v<R, void>) {
					function_(std::forward<Args>(args)...);
					result_ = true;
				}
				else {
					result_ = function_(std::forward<Args>(args)...);
				}
				set_ready_();
			}
			catch (...) {
				exception_ = std::current_exception();
			}
		}
	};

	// Task control & result retrieval
	template <typename R, typename... Args>
	class hb_task_handle
	{
		using my_state = hb_task_state<R(Args...)>;
		std::shared_ptr<my_state> state_;

		// If R = void then try_get() should return bool 
		// to indicate if the task has been completed
		template <typename T>
		struct get_return_type
		{
			using type = std::optional<T>;
		};
		template <>
		struct get_return_type<void>
		{
			using type = bool;
		};

	private: // Data member
		std::shared_ptr<my_state> state_;

		void invalid_throw_() const
		{
			if (!valid()) {
				throw std::exception{ "Task has no associated state!" };
			}
		}

	public: // Member functions
		hb_task_handle() {}
		hb_task_handle(std::shared_ptr<my_state> state) :
			state_(std::move(state)) {}
		hb_task_handle(hb_task_handle&& oth) :
			state_(std::move(oth.state_)) {}
		~hb_task_handle() {}
		hb_task_handle& operator=(hb_task_handle&& rhs)
		{
			state_ = std::move(rhs.state_);
			return *this;
		}

		bool valid() const noexcept
		{
			return state_.get();
		}
		explicit operator bool() const noexcept
		{
			return valid();
		}
		bool ready() const
		{
			invalid_throw_();
			return state_->result_ready();
		}
		void promote(Args... args)
		{
			state_->invoke(std::forward<Args>(args)...);
		}
		bool cancel() const
		{
			invalid_throw_();
			return state_->set_canceled_();
		}
		auto try_get() -> get_return_type<R>::type
		{
			invalid_throw_();
			return state_->try_get();
		}
	};

	// Task invocation
	template <typename R, typename... Args>
	class hb_task
	{		
		using my_state = hb_task_state<R(Args...)>;
		std::shared_ptr<my_state> state_;

	public:	// Member functions
		template <typename F>
		hb_task(F&& f)
		{
			state_ = std::make_shared<my_state>(std::forward<F>(f));
		}
		hb_task(hb_task&& oth) :
			state_(std::move(oth.state_)) {}
		~hb_task() {}
		hb_task& operator=(hb_task&& rhs)
		{
			if (this != &rhs) {
				state_ = std::move(rhs.state_);
			}
			return *this;
		}
		
		auto get_handle() -> hb_task_handle<R, Args...>
		{
			return hb_task_handle{ state_ };
		}
		void operator()(Args&&... args)
		{
			assert(state_);
			state_->invoke(std::forward<Args>(args)...);
		}
		bool valid() const noexcept
		{
			return state_.get();
		}
		explicit operator bool() const noexcept
		{
			return valid();
		}
	};	
}