#include "pch.h"
#include "Renderer.h"
#include "Core/Config.h"
#include "Concrete/RenderSystem/Ogre/OgreRenderer.h"

#include <stdexcept>

namespace plagc
{
    Owner<Renderer> RendererBuilder::BuildRenderer()
    {
        switch (GET_RENDERER())
        {
        case RenderSystem::Ogre_Next:
            return Owner<Renderer>(new OgreRenderer(r_Window));
        case RenderSystem::plagc:
        default:
            throw std::runtime_error("Rendering backend not supported yet");
        }
    }

    Owner<Renderer> RendererBuilder::Build()
    {
        if (!r_Window) throw std::invalid_argument("Window pointer is required to create Renderer");
        return BuildRenderer();
    }
}
