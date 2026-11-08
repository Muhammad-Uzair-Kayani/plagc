#include <plagc.h>
#include <GLFW/glfw3.h>

class SandboxApp : public plagc::Application
{
public:

	SandboxApp() noexcept
	{
		CLIENT_INFO("Sandbox Initialized");
	}

private:

};


Owner<plagc::Application> plagc::CreateApplication()
{
	return MakeUnique<SandboxApp>();
}