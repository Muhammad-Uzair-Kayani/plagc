#include "pch.h"
#include "ResourceManager.h"

#include <OgreRoot.h>
#include <OgreRenderSystem.h>
#include <OgreResourceGroupManager.h>
#include <OgreMeshManager2.h>
#include <OgreTextureGpuManager.h>
#include <OgreTextureGpu.h>

#include <filesystem>
#include <stdexcept>
#include <utility>

namespace plagc
{
    OgreResourceManager::OgreResourceManager()
    {
        Ogre::Root* root = Ogre::Root::getSingletonPtr();
        if (!root || !root->isInitialised() || !root->getRenderSystem())
            throw std::runtime_error("Initialize Ogre Root and RenderSystem before ResourceManager");

        m_GroupManager = &Ogre::ResourceGroupManager::getSingleton();
        m_TextureManager = root->getRenderSystem()->getTextureGpuManager();
        if (!m_TextureManager)
            throw std::runtime_error("Ogre TextureGpuManager is unavailable");
    }

    OgreResourceManager::~OgreResourceManager()
    {
        m_Resources.clear(); // Ogre managers and scene resources must outlive this object.
        m_MeshesByName.clear();
        m_TexturesByName.clear();
    }

    std::string OgreResourceManager::MakeKey(const std::string& name, const std::string& group)
    {
        return group + '\n' + name;
    }

    void OgreResourceManager::AddResourceLocation(const std::string& path, const std::string& group, bool recursive)
    {
        namespace fs = std::filesystem;
        if (path.empty() || group.empty()) throw std::invalid_argument("Resource path and group cannot be empty");

        const fs::path absolutePath = fs::absolute(fs::path(path)).lexically_normal();
        if (!fs::is_directory(absolutePath))
            throw std::runtime_error("Resource directory not found: " + absolutePath.string());

        const Ogre::String location = absolutePath.generic_string();
        if (!m_GroupManager->resourceGroupExists(group)) m_GroupManager->createResourceGroup(group);
        if (!m_GroupManager->resourceLocationExists(location, group))
            m_GroupManager->addResourceLocation(location, "FileSystem", group, recursive, true);
    }

    void OgreResourceManager::PrepareGroup(const std::string& group)
    {
        if (group.empty() || !m_GroupManager->resourceGroupExists(group))
            throw std::runtime_error("Unknown resource group: " + group);
        if (!m_GroupManager->isResourceGroupInitialised(group))
            m_GroupManager->initialiseResourceGroup(group, false);
    }

    ResourceHandle OgreResourceManager::LoadMesh(const std::string& name, const std::string& group)
    {
        if (name.empty()) throw std::invalid_argument("Mesh name cannot be empty");
        const std::string key = MakeKey(name, group);
        if (const ResourceHandle existing = GetMesh(name, group)) return existing;

        PrepareGroup(group);
        if (!m_GroupManager->resourceExists(group, name))
            throw std::runtime_error("Mesh file not found: " + group + "/" + name);

        Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().load(name, group);
        if (!mesh) throw std::runtime_error("Failed to load mesh: " + name);

        const ResourceHandle handle = m_NextHandle;
        m_Resources.emplace(handle, ResourceRecord{ Kind::Mesh, name, group, TextureUsage::Linear, mesh, nullptr });
        try { m_MeshesByName.emplace(key, handle); }
        catch (...) { m_Resources.erase(handle); throw; }
        ++m_NextHandle;
        return handle;
    }

    ResourceHandle OgreResourceManager::LoadTexture(const std::string& name, const std::string& group, TextureUsage usage)
    {
        if (name.empty()) throw std::invalid_argument("Texture name cannot be empty");
        const std::string key = MakeKey(name, group);
        if (const ResourceHandle existing = GetTexture(name, group))
        {
            if (Require(existing, Kind::Texture).Usage != usage)
                throw std::runtime_error("Texture already loaded with a different usage: " + name);
            return existing;
        }

        // Ogre texture aliases are global. Keep names unambiguous across groups.
        for (const auto& [handle, record] : m_Resources)
        {
            if (record.Type == Kind::Texture && record.Name == name && record.Group != group)
                throw std::runtime_error("Texture name already used in another group: " + name);
        }

        PrepareGroup(group);
        if (!m_GroupManager->resourceExists(group, name))
            throw std::runtime_error("Texture file not found: " + group + "/" + name);

        Ogre::uint32 flags = Ogre::TextureFlags::AutomaticBatching;
        Ogre::uint32 filters = Ogre::TextureFilter::TypeGenerateDefaultMipmaps;
        if (usage == TextureUsage::ColorSRGB) flags |= Ogre::TextureFlags::PrefersLoadingFromFileAsSRGB;
        if (usage == TextureUsage::Normal) filters |= Ogre::TextureFilter::TypePrepareForNormalMapping;

        Ogre::TextureGpu* texture = m_TextureManager->createOrRetrieveTexture(
            name, Ogre::GpuPageOutStrategy::Discard, flags, Ogre::TextureTypes::Type2D, group, filters);
        if (!texture) throw std::runtime_error("Failed to create texture: " + name);

        texture->scheduleTransitionTo(Ogre::GpuResidency::Resident);
        texture->waitForData();
        if (!texture->isDataReady()) throw std::runtime_error("Texture data is not ready: " + name);

        const ResourceHandle handle = m_NextHandle;
        m_Resources.emplace(handle, ResourceRecord{ Kind::Texture, name, group, usage, Ogre::MeshPtr(), texture });
        try { m_TexturesByName.emplace(key, handle); }
        catch (...) { m_Resources.erase(handle); throw; }
        ++m_NextHandle;
        return handle;
    }

    ResourceHandle OgreResourceManager::GetMesh(const std::string& name, const std::string& group) const
    {
        const auto it = m_MeshesByName.find(MakeKey(name, group));
        return it == m_MeshesByName.end() ? InvalidResource : it->second;
    }

    ResourceHandle OgreResourceManager::GetTexture(const std::string& name, const std::string& group) const
    {
        const auto it = m_TexturesByName.find(MakeKey(name, group));
        return it == m_TexturesByName.end() ? InvalidResource : it->second;
    }

    bool OgreResourceManager::HasResource(ResourceHandle handle) const
    {
        return m_Resources.contains(handle);
    }

    const OgreResourceManager::ResourceRecord& OgreResourceManager::Require(ResourceHandle handle, Kind type) const
    {
        const auto it = m_Resources.find(handle);
        if (it == m_Resources.end() || it->second.Type != type)
            throw std::runtime_error("Invalid resource handle or incorrect resource type");
        return it->second;
    }

    Ogre::MeshPtr OgreResourceManager::GetOgreMesh(ResourceHandle handle) const
    {
        return Require(handle, Kind::Mesh).Mesh;
    }

    Ogre::TextureGpu* OgreResourceManager::GetOgreTexture(ResourceHandle handle) const
    {
        return Require(handle, Kind::Texture).Texture;
    }

    void OgreResourceManager::ReleaseResource(ResourceHandle handle)
    {
        const auto it = m_Resources.find(handle);
        if (it == m_Resources.end()) throw std::runtime_error("Invalid or released resource handle");

        const std::string key = MakeKey(it->second.Name, it->second.Group);
        if (it->second.Type == Kind::Mesh) m_MeshesByName.erase(key);
        else m_TexturesByName.erase(key);
        m_Resources.erase(it); // Never forcibly destroy native resources still referenced elsewhere.
    }
}
