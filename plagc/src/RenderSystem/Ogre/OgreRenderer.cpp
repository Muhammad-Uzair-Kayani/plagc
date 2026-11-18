#include "pch.h"
#include "OgreRenderer.h"

#include "Core/Config.h"
#include "Core/Core.h"

#include <OgreRenderSystem.h>
#include <OgreAbiUtils.h>
#include <OgreVulkanPlugin.h>

namespace plagc
{

	OgreRenderer::OgreRenderer()
	{

		const Ogre::AbiCookie cookie = Ogre::generateAbiCookie();

		m_Root = MakeUnique<Ogre::Root>(&cookie, "", "", "ogre.log", "plagc");

		m_Plugin = MakeUnique<Ogre::VulkanPlugin>();

		m_Root->installPlugin(m_Plugin.get(), nullptr);

		CORE_DEBUG("Avaliable Rendering Systems");
		for (auto renderSystem : m_Root->getAvailableRenderers())
			CORE_TRACE(renderSystem->getName());

	}

	void OgreRenderer::OnUpdate()
	{

	}

	void OgreRenderer::Init()
	{

	}

	void OgreRenderer::Shutdown()
	{
		m_Root->uninstallPlugin(m_Plugin.get());
	}

}