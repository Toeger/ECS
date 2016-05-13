#ifndef LOG_H
#define LOG_H

#define LOG_LEVEL_NONE 0
#define LOG_LEVEL_NOTE 1
#define LOG_LEVEL_DEBUG 2
#define LOG_LEVEL_ALL 3

#ifdef NDEBUG
#define LOG_LEVEL_DEFAULT LOG_LEVEL_NONE
#else
#define LOG_LEVEL_DEFAULT LOG_LEVEL_ALL
#endif

#define LOG_LEVEL LOG_LEVEL_DEFAULT

#include <functional>
#include <iostream>

namespace Log {
	struct Log_dummy {
		template <class T>
		Log_dummy &operator<<(T && /*unused*/) {
			return *this;
		}
	};
	struct Log_real {
		template <class T>
		Log_real &operator<<(T &&t) {
			std::cerr << std::forward<T>(t);
			return *this;
		}
		~Log_real() {
			std::cerr << std::endl;
		}
	};

	inline auto log_note() {
#if LOG_LEVEL >= 3
		return Log_real{};
#else
		return Log_dummy{};
#endif
	}
	inline auto log_debug() {
#if LOG_LEVEL >= 2
		return Log_real{};
#else
		return Log_dummy{};
#endif
	}
	inline auto log_all() {
#if LOG_LEVEL >= 1
		return Log_real{};
#else
		return Log_dummy{};
#endif
	}
};

#endif // LOG_H