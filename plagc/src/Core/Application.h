#pragma once
#include "Core.h"
#include "Event/Event.h"
#include "Event/EventBus.h"
#include "Contracts/Window.h"
#include "Event/WindowEvent.h"

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
		Owner<Window> m_Window;
		bool m_Running = true;

	};

	Owner<Application> CreateApplication();

}


