#pragma once

// Internal
#include <hellengine/core/core.h>

// STL
#include <numeric>

namespace hellengine
{

	namespace math
	{

		constexpr i64 lcm(i64 a, i64 b) {
			return (a / std::gcd(a, b)) * b;
		}

		template <u64 N>
		constexpr i64 lcm_array(const std::array<i64, N>& nums, i64 index = 0, i64 result = 1) {
			return (index == N) ? result : lcm_array<N>(nums, index + 1, lcm(result, nums[index]));
		}

	} // namespace math
	 
} // namespace hellengine