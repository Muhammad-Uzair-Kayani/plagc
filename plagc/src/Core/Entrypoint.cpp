#pragma once

#include "Application.h"
#include "log.h"

int main(int argc, char* agrv[])
{
	plagc::Log::Init();

	auto app = plagc::CreateApplication();
	app->Run();

}