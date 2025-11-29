#include "hepch.h"
#include "timer.h"

namespace hellengine
{

	namespace core
	{

		Timer::Timer() : m_start_time_point(std::chrono::high_resolution_clock::now()), m_elapsed_time(0.0f), m_stopped(false)
		{
			NO_OP;
		}

		void Timer::Start()
		{
			if (m_stopped)
			{
				m_start_time_point = std::chrono::high_resolution_clock::now();
				m_stopped = false;
			}
		}

		void Timer::Stop()
		{
			if (!m_stopped)
			{
				auto end_time_point = std::chrono::high_resolution_clock::now();
				m_elapsed_time = std::chrono::duration<float>(end_time_point - m_start_time_point).count();
				m_stopped = true;
			}
		}

		void Timer::Reset()
		{
			m_start_time_point = std::chrono::high_resolution_clock::now();
			m_elapsed_time = 0.0f;
			m_stopped = false;
		}

		f32 Timer::GetElapsedTime()
		{
			if (m_stopped)
			{
				return m_elapsed_time;
			}

			auto end_time_point = std::chrono::high_resolution_clock::now();
			m_elapsed_time = std::chrono::duration<float>(end_time_point - m_start_time_point).count();

			return m_elapsed_time;
		}

		b8 Timer::IsStopped() const
		{
			return m_stopped;
		}

	} // namespace core

} // namespace hellengine