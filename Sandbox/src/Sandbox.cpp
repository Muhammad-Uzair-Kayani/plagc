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
		auto job = m_JobSystem.Parallel_For(0, 1000000, [](size_t i)
			{
				CLIENT_INFO("Executed Job A with counter: {0}", i);
			});
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