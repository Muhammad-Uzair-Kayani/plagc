#pragma once

extern "C++" std::unique_ptr<plagc::Application> plagc::CreateApplication();


int main(int argc, char* agrv[])
{
	auto app = plagc::CreateApplication();
	app->Run();
}