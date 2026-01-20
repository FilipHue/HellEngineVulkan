#pragma once

// Internal
#include <hellengine/core/core.h>

namespace hellengine
{
	namespace math
	{
		constexpr u64 FNV_OFFSET_BASIS = 14695981039346656037ULL;
		constexpr u64 FNV_PRIME = 1099511628211ULL;

		inline u64 fnv1a_hash(const void* data, size_t length)
		{
			u64 hash = FNV_OFFSET_BASIS;
			const unsigned char* ptr = static_cast<const unsigned char*>(data);
			for (size_t i = 0; i < length; ++i)
			{
				hash ^= static_cast<u64>(ptr[i]);
				hash *= FNV_PRIME;
			}
			return hash;
		}

		template<typename T>
		inline u64 fnv1a_hash(const T& value)
		{
			return fnv1a_hash(&value, sizeof(T));
		}

		inline u64 fnv1a_hash_combine(u64 seed, u64 hash)
		{
			return seed ^ (hash + 0x9e3779b9 + (seed << 6) + (seed >> 2));
		}

	} // namespace math

} // namespace hellengine
