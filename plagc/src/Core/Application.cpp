#include "pch.h"
#include "Application.h"
#include "Config.h"

#include <chrono>

namespace plagc
{

	Application::Application()
	{
		Initialize();
	}


	Application::~Application() noexcept
	{

	}

	void Application::Initialize()
	{
		m_EventBus = MakeUnique<EventBus>(2048);

		m_WindowCloseReqHandle = m_EventBus->Subscribe<WindowCloseRequestEvent>(std::bind(&Application::OnWindowShouldClose, this, std::placeholders::_1));
		m_WindowClosedHandle = m_EventBus->Subscribe<WindowClosedEvent>(std::bind(&Application::OnWindowShouldClose, this, std::placeholders::_1));

		GET_PLATFROM() == Platform::Windows ? CORE_DEBUG("PLATFROM WINDOWS") : CORE_DEBUG("None");
		GET_RENDERING_API() == RenderingAPI::DirectX ? CORE_DEBUG("PLATFROM DirectX") : CORE_DEBUG("None");
		
		WindowBuilder windowBuilder;
		m_Window = windowBuilder.Width(1280).Height(720).Name("plagc").Vsync(true).CallBack(
			std::bind(&EventBus::PushEvent, m_EventBus.get(), std::placeholders::_1)
		).Build();
		
	}

	void Application::Shutdown()
	{
		CORE_TRACE("Shutting Down Application");
		m_Running.store(false, std::memory_order_release);
	}

	void Application::OnUpdate()
	{
		
	}


	bool Application::OnWindowShouldClose(Event& e)
	{

		if (e.GetType() == EventType::WindowCloseRequest)
		{
			
			Owner<Event> _e = MakeUnique<WindowClosedEvent>();
			m_EventBus->PushEvent(std::move(_e));

		}
		else if (e.GetType() == EventType::WindowClosed)
		{
			Shutdown();
		}

		return true;
	}



	void Application::Run()
	{
		CORE_DEBUG("RUNNING APPLICATION");
		while (m_Running.load(std::memory_order_relaxed))
		{
			OnUpdate();
			m_Window->OnUpdate();
			m_EventBus->OnUpdate();
		}
	}
}