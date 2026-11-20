#pragma once
#include "Contracts/RenderSystem/SceneManager.h"

#include <string>
#include <unordered_map>

namespace Ogre
{
    class Camera;
    class Item;
    class SceneManager;
    class SceneNode;
}

namespace plagc
{
    class OgreMaterialSystem;

    class OgreSceneManager final : public SceneManager
    {
    public:
        OgreSceneManager(const std::string& name, MaterialSystem& materials, std::size_t workerThreads);
        ~OgreSceneManager() override;

        Camera* CreateCamera(const std::string& name) override;
        Camera* GetCamera(const std::string& name) const override;
        void DestroyCamera(const std::string& name) override;

        SceneNodeHandle CreateNode(const std::string& name, SceneNodeHandle parent = InvalidSceneNode) override;
        SceneNodeHandle GetNode(const std::string& name) const override;
        void DestroyNode(SceneNodeHandle node) override;
        void SetNodePosition(SceneNodeHandle node, float x, float y, float z) override;
        void SetNodeScale(SceneNodeHandle node, float x, float y, float z) override;
        void SetNodeRotation(SceneNodeHandle node, float yaw, float pitch, float roll) override;
        SceneVector3 GetNodePosition(SceneNodeHandle node) const override;

        SceneItemHandle CreateItem(const std::string& name, ResourceHandle mesh, SceneNodeHandle node) override;
        SceneItemHandle GetItem(const std::string& name) const override;
        void DestroyItem(SceneItemHandle item) override;
        void SetMaterial(SceneItemHandle item, MaterialHandle material) override;
        void SetSubMaterial(SceneItemHandle item, std::size_t subItemIndex, MaterialHandle material) override;

        // Backend-only: OgreRenderer needs the native scene manager for compositor workspaces.
        Ogre::SceneManager* GetOgreSceneManager() const { return m_SceneManager; }

    private:
        struct CameraRecord
        {
            Owner<Camera> Wrapper;
            Ogre::Camera* Native = nullptr;
        };

        struct NodeRecord
        {
            std::string Name;
            Ogre::SceneNode* Native = nullptr;
        };

        struct ItemRecord
        {
            std::string Name;
            Ogre::Item* Native = nullptr;
        };

        Ogre::SceneNode* RequireNode(SceneNodeHandle handle) const;
        Ogre::Item* RequireItem(SceneItemHandle handle) const;

        Ogre::SceneManager* m_SceneManager = nullptr;
        OgreMaterialSystem* m_Materials = nullptr; // Non-owning; must outlive this scene.
        std::unordered_map<std::string, CameraRecord> m_Cameras;
        std::unordered_map<SceneNodeHandle, NodeRecord> m_Nodes;
        std::unordered_map<SceneItemHandle, ItemRecord> m_Items;
        std::unordered_map<std::string, SceneNodeHandle> m_NodesByName;
        std::unordered_map<std::string, SceneItemHandle> m_ItemsByName;
        SceneNodeHandle m_NextNode = 1;
        SceneItemHandle m_NextItem = 1;
    };
}
