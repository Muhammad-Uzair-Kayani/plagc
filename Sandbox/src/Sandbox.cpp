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
		constexpr size_t count = 11000;

		auto results =
			std::make_shared<std::vector<double>>(count);

		auto start = std::chrono::steady_clock::now();

		plagc::JobGraph graph(m_JobSystem);

		auto job = graph.ParallelFor(0, count, [results](size_t i)
			{
				double value = static_cast<double>(i) + 1.0;

				for (int j = 0; j < 5000; ++j)
				{
					value = std::sqrt(
						value * 1.000001 + (j % 7) + 1.0
					);
				}

				(*results)[i] = value;
			});

		auto finished = graph.AddJob([results, start]()
			{
				double checksum = 0.0;

				for (double value : *results)
					checksum += value;

				auto end = std::chrono::steady_clock::now();

				auto elapsed =
					std::chrono::duration<double, std::milli>(
						end - start
					).count();

				CLIENT_INFO("Execution Time: {} ms", elapsed);
				CLIENT_INFO("Checksum: {}", checksum);
			});

		graph.AddDependency(finished, job);
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