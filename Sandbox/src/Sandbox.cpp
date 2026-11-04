#include <plagc.h>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

class SandboxApp : public plagc::Application
{
public:

	SandboxApp()
	{
		int sucess = glfwInit();
		if (sucess)
		{
			glfwCreateWindow(1280, 720, "Window", nullptr, nullptr);
			spdlog::info("spdlog & Windows initialized successfully!");
		}
	}

private:

};


std::unique_ptr<plagc::Application> plagc::CreateApplication()
{
	return std::make_unique<SandboxApp>();
}