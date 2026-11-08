#include "pch.h"
#include "WindowsWindow.h"


namespace plagc
{

	WindowsWindow::WindowsWindow(uint16_t width, uint16_t height, const std::string& name, bool vsync,
		std::function<void(Ref<Event>)> fn) noexcept
	{
		m_pimpl = MakeUnique<WindowData>();
		m_pimpl->Width = width;
		m_pimpl->Height = height;
		m_pimpl->Vsync = vsync;
		m_pimpl->CallBack = fn;
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
			return;
		}

		m_Window = glfwCreateWindow(static_cast<int>(m_pimpl->Width),
			static_cast<int>(m_pimpl->Height), name.c_str(), nullptr, nullptr);

		if (!m_Window)
		{
			CORE_CRITICAL("FAILED TO CREATE GLFW WINDOW");
			return;
		}

		CORE_DEBUG("WINDOW CREATED SUCESSFULLY");

		glfwSetWindowUserPointer(m_Window, m_pimpl.get());

		glfwSetWindowCloseCallback(m_Window, 
			[](GLFWwindow* window)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				data.CallBack(MakeRef<WindowCloseEvent>());
			}
			);
	}

	void WindowsWindow::Shutdown()
	{
		glfwDestroyWindow(m_Window);
	}

}