#include "pch.h"
#include "SceneManager.h"
#include "Camera.h"
#include "Light.h"
#include "ResourceManager.h"
#include "MaterialSystem.h"

#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreCamera.h>
#include <OgreLight.h>
#include <OgreItem.h>
#include <OgreSubItem.h>
#include <OgreHlmsDatablock.h>
#include <OgreColourValue.h>
#include <OgreVector3.h>

#include <stdexcept>
#include <utility>

namespace plagc
{
    OgreSceneManager::OgreSceneManager(const std::string& name, MaterialSystem& materials, std::size_t workerThreads)
    {
        if (name.empty()) throw std::invalid_argument("Scene name cannot be empty");
        m_Materials = dynamic_cast<OgreMaterialSystem*>(&materials);
        if (!m_Materials) throw std::invalid_argument("OgreSceneManager requires OgreMaterialSystem");

        Ogre::Root* root = Ogre::Root::getSingletonPtr();
        if (!root || !root->isInitialised()) throw std::runtime_error("Initialize Ogre Root before creating a scene");
        if (root->hasSceneManager(name)) throw std::runtime_error("Scene already exists: " + name);

        m_SceneManager = root->createSceneManager(Ogre::ST_GENERIC, workerThreads, name);
        if (!m_SceneManager) throw std::runtime_error("Failed to create Ogre SceneManager: " + name);
    }

    OgreSceneManager::~OgreSceneManager()
    {
        // Remove compositor workspaces using this scene or its cameras before destroying it.
        m_Cameras.clear();
        m_Lights.clear();
        m_Items.clear();
        m_Nodes.clear();
        m_ItemsByName.clear();
        m_NodesByName.clear();
        if (m_SceneManager) Ogre::Root::getSingleton().destroySceneManager(m_SceneManager);
    }

    Camera* OgreSceneManager::CreateCamera(const std::string& name)
    {
        if (name.empty()) throw std::invalid_argument("Camera name cannot be empty");
        if (m_Cameras.contains(name)) throw std::runtime_error("Camera already exists: " + name);

        Ogre::Camera* native = m_SceneManager->createCamera(name);
        try
        {
            CameraRecord record{ Owner<Camera>(new OgreCamera(native)), native };
            auto [it, inserted] = m_Cameras.emplace(name, std::move(record));
            return it->second.Wrapper.get();
        }
        catch (...)
        {
            m_SceneManager->destroyCamera(native);
            throw;
        }
    }

    Camera* OgreSceneManager::GetCamera(const std::string& name) const
    {
        auto it = m_Cameras.find(name);
        return it == m_Cameras.end() ? nullptr : it->second.Wrapper.get();
    }

    void OgreSceneManager::DestroyCamera(const std::string& name)
    {
        auto it = m_Cameras.find(name);
        if (it == m_Cameras.end()) throw std::runtime_error("Unknown camera: " + name);
        // The renderer must first remove any workspace using this camera.
        m_SceneManager->destroyCamera(it->second.Native);
        m_Cameras.erase(it);
    }

    Light* OgreSceneManager::CreateLight(const std::string& name, SceneNodeHandle parent, LightType type)
    {
        if (name.empty()) throw std::invalid_argument("Light name cannot be empty");
        if (m_Lights.contains(name)) throw std::runtime_error("Light already exists: " + name);

        Ogre::SceneNode* parentNode = parent == InvalidSceneNode ? m_SceneManager->getRootSceneNode() : RequireNode(parent);
        Ogre::SceneNode* transform = parentNode->createChildSceneNode();
        Ogre::Light* native = nullptr;
        try
        {
            native = m_SceneManager->createLight();
            transform->attachObject(native);
            Owner<Light> wrapper;
            switch (type)
            {
            case LightType::Directional:
                native->setType(Ogre::Light::LT_DIRECTIONAL);
                wrapper = Owner<Light>(new OgreDirectionalLight(native, transform));
                break;
            case LightType::Point:
                native->setType(Ogre::Light::LT_POINT);
                wrapper = Owner<Light>(new OgrePointLight(native, transform));
                break;
            case LightType::Spot:
                native->setType(Ogre::Light::LT_SPOTLIGHT);
                wrapper = Owner<Light>(new OgreSpotLight(native, transform));
                break;
            default:
                throw std::invalid_argument("Unsupported light type");
            }

            LightRecord record{ std::move(wrapper), native, transform };
            auto [it, inserted] = m_Lights.emplace(name, std::move(record));
            return it->second.Wrapper.get();
        }
        catch (...)
        {
            if (native) m_SceneManager->destroyLight(native);
            m_SceneManager->destroySceneNode(transform);
            throw;
        }
    }

    DirectionalLight* OgreSceneManager::CreateDirectionalLight(const std::string& name, SceneNodeHandle parent)
    {
        return static_cast<DirectionalLight*>(CreateLight(name, parent, LightType::Directional));
    }

    PointLight* OgreSceneManager::CreatePointLight(const std::string& name, SceneNodeHandle parent)
    {
        return static_cast<PointLight*>(CreateLight(name, parent, LightType::Point));
    }

    SpotLight* OgreSceneManager::CreateSpotLight(const std::string& name, SceneNodeHandle parent)
    {
        return static_cast<SpotLight*>(CreateLight(name, parent, LightType::Spot));
    }

    Light* OgreSceneManager::GetLight(const std::string& name) const
    {
        auto it = m_Lights.find(name);
        return it == m_Lights.end() ? nullptr : it->second.Wrapper.get();
    }

    DirectionalLight* OgreSceneManager::GetDirectionalLight(const std::string& name) const
    {
        return dynamic_cast<DirectionalLight*>(GetLight(name));
    }

    PointLight* OgreSceneManager::GetPointLight(const std::string& name) const
    {
        return dynamic_cast<PointLight*>(GetLight(name));
    }

    SpotLight* OgreSceneManager::GetSpotLight(const std::string& name) const
    {
        return dynamic_cast<SpotLight*>(GetLight(name));
    }

    void OgreSceneManager::DestroyLight(const std::string& name)
    {
        auto it = m_Lights.find(name);
        if (it == m_Lights.end()) throw std::runtime_error("Unknown light: " + name);
        m_SceneManager->destroyLight(it->second.Native);
        m_SceneManager->destroySceneNode(it->second.TransformNode);
        m_Lights.erase(it);
    }

    void OgreSceneManager::SetAmbientLight(const AmbientLightProperties& properties)
    {
        const Ogre::Vector3 direction(properties.HemisphereDirection.X,
            properties.HemisphereDirection.Y, properties.HemisphereDirection.Z);
        if (!(direction.squaredLength() > 0.0f)) throw std::invalid_argument("Ambient hemisphere direction cannot be zero");
        if (!(properties.EnvironmentScale >= 0.0f)) throw std::invalid_argument("Ambient environment scale must be non-negative");

        m_SceneManager->setAmbientLight(
            Ogre::ColourValue(properties.UpperColor.R, properties.UpperColor.G, properties.UpperColor.B),
            Ogre::ColourValue(properties.LowerColor.R, properties.LowerColor.G, properties.LowerColor.B),
            direction, properties.EnvironmentScale);
    }

    AmbientLightProperties OgreSceneManager::GetAmbientLight() const
    {
        const auto& upper = m_SceneManager->getAmbientLightUpperHemisphere();
        const auto& lower = m_SceneManager->getAmbientLightLowerHemisphere();
        const auto& direction = m_SceneManager->getAmbientLightHemisphereDir();
        return { { upper.r, upper.g, upper.b }, { lower.r, lower.g, lower.b },
            { direction.x, direction.y, direction.z }, upper.a };
    }

    SceneNodeHandle OgreSceneManager::CreateNode(const std::string& name, SceneNodeHandle parent)
    {
        if (name.empty()) throw std::invalid_argument("Node name cannot be empty");
        if (m_NodesByName.contains(name)) throw std::runtime_error("Node already exists: " + name);

        Ogre::SceneNode* parentNode = parent == InvalidSceneNode ? m_SceneManager->getRootSceneNode() : RequireNode(parent);
        Ogre::SceneNode* native = parentNode->createChildSceneNode();
        try
        {
            const SceneNodeHandle handle = m_NextNode;
            m_Nodes.emplace(handle, NodeRecord{ name, native });
            try { m_NodesByName.emplace(name, handle); }
            catch (...) { m_Nodes.erase(handle); throw; }
            ++m_NextNode;
            return handle;
        }
        catch (...)
        {
            m_SceneManager->destroySceneNode(native);
            throw;
        }
    }

    SceneNodeHandle OgreSceneManager::GetNode(const std::string& name) const
    {
        auto it = m_NodesByName.find(name);
        return it == m_NodesByName.end() ? InvalidSceneNode : it->second;
    }

    Ogre::SceneNode* OgreSceneManager::RequireNode(SceneNodeHandle handle) const
    {
        auto it = m_Nodes.find(handle);
        if (it == m_Nodes.end()) throw std::runtime_error("Invalid scene node handle");
        return it->second.Native;
    }

    void OgreSceneManager::DestroyNode(SceneNodeHandle handle)
    {
        Ogre::SceneNode* native = RequireNode(handle);
        if (native->numChildren() || native->numAttachedObjects())
            throw std::runtime_error("Destroy child nodes and attached Items before destroying this node");
        m_SceneManager->destroySceneNode(native);
        m_NodesByName.erase(m_Nodes.at(handle).Name);
        m_Nodes.erase(handle);
    }

    void OgreSceneManager::SetNodePosition(SceneNodeHandle handle, float x, float y, float z)
    {
        RequireNode(handle)->setPosition(x, y, z);
    }

    void OgreSceneManager::SetNodeScale(SceneNodeHandle handle, float x, float y, float z)
    {
        RequireNode(handle)->setScale(x, y, z);
    }

    void OgreSceneManager::SetNodeRotation(SceneNodeHandle handle, float yaw, float pitch, float roll)
    {
        const Ogre::Quaternion orientation = Ogre::Quaternion(Ogre::Radian(Ogre::Degree(yaw)), Ogre::Vector3::UNIT_Y) *
            Ogre::Quaternion(Ogre::Radian(Ogre::Degree(pitch)), Ogre::Vector3::UNIT_X) *
            Ogre::Quaternion(Ogre::Radian(Ogre::Degree(roll)), Ogre::Vector3::UNIT_Z);
        RequireNode(handle)->setOrientation(orientation);
    }

    SceneVector3 OgreSceneManager::GetNodePosition(SceneNodeHandle handle) const
    {
        const Ogre::Vector3 position = RequireNode(handle)->getPosition();
        return { position.x, position.y, position.z };
    }

    SceneItemHandle OgreSceneManager::CreateItem(const std::string& name, ResourceHandle mesh, SceneNodeHandle node)
    {
        if (name.empty()) throw std::invalid_argument("Item name cannot be empty");
        if (m_ItemsByName.contains(name)) throw std::runtime_error("Item already exists: " + name);

        Ogre::SceneNode* nativeNode = RequireNode(node);
        auto& resources = dynamic_cast<OgreResourceManager&>(ResourceManager::Get());
        Ogre::MeshPtr nativeMesh = resources.GetOgreMesh(mesh);
        Ogre::Item* native = m_SceneManager->createItem(nativeMesh);
        try
        {
            nativeNode->attachObject(native);
            const SceneItemHandle handle = m_NextItem;
            m_Items.emplace(handle, ItemRecord{ name, native });
            try { m_ItemsByName.emplace(name, handle); }
            catch (...) { m_Items.erase(handle); throw; }
            ++m_NextItem;
            return handle;
        }
        catch (...)
        {
            m_SceneManager->destroyItem(native);
            throw;
        }
    }

    SceneItemHandle OgreSceneManager::GetItem(const std::string& name) const
    {
        auto it = m_ItemsByName.find(name);
        return it == m_ItemsByName.end() ? InvalidSceneItem : it->second;
    }

    Ogre::Item* OgreSceneManager::RequireItem(SceneItemHandle handle) const
    {
        auto it = m_Items.find(handle);
        if (it == m_Items.end()) throw std::runtime_error("Invalid scene item handle");
        return it->second.Native;
    }

    void OgreSceneManager::DestroyItem(SceneItemHandle handle)
    {
        Ogre::Item* native = RequireItem(handle);
        m_SceneManager->destroyItem(native);
        m_ItemsByName.erase(m_Items.at(handle).Name);
        m_Items.erase(handle);
    }

    void OgreSceneManager::SetMaterial(SceneItemHandle handle, MaterialHandle material)
    {
        RequireItem(handle)->setDatablock(m_Materials->GetOgreDatablock(material));
    }

    void OgreSceneManager::SetSubMaterial(SceneItemHandle handle, std::size_t subItemIndex, MaterialHandle material)
    {
        Ogre::Item* item = RequireItem(handle);
        if (subItemIndex >= item->getNumSubItems()) throw std::out_of_range("SubItem index out of range");
        item->getSubItem(subItemIndex)->setDatablock(m_Materials->GetOgreDatablock(material));
    }

    void OgreSceneManager::SetItemCastShadows(SceneItemHandle item, bool enabled)
    {
        RequireItem(item)->setCastShadows(enabled);
    }

    bool OgreSceneManager::GetItemCastShadows(SceneItemHandle item) const
    {
        return RequireItem(item)->getCastShadows();
    }
}
