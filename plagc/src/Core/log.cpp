#include "pch.h"
#include "log.h"
#include<spdlog/sinks/stdout_color_sinks.h>

namespace plagc
{

	Log& Log::Get()
	{
		static Log logger;
		return logger;
	}

	Log::Log()
	{
		s_CoreLogger = SET_SINK("plagc");
		s_ClientLogger = SET_SINK("Client");

		s_CoreLogger->set_pattern("[%H:%M:%S.%e] [%n] [%^%l%$] [%s:%#] %v");
		s_ClientLogger->set_pattern("[%H:%M:%S.%e] [%n] [%^%l%$] %v");

		s_CoreLogger->set_level(spdlog::level::trace);
		s_ClientLogger->set_level(spdlog::level::trace);
	}

	Ref<spdlog::logger> Log::GetCoreLogger() noexcept
	{
		return s_CoreLogger;
	}

	Ref<spdlog::logger> Log::GetClientLogger() noexcept
	{
		return s_ClientLogger;
	}


}