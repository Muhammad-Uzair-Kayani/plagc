#pragma once

#include "Application.h"
#include "log.h"

#ifdef _WIN32
#ifdef DIST

int WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	try
	{
		CORE_DEBUG("TESING LOGGER");
		auto app = plagc::CreateApplication();
		app->Run();
	}
	catch (std::exception& e)
	{
		std::puts(e.what());
	}
}

#else

int main(int argc, char* agrv[])
{
	try
	{
		CORE_DEBUG("TESING LOGGER");
		auto app = plagc::CreateApplication();
		app->Run();
	}
	catch (std::exception& e)
	{
		std::puts(e.what());
	}

}

#endif //DIST


#endif // _WIN32
