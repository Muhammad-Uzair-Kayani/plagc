#include "pch.h"
#include "Renderer.h"
#include "Core/Core.h"

#include "RenderSystem/Ogre/OgreRenderer.h"

#include<exception>


enum class RenderSystem
{
	plagc,
	Ogre_Next
};

#ifdef CUSTOM_API
#define GET_RENDERER() RenderSystem::plagc
#elif OGRE_NEXT
#define GET_RENDERER() RenderSystem::Ogre_Next
#endif

namespace plagc
{

	bool Renderer::m_InitDone = false;

	Owner<Renderer> Renderer::Init()
	{
		if(m_InitDone) throw std::runtime_error("Renderer Cannot Re initialized");

		switch (GET_RENDERER())
		{
		case RenderSystem::Ogre_Next:
			return Owner<Renderer>(new OgreRenderer());
			m_InitDone = true;
			break;

		case RenderSystem::plagc:
			throw std::runtime_error("Platform Not Supported Yet");
			break;

		default:
			throw std::runtime_error("Platform Not Supported Yet");
			break;
		}
	}

}