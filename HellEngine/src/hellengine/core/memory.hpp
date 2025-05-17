#pragma once

// Internal
#include "defines.h"

// Standard
#include <memory>

template <typename T>
using Unique = std::unique_ptr<T>;

template <typename T>
using Shared = std::shared_ptr<T>;

template<typename T, typename ... Args>
FORCE_INLINE Unique<T> MakeUnique(Args&& ... args)
{
	return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T, typename ... Args>
FORCE_INLINE Shared<T> MakeShared(Args&& ... args)
{
	return std::make_shared<T>(std::forward<Args>(args)...);
}

INLINE void* AlignedAllocation(size_t size, size_t alignment)
{
	void* data = nullptr;
#if defined(HE_COMPILER_MSVC)
	data = _aligned_malloc(size, alignment);
#else
	int res = posix_memalign(&data, alignment, size);
	if (res != 0)
		data = nullptr;
#endif
	return data;
}

INLINE void AlignedFree(void* data)
{
#if defined(HE_COMPILER_MSVC)
	_aligned_free(data);
#else
	free(data);
#endif
}