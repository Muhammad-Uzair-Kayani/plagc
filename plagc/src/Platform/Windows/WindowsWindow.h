#pragma once
#include "Core/Window.h"

#include <string>

#include <GLFW/glfw3.h>

namespace plagc
{
	class WindowsWindow : public Window
	{
	public:

		WindowsWindow(uint16_t width, uint16_t height, const std::string& name, bool vsync,
			std::function<void(Ref<Event>)> fn) noexcept;
		~WindowsWindow() noexcept;

		virtual void OnUpdate() override;

		virtual const int GetWidth() const noexcept override;
		virtual const int GetHeight() const noexcept override;
		virtual const bool GetVsync() const noexcept override;

	private:

		void Initialize(const std::string& name);
		void Shutdown();



	private:
		struct WindowData
		{
			uint16_t Width;
			uint16_t Height;
			bool Vsync;
			std::function<void(Ref<Event>)> CallBack;
		};

		GLFWwindow* m_Window;
		Owner<WindowData> m_pimpl;
	};
}