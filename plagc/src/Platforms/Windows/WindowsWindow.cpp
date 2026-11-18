#include "pch.h"
#include "WindowsWindow.h"
#include "EventSystem/WindowEvent.h"
#include "EventSystem/KeyEvent.h"
#include "Core/Core.h"

#include <exception>

namespace plagc
{

	WindowsWindow::WindowsWindow(uint16_t width, uint16_t height, const std::string& name, bool vsync, bool fullscreen,
		std::function<bool(Owner<Event>)> fn)
	{
		m_pimpl = MakeUnique<WindowData>();
		m_pimpl->Name = name;
		m_pimpl->Width = width;
		m_pimpl->Height = height;
		m_pimpl->Vsync = vsync;
		m_pimpl->Fullscreen = fullscreen;
		m_pimpl->PublishEvent = fn;
		Initialize();
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

	void WindowsWindow::Initialize()
	{

		if (!glfwInit())
		{
			CORE_CRITICAL("FAILED TO INITIALIZE GLFW");
			throw std::runtime_error("GLFW Initialization Failed");
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		if (!m_pimpl->Fullscreen)
			m_Window = glfwCreateWindow(static_cast<int>(m_pimpl->Width),
				static_cast<int>(m_pimpl->Height), m_pimpl->Name.c_str(), nullptr, nullptr);
		
		else
		{
			GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

			if (!mode)
				throw std::runtime_error("GLFW Failed to Get monitor Mode");

			m_Window = glfwCreateWindow(
				mode->width,
				mode->height,
				m_pimpl->Name.c_str(),
				primaryMonitor, // Pass monitor pointer for fullscreen (nullptr = windowed)
				nullptr
			);

			m_pimpl->Width = mode->width;
			m_pimpl->Height = mode->height;
		}

		if (!m_Window)
		{
			CORE_CRITICAL("FAILED TO CREATE GLFW WINDOW");
			throw std::runtime_error("GLFW Window Creation Failed");
		}

		CORE_DEBUG("WINDOW CREATED SUCESSFULLY");

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
				
				if (data.PublishEvent(std::move(e)))
				{
					CORE_TRACE("WindowClose Request Pushed");
				}
				else
				{
					CORE_TRACE("Failed To Push Close Req");
				}

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