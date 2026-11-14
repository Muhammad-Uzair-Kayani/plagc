#pragma once
#include "Core/Core.h"
#include "Event/Event.h"
#include "Core/Config.h"

#include <functional>
#include <string>

namespace plagc
{
	
	class Window
	{
	public:

		
		virtual ~Window() noexcept = default;

		virtual void OnUpdate() = 0;

		virtual const int GetWidth() const noexcept = 0;
		virtual const int GetHeight() const noexcept = 0;
		virtual const bool GetVsync() const noexcept = 0;

	protected:

		Window() noexcept = default;

	};


	class WindowBuilder
	{
	public:

		WindowBuilder& Width(uint16_t width) { _width = MakeUnique<uint16_t>(width); return *this; }
		WindowBuilder& Height(uint16_t height) { _height = MakeUnique<uint16_t>(height); return *this; }
		WindowBuilder& Name(const std::string& name) { _name = MakeUnique<std::string>(name); return *this; }
		WindowBuilder& Vsync(bool vsync) { _vsync = MakeUnique<bool>(vsync); return *this; }
		WindowBuilder& CallBack(std::function<bool(Owner<Event>)> fn) { _fn = fn; return *this; }
		Owner<Window> Build();

	private:

		Owner<Window> BuildWindow();

	private:

		Owner<uint16_t> _width;
		Owner<uint16_t> _height;
		Owner<std::string> _name;
		Owner<bool> _vsync;
		std::function<bool(Owner<Event>)> _fn;

	};
}