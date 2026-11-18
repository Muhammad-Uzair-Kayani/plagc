#pragma once

#include "Contracts/Renderer.h"

#include <OgreRoot.h>
#include <OgrePlugin.h>
#include <OgreWindow.h>
#include <OgreSceneManager.h>
#include <OgreCamera.h>
#include <Compositor/OgreCompositorManager2.h>

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
		Ogre::SceneManager* m_SceneManager;
		Ogre::Camera* m_Camera;
		Ogre::CompositorManager2* m_CompositorManager2;

	};

}