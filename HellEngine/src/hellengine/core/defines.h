#pragma once

// Internal
#include "platform.h"

// STL
#include <string>

#define _TO_STRING_IMPL(x) #x
#define TO_STRING(x) _TO_STRING_IMPL(x)

#define _CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) _CONCAT_IMPL(x, y)

#if defined(HE_COMPILER_MSVC)
	#define _FORCE_INLINE_IMPL __forceinline
	#define _DEBUG_BREAK_IMPL __debugbreak()
#elif defined(HE_COMPILER_GCC) || defined(HE_COMPILER_CLANG)
	#define _INLINE_IMPL __attribute__((always_inline))
	#define _DEBUG_BREAK_IMPL __builtin_trap()
#else
	#error "Compiler not supported"
#endif // HE_COMPILER_MSVC

#define FORCE_EXIT_IMPL(x) exit(x)
#define EXIT(x) FORCE_EXIT_IMPL(x)

#define FORCE_INLINE _FORCE_INLINE_IMPL
#define DEBUG_BREAK _DEBUG_BREAK_IMPL

#define _INLINE_IMPL inline
#define INLINE _INLINE_IMPL

#define BIT(x) (1 << (x))

#define HE_BIND_EVENTCALLBACK(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#define NO_OP (void)0

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define CLAMP(x, lower, higher) (MAX((lower), MIN((x), (higher))))

#define ALIGN(x, a) (((x) + ((a) - 1)) & ~((a) - 1))

#if defined(HE_COMPILER_MSVC)
	#define ALIGN_AS(size) __declspec(align(size))
#endif

#if defined(HE_PLATFORM_WINDOWS)
#define PATH_SEPARATOR '\\'
#elif defined(HE_PLATFORM_LINUX) || defined(HE_PLATFORM_MACOS)
#define PATH_SEPARATOR '/'
#endif

INLINE std::string ConcatPaths(std::initializer_list<std::string> parts) {
	std::string result;
	for (const auto& part : parts) {
		if (!result.empty() && result.back() != PATH_SEPARATOR)
			result += PATH_SEPARATOR;
		result += part;
	}
	return result;
}

#define CONCAT_PATHS(...) ConcatPaths({ __VA_ARGS__ })