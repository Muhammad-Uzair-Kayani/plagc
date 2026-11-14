#pragma once
#include "Core.h"
#include "Event/Event.h"
#include "Event/EventBus.h"
#include "Contracts/Window.h"
#include "Event/WindowEvent.h"

#include <atomic>

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

		alignas(std::hardware_destructive_interference_size)std::atomic<bool> m_Running{ true };

	};

	Owner<Application> CreateApplication();

}


