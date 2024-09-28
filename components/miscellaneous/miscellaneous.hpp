/**
 * @file miscellaneous.hpp
 * @author Sylvain Mosnier (sylvain.mosnier@energymachines.com)
 * @brief Miscellaneous definitions of code, some usefull functions for debug and some usefull template
 * @version 0.1
 * @date 2022-04-28
 *
 *  Copyright (c) 2022 Energy Machines Wind
 *
 */
#ifndef MISCELLANEOUS_HPP__
#define MISCELLANEOUS_HPP__
#include "sdkconfig.h"
#include <xtensa/hal.h>
#include <esp_log.h>

#include <functional>
#include <utility>
#include <iterator>
#include <limits>

/**
 * @brief Macro for memory barrier : forbid compiler to switch order of two consecutives intructions
 *
 */
#define barrier(explanationWhyYouAreUsingIt) __asm__("" ::: "memory")

/**
 * @brief Macro definition for optimization flags
 *
 */
#define OPTIMIZE_SPEED __attribute__((optimize("-Ofast"))) __attribute__((optimize("-fno-rtti"))) ///< Speed optimization : should not be used with floating point number as Ofast violate IEEE754
#define OPTIMIZE_SPEED_O3 __attribute__((optimize("-O3"))) __attribute__((optimize("-fno-rtti")))
#define USED_FREQUENTLY __attribute__((hot))		   ///< Memory access optimisation : the object will be placed in hot memory with faster access
#define OPTIMIZE_SIZE __attribute__((optimize("-Os"))) ///< Size optimization
#define FORCE_INLINE __attribute__((always_inline))	   ///< Try to force inlining : doesn't mean GCC will inline

/**
 * @brief Undefine Macro for branch optimisation, prefer template
 *
 */
#ifdef likely
#undef likely
#endif
#ifdef unlikely
#undef unlikely
#endif
/**
 * @brief Branch optimization template
 *
 * @tparam T type of argument
 * @param t
 * @return bool
 */
template <typename T>
inline FORCE_INLINE bool likely(T t)
{
	static_assert(std::is_convertible_v<T, bool>);
	return __builtin_expect(!!(t), 1);
};

template <typename T>
inline FORCE_INLINE bool unlikely(T t)
{
	static_assert(std::is_convertible_v<T, bool>);
	return __builtin_expect(!!(t), 0);
};

/**
 * @brief Return the size of a statically declared array
 *
 * @tparam T type of the array
 * @tparam n size of the array ( a priori not know)
 * @return constexpr std::size_t
 */
template <typename T, std::size_t n>
constexpr std::size_t size(const T (&)[n]) { return n; };

namespace misc
{
	/**
	 * @brief Limitied equivalent to consteval in C++20
	 *
	 * @tparam V  must be an integral type or un enum
	 */
	template <auto V>
	static constexpr auto force_consteval = V;

	/**
	 * @brief Abs constexpr function
	 *
	 * @tparam T
	 * @param x
	 * @return constexpr T
	 */
	template <typename T>
	inline constexpr T abs(const T &x)
	{
		// static_assert(std::is_arithmetic_v<T>, " T must be an arithmetic type");
		return (x < T(0)) ? -x : x;
	};

	/**
	 * @brief Positive Linear rectifier (return value if value is positive, 0 otherwise)
	 *
	 * @tparam T
	 * @param x value
	 * @return constexpr T
	 */
	template <typename T>
	inline constexpr T positiv_relu(const T &x)
	{
		return (x < T(0) ? T(0) : x);
	};

	/**
	 * @brief Negative Linear rectifier (return value if value is negative, 0 otherwise)
	 *
	 * @tparam T
	 * @param x value
	 * @return constexpr T
	 */
	template <typename T>
	inline constexpr T negativ_relu(const T &x)
	{
		return (x < T(0) ? x : T(0));
	};

	/**
	 * @brief Return the closest value that is in range
	 * 		  Object has to implement < operator
	 * 		  If  (min > max) -> undefined behaviour
	 *
	 * @tparam T
	 * @param val value to range
	 * @param min minimum bound
	 * @param max maximum bound
	 * @return constexpr T
	 */
	template <typename T>
	inline constexpr T range(const T &val, const T &min, const T &max)
	{
		static_assert(sizeof(min < val) >= 1, " x<y operation not supported for T type");
		return (min < val) ? ((val < max) ? val : max) : min;
		// it is actually better than the version below as the probability of (min < val) to be true is lower than (min <= val) to be true
		// thus, the second test might be skipped more often
		// for the second test: is does not really matter as the options are the same
		// return (min <= val) ? ((val < max) ? val : max) : min;
	};

	/**
	 * @brief Check value is inside the range (bounds are included)
	 *	      Object has to implement < operator
	 * 		  If  (min > max) -> undefined behaviour
	 *
	 * @tparam T
	 * @param val
	 * @param min
	 * @param max
	 * @return constexpr T
	 */
	template <typename T>
	inline constexpr bool isInRange(const T &val, const T &min, const T &max)
	{
		static_assert(sizeof(min < val) >= 1, " x<y operation not supported for T type");
		return (val < min) ? false : ((max < val) ? false : true);
	};

	/**
	 * @brief Check if an integral type is a power of 2 (validated)
	 *
	 * @tparam T type of integral type
	 * @param v
	 * @return true if it is a power of 2
	 */
	template <typename T>
	inline constexpr bool isPowerOf2(const T value)
	{
		// static_assert(std::is_integral_v<T>, "T must be an integral type");
		return value && ((value & (value - 1)) == 0);
	};
	/**
	 * @brief Constexpr to calculate the floor value of log2 aka log2(17)==log2(16)==4
	 *
	 * @tparam T
	 * @param n
	 * @return constexpr T
	 */
	template <typename T>
	inline constexpr T log2(const T n)
	{
		static_assert(std::is_integral_v<T>, "T must be an integral type");
		return ((n < 2) ? T(0) : T(1) + misc::log2(n / 2));
	};

	/**
	 * @brief Constexpr to calculate the next power of 2 greater or equal to the number
	 *
	 * @tparam T
	 * @param v
	 * @return constexpr T
	 */
	template <typename T>
	inline constexpr T nextPowerOf2(const T v)
	{
		static_assert(std::is_integral_v<T>, "T must be an integral type");
		return (1 << (misc::log2(v) + static_cast<T>(!misc::isPowerOf2(v))));
	};

	// template <typename T, typename U, std::size_t N>
	// constexpr T accumulate(U const (&A)[N], int const i = 0)
	//{
	//	return (i < N) ? misc::accumulate(A, i + 1) + misc::abs(static_cast<T>(A[i])) : T(0);
	// }

	namespace
	{
		template <typename T>
		constexpr auto _default_unaryfunction(const T &a) { return a; };
		template <typename T, typename U>
		constexpr auto _default_cmp_less(const T &a, const U &b) { return (a < b); };
		template <typename T, typename U>
		constexpr auto _default_cmp_grtr(const T &a, const U &b) { return (b < a); };
	};

	template <typename T, typename U, std::size_t N, typename unaryfunction = decltype(misc::_default_unaryfunction<U>)>
	inline constexpr T accumulate(U const (&A)[N], unaryfunction f = misc::_default_unaryfunction<U>)
	{
		T sum(T(0));
		for (int i = 0; i < N; ++i)
		{
			auto temp = f(A[i]);

			sum += static_cast<T>(temp);
		}
		return sum;
	};

	template <typename InputIt, typename T, typename unaryfunction = decltype(misc::_default_unaryfunction<T>)>
	inline constexpr T accumulate(InputIt first, InputIt last, T init, unaryfunction f = misc::_default_unaryfunction<T>)
	{
		for (; first != last; ++first)
		{
			init = f(std::move(init), *first);
		}
		return init;
	};

	template <typename T, std::size_t N, typename cmp_function, typename unaryfunction = decltype(misc::_default_unaryfunction<T>)>
	static constexpr T generalized_cmp_element(T const (&A)[N], cmp_function cmp, unaryfunction f = misc::_default_unaryfunction<T>)
	{
		// static_assert(std::is_same_v<std::invoke_result_t<decltype(f),T>, T>, "unary function should return T type");
		// static_assert(std::is_convertible_v<std::invoke_result_t<decltype(cmp),T,std::invoke_result_t<decltype(f),T>>, bool>, "compare function result should be convertible to bool");
		T largest(f(A[0]));
		if (N == 1)
			return largest;

		for (int i = 1; i < N; ++i)
		{
			auto temp = f(A[i]);
			if (cmp(largest, temp))
			{
				largest = temp;
			}
		}
		return largest;
	};

	template <typename T, std::size_t N, typename unaryfunction = decltype(misc::_default_unaryfunction<T>)>
	inline constexpr T max_element(T const (&A)[N], unaryfunction f = misc::_default_unaryfunction<T>)
	{
		return misc::generalized_cmp_element(A, misc::_default_cmp_less<T, T>, f);
	};

	template <typename T, std::size_t N, typename unaryfunction = decltype(misc::_default_unaryfunction<T>)>
	inline constexpr T min_element(T const (&A)[N], unaryfunction f = misc::_default_unaryfunction<T>)
	{
		return misc::generalized_cmp_element(A, misc::_default_cmp_grtr<T, T>, f);
	};

		/**
		 * @brief Unroll a functor
		 *
		 * @tparam N number of iteration to unroll
		 * @tparam Fn type of the functor
		 * @tparam std::enable_if<std::is_same_v<std::invoke_result_t<Fn, decltype(N)>, void>>::type  is true if return type of Fn(decltype(N)) is void
		 * @param fn functor to unroll
		 */
#if defined(__cplusplus) && (__cplusplus > 201703L)
	template <size_t N, typename Fn>
	requires(N >= 1) && requires(Fn fn, size_t i)
	{
		{
			fn(i)
			} -> std::same_as<void>;
	}
	inline OPTIMIZE_SPEED_O3 FORCE_INLINE void unroll(Fn fn)
	{
		auto unroll_n = [&]<size_t... Indices>(std::index_sequence<Indices...>)
		{
			(fn(Indices), ...);
		};
		unroll_n(std::make_index_sequence<N>());
	};
#else
	template <size_t N, typename Fn, typename std::enable_if<std::is_same_v<std::invoke_result_t<Fn, decltype(N)>, void>, bool>::type = true>
	inline FORCE_INLINE OPTIMIZE_SPEED_O3 void unroll(Fn fn)
	{
		static_assert(N >= 1, "N must be strictly positive");																	 // check that N>=1
		static_assert(std::is_same_v<std::invoke_result_t<Fn, decltype(N)>, void>, "Bad return type for functor, must be void"); // check that Fn(index) return void
		auto unroll_n = [&]<size_t... Indices>(std::index_sequence<Indices...>)
		{
			(fn(Indices), ...);
		};
		unroll_n(std::make_index_sequence<N>());
	};
#endif

/**
 * @brief Unroll a functor
 *
 * @tparam N number of iteration to unroll
 * @tparam Fn type of the functor
 * @tparam std::enable_if<std::is_same_v<std::invoke_result_t<Fn, void>, void>>::type  is true if return type of Fn(void) is void
 * @param fn functor to unroll
 */
#if defined(__cplusplus) && (__cplusplus > 201703L)
	template <size_t N, typename Fn>
	requires(N >= 1) && requires(Fn fn)
	{
		{
			fn()
			} -> std::same_as<void>;
	}
	inline FORCE_INLINE OPTIMIZE_SPEED_O3 void unroll(Fn fn)
	{
		auto unroll_n = [&]<size_t... Indices>(std::index_sequence<Indices...>)
		{
			return ((Indices, fn()), ...);
		};
		unroll_n(std::make_index_sequence<N>());
	};
#else
	template <size_t N, typename Fn, typename std::enable_if<std::is_same_v<std::invoke_result_t<Fn, void>, void>, bool>::type = true>
	inline FORCE_INLINE OPTIMIZE_SPEED_O3 void unroll(Fn fn)
	{
		static_assert(N >= 1, "N must be strictly positive"); // check that N>=1
		auto unroll_n = [&]<size_t... Indices>(std::index_sequence<Indices...>)
		{
			return ((Indices, fn()), ...);
		};
		unroll_n(std::make_index_sequence<N>());
	};
#endif

// fn(index) return type is convertible to bool
/**
 * @brief Unroll a functor
 *
 * @tparam N number of iteration to unroll
 * @tparam Fn type of the functor
 * @tparam std::enable_if<std::is_convertible_v<std::invoke_result_t<Fn, decltype(N)>, bool>>::type  is true if return type of Fn(decltype(N)) can be converted to bool
 * @param fn functor to unroll
 */
#if defined(__cplusplus) && (__cplusplus > 201703L)
	template <size_t N, typename Fn>
	requires(N >= 1) && requires(Fn fn, size_t i)
	{
		{
			fn(i)
			} -> std::convertible_to<bool>;
	}
	inline FORCE_INLINE OPTIMIZE_SPEED_O3 bool unroll(Fn fn)
	{
		auto unroll_n = [&]<size_t... Indices>(std::index_sequence<Indices...>)->bool
		{
			return (fn(Indices) && ...);
		};
		return unroll_n(std::make_index_sequence<N>());
	};
#else
	template <size_t N, typename Fn, typename std::enable_if<std::is_convertible_v<std::invoke_result_t<Fn, decltype(N)>, bool>, bool>::type = true>
	inline FORCE_INLINE OPTIMIZE_SPEED_O3 bool unroll(Fn fn)
	{
		static_assert(N >= 1, "N must be strictly positive"); // check that N>=1
		auto unroll_n = [&]<size_t... Indices>(std::index_sequence<Indices...>)->bool
		{
			return (fn(Indices) && ...);
		};
		return unroll_n(std::make_index_sequence<N>());
	};
#endif

/**
 * @brief Unroll a functor
 *
 * @tparam N number of iteration to unroll
 * @tparam Fn type of the functor
 * @tparam std::enable_if<std::is_convertible_v<std::invoke_result_t<Fn, void>, bool>>::type  is true if return type of Fn(void) can be converted to bool
 * @param fn functor to unroll
 */
#if defined(__cplusplus) && (__cplusplus > 201703L)
	template <size_t N, typename Fn>
	requires(N >= 1) && requires(Fn fn)
	{
		{
			fn()
			} -> std::convertible_to<bool>;
	}
	inline FORCE_INLINE OPTIMIZE_SPEED_O3 bool unroll(Fn fn)
	{
		auto unroll_n = [&]<size_t... Indices>(std::index_sequence<Indices...>)->bool
		{
			return ((Indices, fn()) && ...);
		};
		return unroll_n(std::make_index_sequence<N>());
	};
#else
	template <size_t N, typename Fn, typename std::enable_if<std::is_convertible_v<std::invoke_result_t<Fn, void>, bool>>::type = true>
	inline FORCE_INLINE OPTIMIZE_SPEED_O3 bool unroll(Fn fn)
	{
		static_assert(N >= 1, "N must be strictly positive"); // check that N>=1
		auto unroll_n = [&]<size_t... Indices>(std::index_sequence<Indices...>)->bool
		{
			return ((Indices, fn()) && ...);
		};
		return unroll_n(std::make_index_sequence<N>());
	};
#endif

/**
 * @brief "foreach" partial unrolling implementation using iterators
 *
 * @tparam N size of the step
 * @tparam RandomIt type of the iterator
 * @tparam UnaryFunction Type of the functor
 * @param begin begin iterator for partial unrolling
 * @param end end iterator for partial unrolling
 * @param fn functor to unroll
 * @return RandomIt
 */
#if defined(__cplusplus) && (__cplusplus > 201703L)
	template <std::size_t N, typename RandomIt, typename UnaryFunction>
	requires std::random_access_iterator<RandomIt> && requires(UnaryFunction fn, typename std::iterator_traits<RandomIt>::value_type elem) { {fn(elem)}; }
	inline RandomIt unroll_for_each(RandomIt begin, RandomIt end, UnaryFunction fn)
	{
		RandomIt &it = begin;
		if constexpr (N > 1)
			for (; it + N <= end; it += N)
				unroll<N>([&](size_t i)
						  { fn(it[i]); });
		for (; it < end; ++it)
			fn(*begin);
		return it;
	};
#else
	template <std::size_t N, typename RandomIt, typename UnaryFunction>
	inline OPTIMIZE_SPEED_O3 RandomIt unroll_for_each(RandomIt begin, RandomIt end, UnaryFunction fn)
	{
		static_assert(std::is_same_v<std::iterator_traits<RandomIt>::iterator_category, std::random_access_iterator_tag>); // check that RandomIt is a random access iterator
		static_assert(std::is_invocable_v<UnaryFunction, RandomIt>);
		RandomIt &it = begin;
		if constexpr (N > 1)
			for (; it + N <= end; it += N)
				unroll<N>([&](size_t i)
						  { fn(it[i]); });
		for (; it < end; ++it)
			fn(*begin);
		return it;
	};
#endif

/**
 * @brief "for" partial unrolling implementation using index
 *
 * @tparam N size of the step
 * @tparam RandomIt type of the iterator
 * @tparam UnaryFunction Type of the functor
 * @param begin begin iterator for partial unrolling
 * @param end end iterator for partial unrolling
 * @param fn functor to unroll
 * @return RandomIt
 */
#if defined(__cplusplus) && (__cplusplus > 201703L)
// template <std::size_t N, typename RandomIt, typename UnaryFunction>
// requires std::random_access_iterator<RandomIt> && requires(UnaryFunction fn, typename std::iterator_traits<RandomIt>::value_type elem) { {fn(elem)}; }
// inline RandomIt unroll_for_index(RandomIt begin, RandomIt end, UnaryFunction fn)
//{
//	RandomIt &it = begin;
//	if constexpr (N > 1)
//		for (; it + N <= end; it += N)
//			unroll<N>([&](size_t i)
//					  { fn(it[i]); });
//	for (; it < end; ++it)
//		fn(*begin);
//	return it;
// }
#else
	template <std::size_t N, typename IndexIt, typename UnaryFunction, typename std::enable_if<std::is_invocable_v<UnaryFunction, IndexIt>, bool>::type = true>
	inline OPTIMIZE_SPEED_O3 FORCE_INLINE IndexIt unroll_for_index(IndexIt begin, IndexIt end, UnaryFunction fn)
	{
		static_assert(std::is_integral_v<IndexIt> && !std::is_same_v<IndexIt, bool>);
		// static_assert(std::is_same_v<std::iterator_traits<IndexIt>::iterator_category, std::random_access_iterator_tag>); // check that RandomIt is a random access iterator
		// static_assert(std::is_invocable_v<UnaryFunction, IndexIt>);
		if constexpr (N > 1)
		{
			for (; begin + N <= end; begin += N)
			{
				unroll<N>([&](size_t i)
						  { fn(begin + i); });
			}
		}
		for (; begin < end; ++begin)
			fn(begin);
		return begin;
	};
	template <std::size_t N, typename IndexIt, typename UnaryFunction, typename std::enable_if<std::is_invocable_v<UnaryFunction, IndexIt, IndexIt>, bool>::type = true>
	inline OPTIMIZE_SPEED_O3 FORCE_INLINE IndexIt unroll_for_index(IndexIt begin, IndexIt end, UnaryFunction fn)
	{
		static_assert(std::is_integral_v<IndexIt> && !std::is_same_v<IndexIt, bool>);
		// static_assert(std::is_same_v<std::iterator_traits<IndexIt>::iterator_category, std::random_access_iterator_tag>); // check that RandomIt is a random access iterator
		// static_assert(std::is_invocable_v<UnaryFunction, IndexIt,IndexIt>);
		if constexpr (N > 1)
		{
			for (; begin + N <= end; begin += N)
			{
				unroll<N>([&](size_t i)
						  { fn(begin, i); });
			}
		}
		for (; begin < end; ++begin)
			fn(begin, 0);
		return begin;
	};
#endif

	//#if defined(__cplusplus) && (__cplusplus > 201703L)
	//#else
	// template <std::size_t N, std::size_t begin, std::size_t end, typename UnaryFunction>
	// inline size_t unroll_for_index_fast(UnaryFunction fn)
	//{
	//	static_assert(begin<=end);
	//	//static_assert(std::is_integral_v<IndexIt>&& !std::is_same_v<IndexIt,bool>);
	//	//static_assert(std::is_same_v<std::iterator_traits<IndexIt>::iterator_category, std::random_access_iterator_tag>); // check that RandomIt is a random access iterator
	//	static_assert(std::is_invocable_v<UnaryFunction, IndexIt>);
	//	if constexpr (N > 1)
	//		for (; begin + N <= end; begin += N)
	//			unroll<N>([&](size_t i)
	//					  { fn(begin+i); });
	//	for (; begin < end; ++begin)
	//		fn(begin);
	//	return begin;
	//}
	//#endif
	template <typename Function>
	inline FORCE_INLINE auto tick_measure(Function fnct)
	{
		static_assert((std::is_invocable_v<Function>)&&(std::is_same_v<std::invoke_result_t<Function>, void>));
		volatile auto counter_begin = xthal_get_ccount();
		fnct(); // actual function to launch
		volatile auto counter_end = xthal_get_ccount();
		auto temp = counter_end - counter_begin;
		if (unlikely(counter_end < counter_begin))
		{
			temp = UINT32_MAX - temp;
		}
		return temp;
	};
};

/** @cond */

// void IRAM_ATTR OPTIMIZE_SPEED_O3 esp_log_buffer_hexdump(const char *tag, const char *buffer, uint16_t buff_len);
void esp_log_buffer_hexdump(const char *tag, const char *buffer, uint16_t buff_len);
/**
 * @brief Macro to color the output
 */
#if CONFIG_LOG_COLORS
#define LOG_FORMAT_ISR_SAFE(format)       \
	LOG_COLOR(CONFIG_MISC_HEX_BUFF_COLOR) \
	"%s: " format LOG_RESET_COLOR "\n"
#else
#define LOG_FORMAT_ISR_SAFE(format) "%s: " format "\n"
#endif
/** @endcond */

/**
 * @brief Macro to print buffer : can be called from ISR context
 * @param tag to describe the origine of the print
 * @param buffer pointer on buffer
 * @param buff_len length in byte
 * @param log_level level to print to the console
 */
#define ESP_LOG_BUFFER_HEX_SAFE(tag, buffer, buff_len, log_level) \
	do                                                            \
	{                                                             \
		if (_ESP_LOG_EARLY_ENABLED(log_level))                    \
		{                                                         \
			esp_log_buffer_hexdump(tag, buffer, buff_len);        \
		}                                                         \
	} while (0)

#if defined(__cplusplus) && (__cplusplus > 201703L)
#define ESP_LOGE_SAFE(tag, format, ...) ESP_DRAM_LOG_IMPL_SAFE(tag, format, ESP_LOG_ERROR, E __VA_OPT__(, ) __VA_ARGS__)
#define ESP_LOGW_SAFE(tag, format, ...) ESP_DRAM_LOG_IMPL_SAFE(tag, format, ESP_LOG_WARN, W __VA_OPT__(, ) __VA_ARGS__)
#define ESP_LOGI_SAFE(tag, format, ...) ESP_DRAM_LOG_IMPL_SAFE(tag, format, ESP_LOG_INFO, I __VA_OPT__(, ) __VA_ARGS__)
#define ESP_LOGD_SAFE(tag, format, ...) ESP_DRAM_LOG_IMPL_SAFE(tag, format, ESP_LOG_DEBUG, D __VA_OPT__(, ) __VA_ARGS__)
#define ESP_LOGV_SAFE(tag, format, ...) ESP_DRAM_LOG_IMPL_SAFE(tag, format, ESP_LOG_VERBOSE, V __VA_OPT__(, ) __VA_ARGS__)
#else // !(defined(__cplusplus) && (__cplusplus >  201703L))
#define ESP_LOGE_SAFE(tag, format, ...) ESP_DRAM_LOG_IMPL_SAFE(tag, format, ESP_LOG_ERROR, E, ##__VA_ARGS__)
#define ESP_LOGW_SAFE(tag, format, ...) ESP_DRAM_LOG_IMPL_SAFE(tag, format, ESP_LOG_WARN, W, ##__VA_ARGS__)
#define ESP_LOGI_SAFE(tag, format, ...) ESP_DRAM_LOG_IMPL_SAFE(tag, format, ESP_LOG_INFO, I, ##__VA_ARGS__)
#define ESP_LOGD_SAFE(tag, format, ...) ESP_DRAM_LOG_IMPL_SAFE(tag, format, ESP_LOG_DEBUG, D, ##__VA_ARGS__)
#define ESP_LOGV_SAFE(tag, format, ...) ESP_DRAM_LOG_IMPL_SAFE(tag, format, ESP_LOG_VERBOSE, V, ##__VA_ARGS__)
#endif // !(defined(__cplusplus) && (__cplusplus >  201703L))

/** @cond */
#define _ESP_LOG_DRAM_LOG_FORMAT_SAFE(letter, format) LOG_FORMAT_ISR_SAFE(format)

#if defined(__cplusplus) && (__cplusplus > 201703L)
#define ESP_DRAM_LOG_IMPL_SAFE(tag, format, log_level, log_tag_letter, ...)                                        \
	do                                                                                                             \
	{                                                                                                              \
		if (_ESP_LOG_EARLY_ENABLED(log_level))                                                                     \
		{                                                                                                          \
			esp_rom_printf(_ESP_LOG_DRAM_LOG_FORMAT_SAFE(log_tag_letter, format), tag __VA_OPT__(, ) __VA_ARGS__); \
		}                                                                                                          \
	} while (0)
#else // !(defined(__cplusplus) && (__cplusplus >  201703L))
#define ESP_DRAM_LOG_IMPL_SAFE(tag, format, log_level, log_tag_letter, ...)                            \
	do                                                                                                 \
	{                                                                                                  \
		if (_ESP_LOG_EARLY_ENABLED(log_level))                                                         \
		{                                                                                              \
			esp_rom_printf(_ESP_LOG_DRAM_LOG_FORMAT_SAFE(log_tag_letter, format), tag, ##__VA_ARGS__); \
		}                                                                                              \
	} while (0)
#endif // !(defined(__cplusplus) && (__cplusplus >  201703L))
/** @endcond */

#endif /*MISCELLANEOUS_HPP__*/