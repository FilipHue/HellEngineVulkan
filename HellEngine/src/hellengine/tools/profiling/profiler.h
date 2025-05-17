#pragma once

// Internal
#include "hellengine/core/api.h"

// Standard
#include <iostream>
#include <string>
#include <fstream>
#include <mutex>
#include <algorithm>
#include <chrono>

namespace hellengine
{

	namespace tools
	{

		constexpr auto CONSOLE_OUTPUT = "console";

		// TO DO: Make it non-blocking (async with bufferig)
		class ProfilerManager
		{
		public:
			struct ProfilingResult {
				std::string m_timer_name;
				long long m_start, m_end;
				size_t m_thread_id;
			};

		public:
			HE_API ProfilerManager(ProfilerManager& other) = delete;
			HE_API ~ProfilerManager() {
				EndSession();
			};
			HE_API void operator=(const ProfilerManager& other) = delete;

			HE_API void BeginSession(const std::string& session_name, const std::string& output = "result.json")
			{
				if (m_active_session) {
					EndSession();
				}

				m_console_output = output == CONSOLE_OUTPUT;
				m_session_name = session_name;
				m_active_session = true;

				if (m_console_output) {
					WriteHeaderConsole();
				}
				else {
					m_output_stream.open(output);
					WriteHeader();
				}
			}
			HE_API void EndSession()
			{
				if (!m_active_session) {
					return;
				}

				if (m_console_output) {
					WriteFooterConsole();
				}
				else {
					WriteFooter();
					m_output_stream.close();
				}

				m_active_session = false;
				m_timers_count = 0;
			}

			HE_API void WriteHeader()
			{
				//std::lock_guard<std::mutex> lock(m_lock);
				m_output_stream << "{\"otherData\": {},\"traceEvents\": [";
			}
			HE_API void WriteHeaderConsole()
			{
				//std::lock_guard<std::mutex> lock(m_lock);
				std::cout << "Starting session: " << m_session_name << "\n";
			}

			HE_API void WriteProfile(const ProfilingResult& result)
			{
				std::lock_guard<std::mutex> lock(m_lock);

				if (m_timers_count++ > 0) {
					m_output_stream << ",";
				}

				std::string timer_name = result.m_timer_name;
				std::replace(timer_name.begin(), timer_name.end(), '"', '\"');

				m_output_stream << "{";
				m_output_stream << "\"cat\":\"function\",";
				m_output_stream << "\"dur\":" << (result.m_end - result.m_start) << ',';
				m_output_stream << "\"name\":\"" << timer_name << "\",";
				m_output_stream << "\"ph\":\"X\",";
				m_output_stream << "\"pid\":0,";
				m_output_stream << "\"tid\":" << result.m_thread_id << ",";
				m_output_stream << "\"ts\":" << result.m_start;
				m_output_stream << "}";
			}
			HE_API void WriteProfileCosole(const ProfilingResult& result)
			{
				std::lock_guard<std::mutex> lock(m_lock);
				std::cout << "Timer: " << result.m_timer_name << " - Duration: " << (result.m_end - result.m_start) << "ms\n";
			}

			HE_API void WriteFooter()
			{
				//std::lock_guard<std::mutex> lock(m_lock);
				m_output_stream << "]}";
			}
			HE_API void WriteFooterConsole()
			{
				//std::lock_guard<std::mutex> lock(m_lock);
				std::cout << "Ending session: " << m_session_name << "\n";
			}

			HE_API bool IsConsoleOutput() const
			{
				return m_console_output;
			}

			HE_API static ProfilerManager& GetInstance();

		protected:
			ProfilerManager() = default;

		private:
			std::ofstream m_output_stream;
			std::string m_session_name = "None";
			bool m_active_session = false;
			bool m_console_output = false;
			int m_timers_count = 0;
			std::mutex m_lock;
		};

		class ProfilingTimer
		{
		public:
			HE_API ProfilingTimer(const std::string& timer_name)
				: m_result({ timer_name, 0, 0, 0 }), m_start_point_time(std::chrono::high_resolution_clock::now()) {};
			HE_API ~ProfilingTimer()
			{
				if (!m_stopped) {
					auto end_point_time = std::chrono::high_resolution_clock::now();

					m_result.m_start = m_start_point_time.time_since_epoch().count();
					m_result.m_end = end_point_time.time_since_epoch().count();
					m_result.m_thread_id = std::hash<std::thread::id>{}(std::this_thread::get_id());

					if (ProfilerManager::GetInstance().IsConsoleOutput())
						ProfilerManager::GetInstance().WriteProfileCosole(m_result);
					else
						ProfilerManager::GetInstance().WriteProfile(m_result);

					m_stopped = true;
				}
			};

		private:
			std::chrono::time_point<std::chrono::high_resolution_clock> m_start_point_time;
			ProfilerManager::ProfilingResult m_result;

			bool m_stopped = false;
		};

#define PROFILE_BEGIN_SESSION(session_name, output) ProfilerManager::GetInstance().BeginSession(session_name, output)
#define PROFILE_END_SESSION() ProfilerManager::GetInstance().EndSession()

#define PROFILE_SCOPE(timer_name) ProfilingTimer timer##__LINE__(timer_name)
#define PROFILE_FUNCTION() PROFILE_SCOPE(__FUNCSIG__)

	} // tools

} // hellengine