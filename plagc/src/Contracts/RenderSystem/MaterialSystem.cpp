#include "pch.h"
#include "MaterialSystem.h"
#include "Core/Config.h"
#include "Concrete/RenderSystem/Ogre/MaterialSystem.h"

#include <stdexcept>

namespace plagc
{
    Owner<MaterialSystem> CreateMaterialSystem(const std::string& hlmsPath)
    {
        switch (GET_RENDERER())
        {
        case RenderSystem::Ogre_Next:
            return Owner<MaterialSystem>(new OgreMaterialSystem(hlmsPath));

        case RenderSystem::plagc:
        default:
            throw std::runtime_error("Platform Not Supported Yet");
        }
    }
}
