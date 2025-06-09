#include "hepch.h"
#include "logger.h"

namespace hellengine
{

	namespace core
	{

		Shared<spdlog::logger> Logger::s_core_logger;
		Shared<spdlog::logger> Logger::s_graphics_logger;
		Shared<spdlog::logger> Logger::s_ecs_logger;
		Shared<spdlog::logger> Logger::s_client_logger;

		void Logger::Init()
		{
			std::vector<spdlog::sink_ptr> logSinks;
			logSinks.emplace_back(MakeShared<spdlog::sinks::stdout_color_sink_mt>());

			logSinks[0]->set_pattern("%^[%T] %n: %v%$");

			s_core_logger = MakeShared<spdlog::logger>(CORE_LOGGER_NAME, begin(logSinks), end(logSinks));
			spdlog::register_logger(s_core_logger);

			s_core_logger->set_level(spdlog::level::trace);
			s_core_logger->flush_on(spdlog::level::trace);

			s_graphics_logger = MakeShared<spdlog::logger>(GRAPHICS_LOGGER_NAME, begin(logSinks), end(logSinks));
			spdlog::register_logger(s_graphics_logger);

			s_graphics_logger->set_level(spdlog::level::trace);
			s_graphics_logger->flush_on(spdlog::level::trace);

			s_ecs_logger = MakeShared<spdlog::logger>(ECS_LOGGER_NAME, begin(logSinks), end(logSinks));
			spdlog::register_logger(s_ecs_logger);

			s_ecs_logger->set_level(spdlog::level::trace);
			s_ecs_logger->flush_on(spdlog::level::trace);

			s_client_logger = MakeShared<spdlog::logger>(CLIENT_LOGGER_NAME, begin(logSinks), end(logSinks));
			spdlog::register_logger(s_client_logger);

			s_client_logger->set_level(spdlog::level::trace);
			s_client_logger->flush_on(spdlog::level::trace);
		}

		void Logger::Shutdown()
		{
			spdlog::shutdown();
		}

	} // namespace core

} // namespace hellengine