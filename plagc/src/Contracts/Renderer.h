#pragma once
namespace plagc
{

	class Renderer
	{
	public:

		Renderer() = default;

		virtual ~Renderer() = default;

		virtual void OnUpdate() = 0;

		static Owner<Renderer> Init();

	protected:



	private:

		static bool m_InitDone;

	};

}