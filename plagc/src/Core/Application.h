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

		Owner<Window> m_Window;
		bool m_Running = true;

	};

	Owner<Application> CreateApplication();

}


