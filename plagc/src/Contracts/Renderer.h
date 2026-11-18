#pragma once
#include "Contracts/Window.h"

namespace plagc
{

	class Renderer
	{
	public:

		virtual ~Renderer() = default;

		virtual void OnUpdate() = 0;

	protected:

		Renderer() = default;

	private:

	};


	class RendererBuilder
	{
	public:

		RendererBuilder& WindowHandle(Window* window) { r_Window = window; return *this; }

		Owner<Renderer> Build();

	private:

		Owner<Renderer> BuildRenderer();

	private:

		Window* r_Window;

	};

}