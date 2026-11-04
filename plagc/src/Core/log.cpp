#include "pch.h"
#include "log.h"
#include<spdlog/sinks/stdout_color_sinks.h>

namespace plagc
{

	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;

	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

	void Log::Init()
	{
		s_CoreLogger = spdlog::stdout_color_mt("plagc");
		s_ClientLogger = spdlog::stdout_color_mt("Client");

		s_CoreLogger->set_pattern("[%H:%M:%S.%e] [%n] [%^%l%$] [%s:%#] %v");
		s_ClientLogger->set_pattern("[%H:%M:%S.%e] [%n] [%^%l%$] %v");

		s_CoreLogger->set_level(spdlog::level::trace);
		s_ClientLogger->set_level(spdlog::level::info);
	}

	std::shared_ptr<spdlog::logger> Log::GetCoreLogger()
	{
		return s_CoreLogger;
	}

	std::shared_ptr<spdlog::logger> Log::GetClientLogger()
	{
		return s_ClientLogger;
	}




}