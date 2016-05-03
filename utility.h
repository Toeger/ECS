#ifndef UTILITY
#define UTILITY

#include <cmath>
#include <limits>
#include <type_traits>
#include <utility>

namespace Utility {
	template <class T>
	std::enable_if_t<std::is_same<T, T &&>::value, const T &> make_const(T &&t) {
		return t;
	}

	template <class T, std::size_t size>
	constexpr std::size_t element_count(const T (&)[size]) {
		return size;
	}

	template <class T>
	using remove_cvr = std::remove_cv_t<std::remove_reference_t<T>>;

	template <typename T>
	struct return_type;
	template <typename R, typename... Args>
	struct return_type<R (*)(Args...)> {
		using type = R;
	};
	template <typename R, typename... Args>
	struct return_type<R(Args...)> {
		using type = R;
	};
	template <class T>
	using return_type_t = typename return_type<T>::type;

	template <class Function>
	struct RAII {
		RAII(Function &&f)
			: f(std::move(f)) {}
		~RAII() {
			std::move(f)();
		}

		private:
		Function f;
	};
	template <class Function>
	RAII<Function> create_RAII(Function &&f) {
		return RAII<remove_cvr<Function>>(std::forward<Function>(f));
	}
	template <class T>
	struct Move_only_pointer {
		Move_only_pointer()
			: t(nullptr) {}
		Move_only_pointer(T *t)
			: t(t) {}
		Move_only_pointer(Move_only_pointer &&other)
			: t(other.t) {
			other.t = nullptr;
		}
		Move_only_pointer(const Move_only_pointer &other) = delete;
		Move_only_pointer &operator=(Move_only_pointer &&other) {
			std::swap(t, other.t);
			return *this;
		}
		Move_only_pointer &operator=(const Move_only_pointer &other) = delete;
		Move_only_pointer &operator=(T *other) {
			t = other;
			return *this;
		}
		T *operator->() const {
			return t;
		}
		operator bool() const {
			return t;
		}

		private:
		T *t;
	};

#define ON_SCOPE_EXIT_CAT(a, b) ON_SCOPE_EXIT_CAT_(a, b) // force expand
#define ON_SCOPE_EXIT_CAT_(a, b) a##b                    // actually concatenate
#define ON_SCOPE_EXIT(CODE) auto ON_SCOPE_EXIT_CAT(ON_SCOPE_EXIT_, __LINE__) = Utility::create_RAII([&]() { CODE })
}

#endif // UTILITY
