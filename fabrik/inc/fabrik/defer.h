
#pragma once

namespace detail {
	template<typename F>
	class defer_impl {
	public:
		inline defer_impl(const F& f) : f(f) {}
		inline defer_impl(defer_impl<F>&& other) : f(other.c_f) {}
		inline ~defer_impl() {
			try {
				f();
			}
			catch (...) {}
		}
	private:
		F f;
	};

	struct defer_impl_helper {
		template<typename F>
		inline defer_impl<F> operator<<(const F& lambda) {
			return defer_impl<F>(lambda);
		}
	};
}

#define defer_CAT_IMPL(x, y) x ## y
#define defer_CAT(x, y) defer_CAT_IMPL(x, y)
#define defer auto defer_CAT(defer_, __LINE__) = detail::defer_impl_helper() << [&]()
