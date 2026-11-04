#pragma once

#include "Application.h"

int main(int argc, char* agrv[])
{
	auto app = plagc::CreateApplication();
	app->Run();
}