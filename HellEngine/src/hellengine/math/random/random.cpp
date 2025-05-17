#include "hepch.h"
#include "random.h"

namespace hellengine
{
	namespace math
	{

		static std::random_device s_random_device;
		static std::mt19937_64 s_engine(s_random_device());

		static std::uniform_real_distribution<f32> s_real_distribution(0.0f, 1.0f);
		static std::uniform_real_distribution<f64> s_real_distribution_64(0.0, 1.0);

		static std::uniform_int_distribution<i32> s_int_distribution_max(std::numeric_limits<i32>::min(), std::numeric_limits<i32>::max());
		static std::uniform_int_distribution<i64> s_int_distribution_64_max(std::numeric_limits<i64>::min(), std::numeric_limits<i64>::max());

		static std::uniform_int_distribution<u32> s_uint_distribution_max(std::numeric_limits<u32>::min(), std::numeric_limits<u32>::max());
		static std::uniform_int_distribution<u64> s_uint_distribution_64_max(std::numeric_limits<u64>::min(), std::numeric_limits<u64>::max());

		f32 Random::GetFloat() { return s_real_distribution(s_engine); }
		f64 Random::GetFloat64() { return s_real_distribution_64(s_engine); }
		i32 Random::GetInt() { return s_int_distribution_max(s_engine); }
		i64 Random::GetInt64() { return s_int_distribution_64_max(s_engine); }
		u32 Random::GetUInt() { return s_uint_distribution_max(s_engine); }
		u64 Random::GetUInt64() { return s_uint_distribution_64_max(s_engine); }

		template HE_API f32 Random::GetInRange(f32 min, f32 max);
		template HE_API f64 Random::GetInRange(f64 min, f64 max);
		template HE_API i32 Random::GetInRange(i32 min, i32 max);
		template HE_API i64 Random::GetInRange(i64 min, i64 max);
		template HE_API u32 Random::GetInRange(u32 min, u32 max);

		template<typename T>
		T Random::GetInRange(T min, T max)
		{
			if constexpr (std::is_floating_point<T>::value)
			{
				std::uniform_real_distribution<T> distribution(min, max);
				return distribution(s_engine);
			}
			else if constexpr (std::is_integral<T>::value)
			{
				std::uniform_int_distribution<T> distribution(min, max);
				return distribution(s_engine);
			}
			else
			{
				HE_ASSERT(false, "Random::GetInRange: Unsupported type");
				return T();
			}
		}

	} // namespace math

} // namespace hellengine