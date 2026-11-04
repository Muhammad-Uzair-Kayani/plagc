#pragma once

namespace plagc
{
	class Application
	{

	public:

		Application();
		virtual ~Application();

		void Run();

	private:

		virtual void OnUpdate();
		virtual void OnEvent();

		bool m_Running = true;

	};

	std::unique_ptr<Application> CreateApplication();

}


