#include "pch.h"
#include "Application.h"
#include "Config.h"
#include "WindowEvent.h"

namespace plagc
{

	Application::Application()
	{
		Initialize();
	}


	Application::~Application() noexcept
	{
		Shutdown();
	}

	void Application::Initialize()
	{
		m_EventBus = MakeUnique<EventBus>();

		m_EventBus->Subscribe<WindowCloseRequestEvent>(std::bind(&Application::OnWindowShouldClose, this, std::placeholders::_1));

		GET_PLATFROM() == Platform::Windows ? CORE_DEBUG("PLATFROM WINDOWS") : CORE_DEBUG("None");
		GET_RENDERING_API() == RenderingAPI::DirectX ? CORE_DEBUG("PLATFROM DirectX") : CORE_DEBUG("None");
		
		WindowBuilder windowBuilder;
		m_Window = windowBuilder.Width(1280).Height(720).Name("plagc").Vsync(true).CallBack(
			std::bind(&EventBus::PushEvent, m_EventBus.get(), std::placeholders::_1)
		).Build();
		
	}

	void Application::Shutdown()
	{
		m_Running = false;
	}

	void Application::OnUpdate()
	{
		
	}


	bool Application::OnWindowShouldClose(Event& e)
	{
		if (e.GetType() == EventType::WindowCloseRequest) Shutdown();
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
		}
	}
}