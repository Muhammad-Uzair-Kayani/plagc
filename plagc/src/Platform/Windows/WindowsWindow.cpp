#include "pch.h"
#include "WindowsWindow.h"
#include "Core/WindowEvent.h"
#include "Core/KeyEvent.h"
#include "Core/Core.h"

#include <exception>

namespace plagc
{

	WindowsWindow::WindowsWindow(uint16_t width, uint16_t height, const std::string& name, bool vsync,
		std::function<void(Owner<Event>)> fn)
	{
		m_pimpl = MakeUnique<WindowData>();
		m_pimpl->Width = width;
		m_pimpl->Height = height;
		m_pimpl->Vsync = vsync;
		m_pimpl->PublishEvent = fn;
		Initialize(name);
	}

	WindowsWindow::~WindowsWindow() noexcept
	{
		Shutdown();
	}

	void WindowsWindow::OnUpdate()
	{
		glfwPollEvents();
	}

	const int WindowsWindow::GetWidth() const noexcept
	{
		return m_pimpl->Width;
	}

	const int WindowsWindow::GetHeight() const noexcept
	{
		return m_pimpl->Height;
	}

	const bool WindowsWindow::GetVsync() const noexcept
	{
		return m_pimpl->Vsync;
	}

	void WindowsWindow::Initialize(const std::string& name)
	{

		if (!glfwInit())
		{
			CORE_CRITICAL("FAILED TO INITIALIZE GLFW");
			throw std::runtime_error("GLFW Initialization Failed");
		}

		m_Window = glfwCreateWindow(static_cast<int>(m_pimpl->Width),
			static_cast<int>(m_pimpl->Height), name.c_str(), nullptr, nullptr);

		if (!m_Window)
		{
			CORE_CRITICAL("FAILED TO CREATE GLFW WINDOW");
			throw std::runtime_error("GLFW Window Creation Failed");
		}

		CORE_DEBUG("WINDOW CREATED SUCESSFULLY");

		m_pimpl->Vsync ? glfwSwapInterval(1) : glfwSwapInterval(0);

		glfwSetWindowUserPointer(m_Window, m_pimpl.get());


		/* 1. Window Position */
		glfwSetWindowPosCallback(m_Window,
			[](GLFWwindow* window, int xpos, int ypos)
			{
				WindowData& data = *static_cast<WindowData*>(
					glfwGetWindowUserPointer(window));
			}
		);

		/* 2. Window Size */
		glfwSetWindowSizeCallback(m_Window,
			[](GLFWwindow* window, int width, int height)
			{
				WindowData& data = *static_cast<WindowData*>(
					glfwGetWindowUserPointer(window));
			}
		);

		/* 3. Window Close */
		glfwSetWindowCloseCallback(m_Window,
			[](GLFWwindow* window)
			{
				WindowData& data = *static_cast<WindowData*>(
					glfwGetWindowUserPointer(window));

				auto e = MakeUnique<WindowCloseRequestEvent>();
				data.PublishEvent(std::move(e));
			}
		);

		/* 4. Window Refresh */
		glfwSetWindowRefreshCallback(m_Window,
			[](GLFWwindow* window)
			{
				WindowData& data = *static_cast<WindowData*>(
					glfwGetWindowUserPointer(window));
			}
		);

		/* 5. Window Focus */
		glfwSetWindowFocusCallback(m_Window,
			[](GLFWwindow* window, int focused)
			{
				WindowData& data = *static_cast<WindowData*>(
					glfwGetWindowUserPointer(window));
			}
		);

		/* 6. Window Minimize / Restore */
		glfwSetWindowIconifyCallback(m_Window,
			[](GLFWwindow* window, int iconified)
			{
				WindowData& data = *static_cast<WindowData*>(
					glfwGetWindowUserPointer(window));
			}
		);

		/* 7. Window Maximize / Restore */
		glfwSetWindowMaximizeCallback(m_Window,
			[](GLFWwindow* window, int maximized)
			{
				WindowData& data = *static_cast<WindowData*>(
					glfwGetWindowUserPointer(window));
			}
		);

		/* 8. Framebuffer Resize */
		glfwSetFramebufferSizeCallback(m_Window,
			[](GLFWwindow* window, int width, int height)
			{
				WindowData& data = *static_cast<WindowData*>(
					glfwGetWindowUserPointer(window));
			}
		);

		/* 9. Window Content Scale (DPI) */
		glfwSetWindowContentScaleCallback(m_Window,
			[](GLFWwindow* window, float xscale, float yscale)
			{
				WindowData& data = *static_cast<WindowData*>(
					glfwGetWindowUserPointer(window));
			}
		);


		/* 10. Keyboard */
		glfwSetKeyCallback(m_Window,
			[](GLFWwindow* window, int key, int scancode,
				int action, int mods)
			{
				WindowData& data = *static_cast<WindowData*>(
					glfwGetWindowUserPointer(window));

				auto e = MakeUnique<KeyPressedEvent>(key, scancode, mods);
				data.PublishEvent(std::move(e));
				CORE_DEBUG("Key Callback");
			}
		);

		/* 11. Character / Unicode Input */
		glfwSetCharCallback(m_Window,
			[](GLFWwindow* window, unsigned int codepoint)
			{
				WindowData& data = *static_cast<WindowData*>(
					glfwGetWindowUserPointer(window));
			}
		);

		/* 12. Mouse Button */
		glfwSetMouseButtonCallback(m_Window,
			[](GLFWwindow* window, int button,
				int action, int mods)
			{
				WindowData& data = *static_cast<WindowData*>(
					glfwGetWindowUserPointer(window));
			}
		);

		/* 13. Cursor Position */
		glfwSetCursorPosCallback(m_Window,
			[](GLFWwindow* window, double xpos, double ypos)
			{
				WindowData& data = *static_cast<WindowData*>(
					glfwGetWindowUserPointer(window));
			}
		);

		/* 14. Cursor Enter / Leave */
		glfwSetCursorEnterCallback(m_Window,
			[](GLFWwindow* window, int entered)
			{
				WindowData& data = *static_cast<WindowData*>(
					glfwGetWindowUserPointer(window));
			}
		);

		/* 15. Mouse Scroll */
		glfwSetScrollCallback(m_Window,
			[](GLFWwindow* window, double xoffset, double yoffset)
			{
				WindowData& data = *static_cast<WindowData*>(
					glfwGetWindowUserPointer(window));
			}
		);

		/* 16. File Drop */
		glfwSetDropCallback(m_Window,
			[](GLFWwindow* window, int count, const char* paths[])
			{
				WindowData& data = *static_cast<WindowData*>(
					glfwGetWindowUserPointer(window));
			}
		);

	}


	void WindowsWindow::Shutdown()
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

}