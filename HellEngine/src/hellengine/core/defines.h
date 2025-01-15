#pragma once

// Internal
#include "platform.h"

#define _TO_STRING_IMPL(x) #x
#define TO_STRING(x) _TO_STRING_IMPL(x)

#if defined(HE_COMPILER_MSVC)
	#define _FORCE_INLINE_IMPL __forceinline
	#define _DEBUG_BREAK_IMPL __debugbreak()
#elif defined(HE_COMPILER_GCC) || defined(HE_COMPILER_CLANG)
	#define _INLINE_IMPL __attribute__((always_inline))
	#define _DEBUG_BREAK_IMPL __builtin_trap()
#else
	#error "Compiler not supported"
#endif

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