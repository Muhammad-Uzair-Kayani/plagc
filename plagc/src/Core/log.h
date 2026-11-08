#pragma once
#include "Core.h"
#include<spdlog/logger.h>

namespace plagc
{
	class Log
	{
	public:

		static Log& Get();

		spdlog::logger& GetCoreLogger() noexcept;
		spdlog::logger& GetClientLogger() noexcept;

	private:

		Log();
		

	private:
		Ref<spdlog::logger> s_CoreLogger;
		Ref<spdlog::logger> s_ClientLogger;

	};

}

#include <spdlog/spdlog.h>

#ifdef DIST

#include <spdlog/sinks/basic_file_sink.h>

#define SET_SINK(name) \
        ::spdlog::basic_logger_mt(name, name ".log")

#else

#include <spdlog/sinks/stdout_color_sinks.h>

#define SET_SINK(name) \
        ::spdlog::stdout_color_mt(name)

#endif

// ==========================================
// CORE LOGGER — All levels
// ==========================================

#define CORE_TRACE(...) \
    SPDLOG_LOGGER_TRACE(&::plagc::Log::Get().GetCoreLogger(), __VA_ARGS__)

#define CORE_DEBUG(...) \
    SPDLOG_LOGGER_DEBUG(&::plagc::Log::Get().GetCoreLogger(), __VA_ARGS__)

#define CORE_INFO(...) \
    SPDLOG_LOGGER_INFO(&::plagc::Log::Get().GetCoreLogger(), __VA_ARGS__)

#define CORE_WARN(...) \
    SPDLOG_LOGGER_WARN(&::plagc::Log::Get().GetCoreLogger(), __VA_ARGS__)

#define CORE_ERROR(...) \
    SPDLOG_LOGGER_ERROR(&::plagc::Log::Get().GetCoreLogger(), __VA_ARGS__)

#define CORE_CRITICAL(...) \
    SPDLOG_LOGGER_CRITICAL(&::plagc::Log::Get().GetCoreLogger(), __VA_ARGS__)


// ==========================================
// CLIENT LOGGER — Info and above
// ==========================================

#define CLIENT_INFO(...) \
    SPDLOG_LOGGER_INFO(&::plagc::Log::Get().GetClientLogger(), __VA_ARGS__)

#define CLIENT_WARN(...) \
    SPDLOG_LOGGER_WARN(&::plagc::Log::Get().GetClientLogger(), __VA_ARGS__)

#define CLIENT_ERROR(...) \
    SPDLOG_LOGGER_ERROR(&::plagc::Log::Get().GetClientLogger(), __VA_ARGS__)

#define CLIENT_CRITICAL(...) \
    SPDLOG_LOGGER_CRITICAL(&::plagc::Log::Get().GetClientLogger(), __VA_ARGS__)