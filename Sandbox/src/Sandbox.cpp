#include <plagc.h>

class SandboxApp : public plagc::Application
{
public:

	SandboxApp()
	{
	}

private:

};


std::unique_ptr<plagc::Application> plagc::CreateApplication()
{
	return std::make_unique<SandboxApp>();
}