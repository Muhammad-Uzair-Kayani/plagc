#pragma once
#include "Core.h"
#include<spdlog/logger.h>

namespace plagc
{
	class Log
	{
	public:

		static Log& Get();

		Ref<spdlog::logger> GetCoreLogger() noexcept;
		Ref<spdlog::logger> GetClientLogger() noexcept;

	private:

		Log();
		
		void Init();

	private:
		Ref<spdlog::logger> s_CoreLogger;
		Ref<spdlog::logger> s_ClientLogger;

	};

}

#pragma 
#include <spdlog/spdlog.h>

#ifdef DIST

#define SET_SINK(...) ::spdlog::(__VA_ARGS__);

#else

#define SET_SINK(...) ::spdlog::stdout_color_mt(__VA_ARGS__);

#endif

// ==========================================
// CORE LOGGER — All levels
// ==========================================

#define CORE_TRACE(...) \
    SPDLOG_LOGGER_TRACE(::plagc::Log::Get().GetCoreLogger().get(), __VA_ARGS__)

#define CORE_DEBUG(...) \
    SPDLOG_LOGGER_DEBUG(::plagc::Log::Get().GetCoreLogger().get(), __VA_ARGS__)

#define CORE_INFO(...) \
    SPDLOG_LOGGER_INFO(::plagc::Log::Get().GetCoreLogger().get(), __VA_ARGS__)

#define CORE_WARN(...) \
    SPDLOG_LOGGER_WARN(::plagc::Log::Get().GetCoreLogger().get(), __VA_ARGS__)

#define CORE_ERROR(...) \
    SPDLOG_LOGGER_ERROR(::plagc::Log::Get().GetCoreLogger().get(), __VA_ARGS__)

#define CORE_CRITICAL(...) \
    SPDLOG_LOGGER_CRITICAL(::plagc::Log::Get().GetCoreLogger().get(), __VA_ARGS__)


// ==========================================
// CLIENT LOGGER — Info and above
// ==========================================

#define CLIENT_INFO(...) \
    SPDLOG_LOGGER_INFO(::plagc::Log::Get().GetClientLogger().get(), __VA_ARGS__)

#define CLIENT_WARN(...) \
    SPDLOG_LOGGER_WARN(::plagc::Log::Get().GetClientLogger().get(), __VA_ARGS__)

#define CLIENT_ERROR(...) \
    SPDLOG_LOGGER_ERROR(::plagc::Log::Get().GetClientLogger().get(), __VA_ARGS__)

#define CLIENT_CRITICAL(...) \
    SPDLOG_LOGGER_CRITICAL(::plagc::Log::Get().GetClientLogger().get(), __VA_ARGS__)