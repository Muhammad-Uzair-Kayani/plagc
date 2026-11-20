#include "pch.h"
#include "SceneManager.h"
#include "Core/Config.h"
#include "Concrete/RenderSystem/Ogre/SceneManager.h"

#include <stdexcept>

namespace plagc
{
    Owner<SceneManager> CreateSceneManager(const std::string& name, MaterialSystem& materials, std::size_t workerThreads)
    {
        switch (GET_RENDERER())
        {
        case RenderSystem::Ogre_Next:
            return Owner<SceneManager>(new OgreSceneManager(name, materials, workerThreads));
        case RenderSystem::plagc:
        default:
            throw std::runtime_error("Render system not supported yet");
        }
    }
}
