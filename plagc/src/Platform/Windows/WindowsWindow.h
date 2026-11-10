#pragma once
#include "Contracts/Window.h"

#include <string>

#include <GLFW/glfw3.h>

namespace plagc
{
	class WindowsWindow : public Window
	{
	public:

		~WindowsWindow() noexcept;

		virtual void OnUpdate() override;

		virtual const int GetWidth() const noexcept override;
		virtual const int GetHeight() const noexcept override;
		virtual const bool GetVsync() const noexcept override;

	private:

		void Initialize(const std::string& name);
		void Shutdown();

		WindowsWindow(uint16_t width, uint16_t height, const std::string& name, bool vsync,
			std::function<void(Owner<Event>)> fn);


		friend class ::plagc::WindowBuilder;


	private:
		struct WindowData
		{
			uint16_t Width;
			uint16_t Height;
			bool Vsync;
			std::function<void(Owner<Event>)> PublishEvent;
		};

		GLFWwindow* m_Window;
		Owner<WindowData> m_pimpl;
	};
}