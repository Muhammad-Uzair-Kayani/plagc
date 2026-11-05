#include "pch.h"
#include "log.h"
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
		EventBus::PushEvent(std::make_shared<WindowCloseEvent>());
	}


	bool Application::OnEvent(std::shared_ptr<Event> e)
	{
		if (e->GetType() == EventType::WindowClose) m_Running = false;
		return true;
	}



	void Application::Run()
	{
		CORE_DEBUG("RUNNING APPLICATION");
		while (m_Running)
		{
			EventBus::OnUpdate();
			OnUpdate();
		}
	}
}