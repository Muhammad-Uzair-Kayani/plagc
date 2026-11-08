#include "pch.h"
#include "Application.h"
#include "Config.h"

namespace plagc
{

	Application::Application() noexcept
	{
		Initialize();
	}


	Application::~Application() noexcept
	{
		Shutdown();
	}

	void Application::Initialize()
	{
		try
		{
			m_EventBus = MakeUnique<EventBus>(std::bind(&Application::OnEvent, this, std::placeholders::_1));
			GET_PLATFROM() == Platform::Windows ? CORE_DEBUG("PLATFROM WINDOWS") : CORE_DEBUG("None");
			GET_RENDERING_API() == RenderingAPI::DirectX ? CORE_DEBUG("PLATFROM DirectX") : CORE_DEBUG("None");
			WindowBuilder windowBuilder;
			m_Window = windowBuilder.Width(1280).Height(720).Name("plagc").Vsync(true).CallBack(
				std::bind(&EventBus::PushEvent, m_EventBus.get(), std::placeholders::_1)
			).Build();
		}
		catch (std::exception e)
		{
			std::puts(e.what());
		}
		
	}

	void Application::Shutdown()
	{
		m_Running = false;
	}

	void Application::OnUpdate()
	{
		m_Window->OnUpdate();
	}


	bool Application::OnEvent(Ref<Event> e)
	{
		if (e->GetType() == EventType::WindowClose) Shutdown();
		return true;
	}



	void Application::Run()
	{
		CORE_DEBUG("RUNNING APPLICATION");
		while (m_Running)
		{
			OnUpdate();
			m_EventBus->OnUpdate();
		}
	}
}