#pragma once

#include "Contracts/Renderer.h"

#include <OgreRoot.h>
#include <OgrePlugin.h>
#include <OgreWindow.h>

namespace plagc
{

	class OgreRenderer : public Renderer
	{

	public:

		~OgreRenderer();

		void OnUpdate() override;


	private:

		OgreRenderer(Window* window);

		void Init(Window* window);

		void Shutdown();

		friend class ::plagc::RendererBuilder;

	private:

		Owner<Ogre::Plugin> m_Plugin;
		Owner<Ogre::Root> m_Root;
		Ogre::Window* m_Window;

	};

}