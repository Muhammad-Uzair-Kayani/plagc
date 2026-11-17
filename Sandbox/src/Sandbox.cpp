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
		StressTest();
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
		
	}

	void StressTest()
	{
		auto graph = plagc::JobGraph(m_JobSystem);

		auto job = graph.ParallelFor(0, 11000, [](size_t i)
			{
				std::this_thread::sleep_for(std::chrono::microseconds(1));
			});

		auto job2 = graph.AddJob([] {CLIENT_INFO("Job Finished"); });
		
		graph.AddDependency(job2, job);

		graph.Submit();

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