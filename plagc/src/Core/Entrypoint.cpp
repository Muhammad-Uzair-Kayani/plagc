#pragma once

#include "Application.h"
#include "log.h"

int main(int argc, char* agrv[])
{
	plagc::Log::Init();

	CORE_INFO("Initialized Application");

	std::unique_ptr<plagc::Application> app = plagc::CreateApplication();
	CORE_DEBUG("Starting Application");
	app->Run();
}