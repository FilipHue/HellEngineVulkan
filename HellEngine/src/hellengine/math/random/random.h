#pragma once

// Internal
#include <hellengine/core/core.h>

namespace hellengine
{

	namespace math
	{

		class Random
		{
		public:
			HE_API static f32 GetFloat();
			HE_API static f64 GetFloat64();
			HE_API static i32 GetInt();
			HE_API static i64 GetInt64();
			HE_API static u32 GetUInt();
			HE_API static u64 GetUInt64();

			template<typename T>
			HE_API static T GetInRange(T min, T max);
		};

	} // namespace math

} // namespace hellengine