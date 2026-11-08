#pragma once
#include "Core.h"
#include "Event.h"
#include "EventBus.h"
#include "Window.h"

namespace plagc
{
	class Application
	{

	public:

		Application() noexcept;
		virtual ~Application() noexcept;

		void Run();

		bool OnEvent(Ref<Event> e);

	private:

		void Initialize();
		void Shutdown();

		void OnUpdate();

	private:

		Owner<EventBus> m_EventBus;
		Owner<Window> m_Window;
		bool m_Running = true;

	};

	Owner<Application> CreateApplication();

}


