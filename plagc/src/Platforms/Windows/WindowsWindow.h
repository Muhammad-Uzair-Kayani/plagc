#pragma once
#include "Contracts/Window.h"

#include <string>

#include <GLFW/glfw3.h>

namespace plagc
{
	struct WindowData
	{
		std::string Name;
		uint16_t Width;
		uint16_t Height;
		std::function<bool(Owner<Event>)> PublishEvent;
		bool Vsync;
		bool Fullscreen;
	};


	class WindowsWindow : public Window
	{
	public:

		~WindowsWindow() noexcept;

		virtual void OnUpdate() override;

		GLFWwindow* GetNativeWindow() const noexcept { return m_Window; }

		WindowData* GetWindowsWindowData() const noexcept { return m_pimpl.get(); }

		const int GetWidth() const noexcept override;
		const int GetHeight() const noexcept override;
		const bool GetVsync() const noexcept override;

	private:

		void Initialize();
		void Shutdown();

		WindowsWindow(uint16_t width, uint16_t height, const std::string& name, bool vsync, bool fullscreen,
			std::function<bool(Owner<Event>)> fn);


		friend class ::plagc::WindowBuilder;


	private:

		GLFWwindow* m_Window;
		Owner<WindowData> m_pimpl;
	};
}