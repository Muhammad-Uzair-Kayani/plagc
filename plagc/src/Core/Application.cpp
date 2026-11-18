#include "pch.h"
#include "Application.h"
#include "Config.h"

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
		m_Window = windowBuilder.Width(1280).Height(720).Name("plagc").Vsync(false).FullScreen(false).CallBack(
			std::bind(&EventBus::PushEvent, m_EventBus.get(), std::placeholders::_1)
		).Build();
		

		RendererBuilder rendererBuilder;
		m_Renderer = rendererBuilder.WindowHandle(m_Window.get()).Build();

	}

	void Application::Shutdown()
	{
		CORE_TRACE("Shutting Down Application");
		m_Running = false;
		m_JobSystem.ShutDown(true);
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
		while (m_Running)
		{
			OnUpdate();
			m_Window->OnUpdate();
			m_EventBus->OnUpdate();
			m_Renderer->OnUpdate();
		}
	}
}