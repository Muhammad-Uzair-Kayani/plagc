#pragma once
#include "Core/Core.h"
#include "Contracts/RenderSystem/Camera.h"
#include "Contracts/RenderSystem/ResourceManager.h"
#include "Contracts/RenderSystem/MaterialSystem.h"

#include <cstddef>
#include <cstdint>
#include <string>

namespace plagc
{
    using SceneNodeHandle = std::uint64_t;
    using SceneItemHandle = std::uint64_t;
    constexpr SceneNodeHandle InvalidSceneNode = 0;
    constexpr SceneItemHandle InvalidSceneItem = 0;

    struct SceneVector3
    {
        float X = 0.0f;
        float Y = 0.0f;
        float Z = 0.0f;
    };

    class SceneManager
    {
    public:
        virtual ~SceneManager() = default;

        virtual Camera* CreateCamera(const std::string& name) = 0;
        virtual Camera* GetCamera(const std::string& name) const = 0;
        virtual void DestroyCamera(const std::string& name) = 0;

        virtual SceneNodeHandle CreateNode(const std::string& name, SceneNodeHandle parent = InvalidSceneNode) = 0;
        virtual SceneNodeHandle GetNode(const std::string& name) const = 0;
        virtual void DestroyNode(SceneNodeHandle node) = 0;
        virtual void SetNodePosition(SceneNodeHandle node, float x, float y, float z) = 0;
        virtual void SetNodeScale(SceneNodeHandle node, float x, float y, float z) = 0;
        virtual void SetNodeRotation(SceneNodeHandle node, float yaw, float pitch, float roll) = 0;
        virtual SceneVector3 GetNodePosition(SceneNodeHandle node) const = 0;

        virtual SceneItemHandle CreateItem(const std::string& name, ResourceHandle mesh, SceneNodeHandle node) = 0;
        virtual SceneItemHandle GetItem(const std::string& name) const = 0;
        virtual void DestroyItem(SceneItemHandle item) = 0;
        virtual void SetMaterial(SceneItemHandle item, MaterialHandle material) = 0;
        virtual void SetSubMaterial(SceneItemHandle item, std::size_t subItemIndex, MaterialHandle material) = 0;

    protected:
        SceneManager() = default;
    };

    Owner<SceneManager> CreateSceneManager(const std::string& name, MaterialSystem& materials, std::size_t workerThreads = 0);
}
