#pragma once

#include "Contracts/Renderer.h"

#include <OgreRoot.h>
#include <OgrePlugin.h>

namespace plagc
{

	class OgreRenderer : public Renderer
	{

	public:

		OgreRenderer();

		void OnUpdate() override;


	private:

		void Init();

		void Shutdown();

	private:

		Owner<Ogre::Plugin> m_Plugin;
		Owner<Ogre::Root> m_Root;

	};

}