#pragma once

// Internal
#include <hellengine/core/application/application.h>
#include <hellengine/core/time/timer.h>
#include <hellengine/core/api.h>

namespace hellengine
{

	namespace core
	{

		class Engine final
		{
		public:
			HE_API void Init();
			HE_API void Shutdown();

			HE_API void Run(Application& application);

			HE_API const Application& GetApplication() const { return *m_application; }
			HE_API f32 GetFrameTime() const;
			HE_API f32 GetTimeElapsed();

			HE_API static Engine& GetInstance();

		private:
			Engine() = default;

		private:
			Application* m_application{ nullptr };

			f32 m_frame_time{ 0.0f };
			f32 m_last_frame_time{ 0.0f };
			u64 m_frame_count{ 0 };

			f32 m_fps_timer{ 0.0f };
			u64 m_fps_timer_start_frame{ 0 };

			Timer m_timer;
		};

	} // namespace core

} // namespace hellengine