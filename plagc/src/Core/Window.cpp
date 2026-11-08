#include "pch.h"
#include "Window.h"
#include "Platform/Windows/WindowsWindow.h"

#include <exception>

namespace plagc
{

	Owner<plagc::Window> WindowBuilder::BuildWindow()
	{
		switch (GET_PLATFROM())
		{
		case Platform::Windows:
			return Owner<Window>(new WindowsWindow(*_width.get(), *_height.get(), *_name.get(), *_vsync.get(), _fn));
			break;

		default:
			return nullptr;
			break;
		}
	}

	Owner<Window> WindowBuilder::Build()
	{
		if (!_height) throw std::exception("Height is required to Build Window");

		if (!_width) throw std::exception("Width is required to Build Window");

		if (!_name) throw std::exception("Name is required to Build Window");

		if (!_vsync) throw std::exception("Vsync is required to Build Window");

		if (!_fn) throw std::exception("Callback is required to Build Window");

		return BuildWindow();
	}

}