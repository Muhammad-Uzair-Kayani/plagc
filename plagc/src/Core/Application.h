#pragma once
#include "Event.h"

namespace plagc
{
	class Application
	{

	public:

		Application();
		virtual ~Application();

		void Run();

		bool OnEvent(Ref<Event> e);

	private:

		virtual void OnUpdate();

		bool m_Running = true;

	};

	Owner<Application> CreateApplication();

}


