#pragma once

#include "Application.h"
#include "log.h"

int main(int argc, char* agrv[])
{
	plagc::Log::Init();

	CORE_INFO("Initialized Application");

	auto app = plagc::CreateApplication();
	app->Run();
}