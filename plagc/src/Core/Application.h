#pragma once
#include <memory>
#include "Event.h"

namespace plagc
{
	class Application
	{

	public:

		Application();
		virtual ~Application();

		void Run();

		bool OnEvent(std::shared_ptr<Event> e);

	private:

		virtual void OnUpdate();

		bool m_Running = true;

	};

	std::unique_ptr<Application> CreateApplication();

}


