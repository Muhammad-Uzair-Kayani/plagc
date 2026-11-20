#pragma once
#include "Contracts/RenderSystem/ResourceManager.h"

#include <OgreMesh2.h>

#include <string>
#include <unordered_map>

namespace Ogre
{
    class ResourceGroupManager;
    class TextureGpu;
    class TextureGpuManager;
}

namespace plagc
{
    class OgreResourceManager final : public ResourceManager
    {
        friend class ResourceManager;

    public:
        ~OgreResourceManager() override;

        void AddResourceLocation(const std::string& path, const std::string& group = "General", bool recursive = false) override;

        ResourceHandle LoadMesh(const std::string& name, const std::string& group = "General") override;
        ResourceHandle LoadTexture(const std::string& name, const std::string& group = "General", TextureUsage usage = TextureUsage::ColorSRGB) override;

        ResourceHandle GetMesh(const std::string& name, const std::string& group = "General") const override;
        ResourceHandle GetTexture(const std::string& name, const std::string& group = "General") const override;
        bool HasResource(ResourceHandle handle) const override;
        void ReleaseResource(ResourceHandle handle) override;

        // Backend-only access for OgreSceneManager and other Ogre implementations.
        Ogre::MeshPtr GetOgreMesh(ResourceHandle handle) const;
        Ogre::TextureGpu* GetOgreTexture(ResourceHandle handle) const;

    private:
        OgreResourceManager();

        enum class Kind { Mesh, Texture };

        struct ResourceRecord
        {
            Kind Type;
            std::string Name;
            std::string Group;
            TextureUsage Usage = TextureUsage::ColorSRGB;
            Ogre::MeshPtr Mesh;
            Ogre::TextureGpu* Texture = nullptr;
        };

        static std::string MakeKey(const std::string& name, const std::string& group);
        void PrepareGroup(const std::string& group);
        const ResourceRecord& Require(ResourceHandle handle, Kind type) const;

        Ogre::ResourceGroupManager* m_GroupManager = nullptr;
        Ogre::TextureGpuManager* m_TextureManager = nullptr;
        std::unordered_map<ResourceHandle, ResourceRecord> m_Resources;
        std::unordered_map<std::string, ResourceHandle> m_MeshesByName;
        std::unordered_map<std::string, ResourceHandle> m_TexturesByName;
        ResourceHandle m_NextHandle = 1;
    };
}
