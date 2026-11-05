#pragma once
#include<spdlog/logger.h>

namespace plagc
{
	class Log
	{
	public:

		static void Init();

		static std::shared_ptr<spdlog::logger>GetCoreLogger();
		static std::shared_ptr<spdlog::logger>GetClientLogger();

	private:

		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;

	};

}

#pragma once

#include <spdlog/spdlog.h>

// ==========================================
// CORE LOGGER — All levels
// ==========================================

#define CORE_TRACE(...) \
    SPDLOG_LOGGER_TRACE(::plagc::Log::GetCoreLogger().get(), __VA_ARGS__)

#define CORE_DEBUG(...) \
    SPDLOG_LOGGER_DEBUG(::plagc::Log::GetCoreLogger().get(), __VA_ARGS__)

#define CORE_INFO(...) \
    SPDLOG_LOGGER_INFO(::plagc::Log::GetCoreLogger().get(), __VA_ARGS__)

#define CORE_WARN(...) \
    SPDLOG_LOGGER_WARN(::plagc::Log::GetCoreLogger().get(), __VA_ARGS__)

#define CORE_ERROR(...) \
    SPDLOG_LOGGER_ERROR(::plagc::Log::GetCoreLogger().get(), __VA_ARGS__)

#define CORE_CRITICAL(...) \
    SPDLOG_LOGGER_CRITICAL(::plagc::Log::GetCoreLogger().get(), __VA_ARGS__)


// ==========================================
// CLIENT LOGGER — Info and above
// ==========================================

#define CLIENT_INFO(...) \
    SPDLOG_LOGGER_INFO(::plagc::Log::GetClientLogger().get(), __VA_ARGS__)

#define CLIENT_WARN(...) \
    SPDLOG_LOGGER_WARN(::plagc::Log::GetClientLogger().get(), __VA_ARGS__)

#define CLIENT_ERROR(...) \
    SPDLOG_LOGGER_ERROR(::plagc::Log::GetClientLogger().get(), __VA_ARGS__)

#define CLIENT_CRITICAL(...) \
    SPDLOG_LOGGER_CRITICAL(::plagc::Log::GetClientLogger().get(), __VA_ARGS__)