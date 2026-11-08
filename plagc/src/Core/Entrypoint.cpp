
#include "Application.h"
#include "log.h"

#include <exception>
#include <stdexcept>

#if defined(_WIN32) && defined(DIST)
#include <Windows.h>

#define LOG_CRITICAL(message) \
        MessageBoxA(nullptr, message, "plagc - Fatal Error", \
                    MB_OK | MB_ICONERROR)
#else
#define LOG_CRITICAL(message) std::puts(message)
#endif


int StartPlagc()
{
	try
	{
		CORE_DEBUG("Starting plagc");

		auto app = plagc::CreateApplication();

		if (!app)
			throw std::runtime_error("Failed to initialize Application");

		app->Run();
		return 0;
	}
	catch (const std::exception& e)
	{
		LOG_CRITICAL(e.what());
		return 1;
	}
	catch (...)
	{
		LOG_CRITICAL("Unknown fatal exception");
		return 1;
	}
}


// Windows Dist
#if defined(_WIN32) && defined(DIST)

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	return StartPlagc();
}

// Windows Debug/Release and Linux
#else

int main()
{
	return StartPlagc();
}

#endif
