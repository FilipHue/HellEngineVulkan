#pragma once

// Internal
#include <hellengine/core/api.h>
#include <hellengine/core/defines.h>
#include <hellengine/core/memory.hpp>

// External
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace hellengine
{

	namespace core
	{

		constexpr const char* CORE_LOGGER_NAME = "HELLENGINE";
		constexpr const char* GRAPHICS_LOGGER_NAME = "GRAPHICS";
		constexpr const char* ECS_LOGGER_NAME = "ECS";
		constexpr const char* CLIENT_LOGGER_NAME = "CLIENT";

		class Logger final
		{
		public:
			Logger() = delete;

			HE_API static void Init();
			HE_API static void Shutdown();

			HE_API FORCE_INLINE static Shared<spdlog::logger>& GetCoreLogger() { return s_core_logger; }
			HE_API FORCE_INLINE static Shared<spdlog::logger>& GetGraphicsLogger() { return s_graphics_logger; }
			HE_API FORCE_INLINE static Shared<spdlog::logger>& GetEcsLogger() { return s_ecs_logger; }
			HE_API FORCE_INLINE static Shared<spdlog::logger>& GetClientLogger() { return s_client_logger; }

		private:
			HE_API static Shared<spdlog::logger> s_core_logger;
			HE_API static Shared<spdlog::logger> s_graphics_logger;
			HE_API static Shared<spdlog::logger> s_ecs_logger;
			HE_API static Shared<spdlog::logger> s_client_logger;
		};

#define HE_CORE_TRACE(...)			::hellengine::core::Logger::GetCoreLogger()->trace(__VA_ARGS__)
#define HE_CORE_INFO(...)			::hellengine::core::Logger::GetCoreLogger()->info(__VA_ARGS__)
#define HE_CORE_WARN(...)			::hellengine::core::Logger::GetCoreLogger()->warn(__VA_ARGS__)
#define HE_CORE_ERROR(...)			::hellengine::core::Logger::GetCoreLogger()->error(__VA_ARGS__)
#define HE_CORE_CRITICAL(...)		::hellengine::core::Logger::GetCoreLogger()->critical(__VA_ARGS__)

#define HE_GRAPHICS_TRACE(...)		::hellengine::core::Logger::GetGraphicsLogger()->trace(__VA_ARGS__)
#define HE_GRAPHICS_INFO(...)		::hellengine::core::Logger::GetGraphicsLogger()->info(__VA_ARGS__)
#define HE_GRAPHICS_WARN(...)		::hellengine::core::Logger::GetGraphicsLogger()->warn(__VA_ARGS__)
#define HE_GRAPHICS_ERROR(...)		::hellengine::core::Logger::GetGraphicsLogger()->error(__VA_ARGS__)
#define HE_GRAPHICS_CRITICAL(...)	::hellengine::core::Logger::GetGraphicsLogger()->critical(__VA_ARGS__)

#define HE_ECS_TRACE(...)			::hellengine::core::Logger::GetEcsLogger()->trace(__VA_ARGS__)
#define HE_ECS_INFO(...)			::hellengine::core::Logger::GetEcsLogger()->info(__VA_ARGS__)
#define HE_ECS_WARN(...)			::hellengine::core::Logger::GetEcsLogger()->warn(__VA_ARGS__)
#define HE_ECS_ERROR(...)			::hellengine::core::Logger::GetEcsLogger()->error(__VA_ARGS__)
#define HE_ECS_CRITICAL(...)		::hellengine::core::Logger::GetEcsLogger()->critical(__VA_ARGS__)

#define HE_CLIENT_TRACE(...)		::hellengine::core::Logger::GetClientLogger()->trace(__VA_ARGS__)
#define HE_CLIENT_INFO(...)			::hellengine::core::Logger::GetClientLogger()->info(__VA_ARGS__)
#define HE_CLIENT_WARN(...)			::hellengine::core::Logger::GetClientLogger()->warn(__VA_ARGS__)
#define HE_CLIENT_ERROR(...)		::hellengine::core::Logger::GetClientLogger()->error(__VA_ARGS__)
#define HE_CLIENT_CRITICAL(...)		::hellengine::core::Logger::GetClientLogger()->critical(__VA_ARGS__)

#if defined(HE_DEBUG)
#define HE_CORE_DEBUG(...)			::hellengine::core::Logger::GetCoreLogger()->debug(__VA_ARGS__)
#define HE_GRAPHICS_DEBUG(...)		::hellengine::core::Logger::GetGraphicsLogger()->debug(__VA_ARGS__)
#define HE_ECS_DEBUG(...)			::hellengine::core::Logger::GetEcsLogger()->debug(__VA_ARGS__)
#define HE_CLIENT_DEBUG(...)		::hellengine::core::Logger::GetClientLogger()->debug(__VA_ARGS__)
#else
#define HE_CORE_DEBUG(...)
#define HE_GRAPHICS_DEBUG(...)
#define HE_CLIENT_DEBUG(...)
#endif

	} // namespace core

} // namespace hellengine