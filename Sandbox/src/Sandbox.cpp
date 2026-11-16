#include <plagc.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

class SandboxApp : public plagc::Application
{
public:

	SandboxApp()
	{
		CLIENT_INFO("Sandbox Initialized");
		m_KeyEventHandle = m_EventBus->Subscribe<plagc::KeyPressedEvent>(std::bind(&SandboxApp::OnKeyPressed, this, std::placeholders::_1));

	}

	~SandboxApp()
	{
		
	}

	void OnKeyPressed(plagc::Event& e)
	{
		if (e.GetType() == plagc::KeyPressedEvent::StaticType)
		{
			auto& keyEvent = static_cast<plagc::KeyPressedEvent&>(e);
			CLIENT_INFO("Key Board Button Pressed, Key Code: {}", keyEvent.ScanCode);
		}
	}

	void OnUpdate() override
	{
		StressTest();
	}

	void StressTest()
	{
		auto fn = [] { CLIENT_INFO("Job Executed;"); };

		auto job = m_Allocator.Allocate(std::move(fn));

		m_JobSystem.Submit(job);
	}

private:


private:

	plagc::JobAllocator m_Allocator;

	plagc::SubscriptionHandler<plagc::KeyPressedEvent> m_KeyEventHandle;

};


Owner<plagc::Application> plagc::CreateApplication()
{
	return MakeUnique<SandboxApp>();
}