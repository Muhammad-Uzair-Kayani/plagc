#include <plagc.h>
#include <GLFW/glfw3.h>

class SandboxApp : public plagc::Application
{
public:

	SandboxApp()
	{
		CLIENT_INFO("Sandbox Initialized");
	}

private:

};


std::unique_ptr<plagc::Application> plagc::CreateApplication()
{
	return std::make_unique<SandboxApp>();
}