#pragma once
#include <deque>
#include <mutex>
#include <atomic>
#include "../../concurrent_data_structures/concurrent_data_structures/spinlock.h"

namespace hungbiu
{
	template <typename T>
	class concurrent_std_deque
	{
		hungbiu::spinlock lock_;
		std::deque<T> deque_;
		std::atomic<std::size_t> pop_count_{ 0 };
		void shrink_if_needed()
		{
			const auto cnt = pop_count_.fetch_add(1);
			if (cnt % 64 == 0) deque_.shrink_to_fit();
		}

	public:
		concurrent_std_deque() = default;
		~concurrent_std_deque() = default;
		concurrent_std_deque(const concurrent_std_deque&) = delete;
		concurrent_std_deque& operator=(const concurrent_std_deque&) = delete;

		void push_back(T& v)
		{
			std::lock_guard lg{ lock_ };
			deque_.emplace_back(std::move(v));
		}
		void push_front(T& v)
		{
			std::lock_guard lg{ lock_ };
			deque_.emplace_front(std::move(v));
		}
		[[nodiscard]] bool pop_front(T& v) noexcept
		{
			std::lock_guard lg{ lock_ };
			if (deque_.empty()) return false;
			v = std::move(deque_.front());
			deque_.pop_front();
			shrink_if_needed();
			return true;
		}
		[[nodiscard]] bool pop_back(T& v) noexcept
		{
			std::lock_guard lg{ lock_ };
			if (deque_.empty()) return false;
			v = std::move(deque_.back());
			deque_.pop_back();
			shrink_if_needed();
			return true;
		}
		[[nodiscard]] bool try_push_front(T& v)
		{
			if (!lock_.try_lock()) return false;
			std::lock_guard lg{ lock_, std::adopt_lock };
			deque_.emplace_front(std::move(v));
			return true;
		}
		[[nodiscard]] bool try_pop_front(T& v) noexcept
		{
			if (!lock_.try_lock()) return false;
			std::lock_guard lg{ lock_, std::adopt_lock };
			if (deque_.empty()) return false;
			v = std::move(deque_.front());
			deque_.pop_front();
			shrink_if_needed();
			return true;
		}
	};
}
