#include "pch.h"
#include "Application.h"
#include "EventBus.h"

namespace plagc
{
	Application::Application()
	{
		EventBus::Init(std::bind(&Application::OnEvent, this, std::placeholders::_1));
	}


	Application::~Application()
	{

	}

	void Application::OnUpdate()
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		EventBus::PushEvent(std::make_shared<Event>(EventType::WindowClose));
	}


	bool Application::OnEvent(std::shared_ptr<Event> e)
	{
		if (e->GetType() == EventType::WindowClose) m_Running = false;
	}



	void Application::Run()
	{
		while (m_Running)
		{
			OnUpdate();
		}
	}
}