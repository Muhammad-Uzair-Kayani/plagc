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

	Owner<Renderer> RendererBuilder::BuildRenderer()
	{
		switch (GET_RENDERER())
		{
		case RenderSystem::Ogre_Next:
			return Owner<Renderer>(new OgreRenderer(r_Window));
			break;

		case RenderSystem::plagc:
			throw std::runtime_error("Platform Not Supported Yet");
			break;

		default:
			throw std::runtime_error("Platform Not Supported Yet");
			break;
		}
	}

	Owner<Renderer> RendererBuilder::Build()
	{
		if (!r_Window) throw std::exception("User Window Pointer is required to Create Renderer");
		return BuildRenderer();
	}

}