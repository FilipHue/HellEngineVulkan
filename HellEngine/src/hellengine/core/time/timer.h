#pragma once

// Internal
#include <hellengine/core/api.h>
#include <hellengine/core/typedefs.h>

// Standard
#include <chrono>

namespace hellengine
{
	namespace core
	{
		class Timer
		{
		public:
			HE_API Timer();
			HE_API ~Timer() = default;

			HE_API void Start();
			HE_API void Stop();
			HE_API void Reset();
			HE_API f32 GetElapsedTime();
			HE_API b8 IsStopped() const;

		private:
			std::chrono::time_point<std::chrono::high_resolution_clock> m_start_time_point;
			f32 m_elapsed_time;
			b8 m_stopped;
		};
	}
}