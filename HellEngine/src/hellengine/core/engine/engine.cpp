#include "hepch.h"
#include "engine.h"

// Internal
#include <hellengine/core/input/input.h>
#include <hellengine/core/logging/logger.h>

namespace hellengine
{

	namespace core
	{

		void Engine::Init()
		{
			Logger::Init();
		}

		void Engine::Shutdown()
		{
			Logger::Shutdown();
		}

		void Engine::Run(Application& application)
		{
			application.Setup(application.m_configuration);

			m_last_frame_time = m_timer.GetElapsedTime();
			while (application.IsRunning())
			{
				if (!application.IsSuspended())
				{
					m_frame_time = m_timer.GetElapsedTime();
					f32 delta_time = m_frame_time - m_last_frame_time;
					m_last_frame_time = m_frame_time;

					Input::Update();

					application.OnProcessUpdate(delta_time);
					application.m_backend->DrawFrame();

					//application.OnRenderBegin();
					//application.OnRenderUpdate();
					//application.OnRenderEnd();

					m_frame_count++;

					m_fps_timer += delta_time;
					if (m_fps_timer >= 1.0f)
					{
						application.SetWindowTitle(("Sandbox | FPS: " + std::to_string(1.0f / delta_time)).c_str());

						m_fps_timer = 0.0f;
					}
				}

				application.OnPollEvents();
			}

			application.Cleanup();
		}

		f32 Engine::GetTimeElapsed()
		{
			return m_timer.GetElapsedTime();
		}

		Engine& Engine::GetInstance()
		{
			static Engine instance;
			return instance;
		}

	} // namespace core

} // namespace hellengine