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

		size_t count = std::thread::hardware_concurrency();
		if (count == 0) count = 4;

		std::vector<std::thread> m_ThreadPool;

		m_ThreadPool.reserve(count);


		while (count-- > 0)
		{
			Owner<plagc::Event> e = MakeUnique<plagc::KeyPressedEvent>(1, count, 1);
			m_ThreadPool.emplace_back(&plagc::EventBus::PushEvent, m_EventBus.get(), std::move(e));
		}

		for (auto& thread : m_ThreadPool) if (thread.joinable()) thread.join();

	}

	void OnKeyPressed(plagc::Event& e)
	{
		if (e.GetType() == plagc::KeyPressedEvent::StaticType)
		{
			auto& keyEvent = static_cast<plagc::KeyPressedEvent&>(e);
			CLIENT_INFO("Key Board Button Pressed, Key Code: {}", keyEvent.ScanCode);
		}
	}

private:
	plagc::SubscriptionHandler<plagc::KeyPressedEvent> m_KeyEventHandle;
};


Owner<plagc::Application> plagc::CreateApplication()
{
	return MakeUnique<SandboxApp>();
}