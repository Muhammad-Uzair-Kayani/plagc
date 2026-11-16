#pragma once
#include "Core.h"
#include "EventSystem/Event.h"
#include "EventSystem/EventBus.h"
#include "Contracts/Window.h"
#include "EventSystem/WindowEvent.h"
#include "JobSystem/JobSystem.h"

namespace plagc
{
	class Application
	{

	public:

		Application();
		virtual ~Application() noexcept;

		void Run();

		bool OnWindowShouldClose(Event& e);

	private:

		void Initialize();
		void Shutdown();

		virtual void OnUpdate();


	protected:

		Owner<EventBus> m_EventBus;

	private:

		SubscriptionHandler<WindowCloseRequestEvent> m_WindowCloseReqHandle;
		SubscriptionHandler<WindowClosedEvent> m_WindowClosedHandle;
		Owner<Window> m_Window;
		
	protected:

		JobSystem m_JobSystem;


	private:

		bool m_Running = true;

	};

	Owner<Application> CreateApplication();

}


