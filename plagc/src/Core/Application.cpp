#include "pch.h"
#include "Application.h"

namespace plagc
{
	Application::Application()
	{

	}


	Application::~Application()
	{

	}

	void Application::OnUpdate()
	{
		
	}


	void Application::OnEvent()
	{

	}



	void Application::Run()
	{
		while (m_Running)
		{
			OnUpdate();
		}
	}
}