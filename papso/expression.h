#pragma once
#include <vector>
#include <functional>
#include <tuple>
#include <cassert>

// Expression templates 
// for expressive, lazy-evaluation formula calculation
namespace hungbiu {
	template <typename T>
	concept has_subsript = requires (const T & t, size_t i) {
		t[i];
	};

	template <typename T>
	auto at(const T& t, size_t i) {
		if constexpr (has_subsript<T>) return t[i];
		else return t;
	}

	template <typename F, typename... Operands>
	class expression {
		F f_;
		std::tuple<const Operands&...> args_;  // Const ref!

	public:
		expression(F f, const Operands&... args) : // Const ref!
			f_(f), args_(args...) {}

		auto operator[](size_t i) const {
			auto call_at_i = [this, i](const auto&... args) {
				return f_(at(args, i)...);
			};
			return std::apply(call_at_i, args_);
		}
	};


	// Does not allocate!
	// Always make sure that `result` has enough capacity
	template <typename R, typename Expr>
	requires has_subsript<R>&& has_subsript<Expr>
	R& evaluate_expression(R& result, const Expr& expr) noexcept {
		size_t rsz = result.size();
		for (size_t i = 0; i < rsz; ++i) {
			result[i] = expr[i];
		}
		return result;
	}
}
