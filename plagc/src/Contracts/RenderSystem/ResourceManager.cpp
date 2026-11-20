#include "pch.h"
#include "ResourceManager.h"
#include "Core/Config.h"
#include "Concrete/RenderSystem/Ogre/ResourceManager.h"

#include <stdexcept>

namespace plagc
{
    namespace
    {
        Owner<ResourceManager>& ResourceInstance()
        {
            static Owner<ResourceManager> instance;
            return instance;
        }
    }

    ResourceManager& ResourceManager::Get()
    {
        Owner<ResourceManager>& instance = ResourceInstance();
        if (!instance)
        {
            switch (GET_RENDERER())
            {
            case RenderSystem::Ogre_Next:
                instance = Owner<ResourceManager>(new OgreResourceManager());
                break;
            case RenderSystem::plagc:
            default:
                throw std::runtime_error("Render system not supported yet");
            }
        }
        return *instance;
    }

    void ResourceManager::Shutdown()
    {
        ResourceInstance().reset();
    }
}
