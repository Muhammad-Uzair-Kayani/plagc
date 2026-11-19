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

		void InitWorkspace();

		void Shutdown();

		friend class ::plagc::RendererBuilder;

	private:

		struct OgreFatcory
		{
			Ogre::Window* m_RenderWindow = nullptr;
			Ogre::SceneManager* m_SceneManager = nullptr;
			Ogre::Camera* m_Camera = nullptr;

			Ogre::CompositorManager2* m_Compositor = nullptr;
			Ogre::CompositorWorkspace* m_Workspace = nullptr;
			Ogre::String m_WorkspaceName = "plagc.BasicWorkspace";
		};

		Owner<Ogre::Root> m_Root;

		Owner<OgreFatcory> m_OgreFactory;

	};

}