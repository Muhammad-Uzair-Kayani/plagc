#include "pch.h"
#include "MaterialSystem.h"

#include <OgreRoot.h>
#include <OgreRenderSystem.h>
#include <OgreArchiveManager.h>
#include <OgreHlms.h>
#include <OgreHlmsDatablock.h>
#include <OgreHlmsPbsDatablock.h>
#include <OgreHlmsUnlitDatablock.h>

#include <filesystem>
#include <stdexcept>
#include <utility>

namespace plagc
{
	OgreMaterialSystem::OgreMaterialSystem(const std::string& hlmsPath)
	{
		namespace fs = std::filesystem;

		Ogre::Root& root = Ogre::Root::getSingleton();
		if (!root.getRenderSystem())
			throw std::runtime_error("Ogre RenderSystem must be selected first");

		m_HlmsManager = root.getHlmsManager();
		Ogre::ArchiveManager& archiveManager = Ogre::ArchiveManager::getSingleton();
		
		const fs::path mediaPath = fs::absolute(hlmsPath).lexically_normal();

		if (!fs::is_directory(mediaPath / "Hlms"))
			throw std::runtime_error("Invalid HLMS media directory: " + mediaPath.string());

		auto loadArchive = [&](const Ogre::String& folder) -> Ogre::Archive*
			{
				const fs::path fullPath = mediaPath / fs::path(folder);
				if (!fs::is_directory(fullPath))
					throw std::runtime_error("HLMS directory missing: " + fullPath.string());

				return archiveManager.load(fullPath.generic_string(), "FileSystem", true);
			};

		Ogre::String mainFolder;
		Ogre::StringVector libraryFolders;

		if (!m_HlmsManager->getHlms(Ogre::HLMS_UNLIT))
		{
			Ogre::HlmsUnlit::getDefaultPaths(mainFolder, libraryFolders);
			Ogre::Archive* mainArchive = loadArchive(mainFolder);
			Ogre::ArchiveVec libraries;
			for (const auto& folder : libraryFolders)
				libraries.push_back(loadArchive(folder));

			m_HlmsUnlit = OGRE_NEW Ogre::HlmsUnlit(mainArchive, &libraries);
			m_HlmsManager->registerHlms(m_HlmsUnlit);
		}

		if (!m_HlmsManager->getHlms(Ogre::HLMS_PBS))
		{
			Ogre::HlmsPbs::getDefaultPaths(mainFolder, libraryFolders);
			Ogre::Archive* mainArchive = loadArchive(mainFolder);
			Ogre::ArchiveVec libraries;
			for (const auto& folder : libraryFolders)
				libraries.push_back(loadArchive(folder));

			m_HlmsPbs = OGRE_NEW Ogre::HlmsPbs(mainArchive, &libraries);
			m_HlmsManager->registerHlms(m_HlmsPbs);
		}

		// These pointers must also be populated if HLMS providers already existed.
		m_HlmsUnlit = static_cast<Ogre::HlmsUnlit*>(m_HlmsManager->getHlms(Ogre::HLMS_UNLIT));
		m_HlmsPbs = static_cast<Ogre::HlmsPbs*>(m_HlmsManager->getHlms(Ogre::HLMS_PBS));

		// Enable shader debugging for Vulkan GLSL compilation.
		const fs::path shaderDebugPath = "C:/Personals/Game Engine Development/Debug";

		fs::create_directories(shaderDebugPath);

		m_HlmsPbs->setDebugOutputPath(
			true,
			false,
			shaderDebugPath.generic_string() + "/"
		);

		CORE_INFO("Ogre Material System Initialized");
		CORE_INFO("HLMS Unlit Registered");
		CORE_INFO("HLMS PBS Registered");
	}

	void OgreMaterialSystem::ApplyShadowSettings(const ShadowSettings& settings)
    {
        Ogre::HlmsPbs::ShadowFilter filter;
        switch (settings.Filter)
        {
        case ShadowFilter::PCF_2x2: filter = Ogre::HlmsPbs::PCF_2x2; break;
        case ShadowFilter::PCF_3x3: filter = Ogre::HlmsPbs::PCF_3x3; break;
        case ShadowFilter::PCF_4x4: filter = Ogre::HlmsPbs::PCF_4x4; break;
        case ShadowFilter::PCF_5x5: filter = Ogre::HlmsPbs::PCF_5x5; break;
        case ShadowFilter::PCF_6x6: filter = Ogre::HlmsPbs::PCF_6x6; break;
        case ShadowFilter::ESM: filter = Ogre::HlmsPbs::ExponentialShadowMaps; break;
        default: throw std::invalid_argument("Unsupported shadow filter");
        }

        const bool esm = settings.Filter == ShadowFilter::ESM;
        m_HlmsPbs->setShadowSettings(filter);
        m_HlmsUnlit->setShadowSettings(esm);
        if (esm)
        {
            m_HlmsPbs->setEsmK(settings.EsmK);
            m_HlmsUnlit->setEsmK(settings.EsmK);
        }
    }

	OgreMaterialSystem::~OgreMaterialSystem()
	{
		// Destroy scene Items before this MaterialSystem; Root must still be alive.
		for (const auto& [handle, material] : m_Materials)
		{
			if (!material.Datablock->getLinkedRenderables().empty())
			{
				CORE_ERROR("Material still in use during shutdown: {}", material.Name);
				continue;
			}

			try
			{
				material.Datablock->getCreator()->destroyDatablock(Ogre::IdString(material.Name));
			}
			catch (const std::exception& e)
			{
				CORE_ERROR("Failed to destroy material {}: {}", material.Name, e.what());
			}
		}
	}

	OgreMaterialSystem::MaterialRecord& OgreMaterialSystem::Require(MaterialHandle handle)
	{
		auto it = m_Materials.find(handle);
		if (it == m_Materials.end())
			throw std::runtime_error("Invalid or destroyed material handle");
		return it->second;
	}

	const OgreMaterialSystem::MaterialRecord& OgreMaterialSystem::Require(MaterialHandle handle) const
	{
		auto it = m_Materials.find(handle);
		if (it == m_Materials.end())
			throw std::runtime_error("Invalid or destroyed material handle");
		return it->second;
	}

	MaterialHandle OgreMaterialSystem::RegisterMaterial(MaterialType type, const std::string& name)
	{
		if (name.empty())
			throw std::invalid_argument("Material name cannot be empty");
		if (m_MaterialsByName.contains(name))
			throw std::runtime_error("Material already registered: " + name);

		const Ogre::IdString id(name);
		if (m_HlmsPbs->getDatablock(id) || m_HlmsUnlit->getDatablock(id))
			throw std::runtime_error("Ogre material name already exists: " + name);

		Ogre::Hlms* provider = nullptr;
		switch (type)
		{
		case MaterialType::Unlit: provider = m_HlmsUnlit; break;
		case MaterialType::PBS:   provider = m_HlmsPbs; break;
		default: throw std::invalid_argument("Unsupported material type");
		}

		Ogre::HlmsDatablock* datablock = provider->createDatablock(
			id, name, Ogre::HlmsMacroblock(), Ogre::HlmsBlendblock(), Ogre::HlmsParamVec());

		try
		{
			if (type == MaterialType::Unlit)
			{
				auto& unlit = dynamic_cast<Ogre::HlmsUnlitDatablock&>(*datablock);
				unlit.setUseColour(true);
				unlit.setColour(Ogre::ColourValue::White);
			}
			else
			{
				auto& pbs = dynamic_cast<Ogre::HlmsPbsDatablock&>(*datablock);
				pbs.setWorkflow(Ogre::HlmsPbsDatablock::MetallicWorkflow);
				pbs.setRoughness(0.5f);
				pbs.setMetalness(0.0f);
			}

			const MaterialHandle handle = m_NextHandle;
			m_Materials.emplace(handle, MaterialRecord{ type, name, datablock });
			try
			{
				m_MaterialsByName.emplace(name, handle);
			}
			catch (...)
			{
				m_Materials.erase(handle);
				throw;
			}

			++m_NextHandle;
			CORE_INFO("Material registered: {}", name);
			return handle;
		}
		catch (...)
		{
			provider->destroyDatablock(id);
			throw;
		}
	}

	MaterialHandle OgreMaterialSystem::GetMaterial(const std::string& name) const
	{
		auto it = m_MaterialsByName.find(name);
		return it == m_MaterialsByName.end() ? InvalidMaterial : it->second;
	}

	void OgreMaterialSystem::DestroyMaterial(MaterialHandle handle)
	{
		const MaterialRecord& material = Require(handle);
		if (!material.Datablock->getLinkedRenderables().empty())
			throw std::runtime_error("Cannot destroy material while in use: " + material.Name);

		const Ogre::IdString id(material.Name);
		const std::string name = material.Name;
		material.Datablock->getCreator()->destroyDatablock(id);
		m_MaterialsByName.erase(name);
		m_Materials.erase(handle);
		CORE_INFO("Material destroyed: {}", name);
	}

	Ogre::HlmsDatablock* OgreMaterialSystem::GetOgreDatablock(MaterialHandle handle) const
	{
		return Require(handle).Datablock;
	}

	void OgreMaterialSystem::SetColor(MaterialHandle handle, const MaterialColor& color)
	{
		MaterialRecord& material = Require(handle);
		switch (material.Type)
		{
		case MaterialType::Unlit:
		{
			auto& unlit = dynamic_cast<Ogre::HlmsUnlitDatablock&>(*material.Datablock);
			unlit.setUseColour(true);
			unlit.setColour(Ogre::ColourValue(color.R, color.G, color.B, color.A));
			break;
		}
		case MaterialType::PBS:
		{
			auto& pbs = dynamic_cast<Ogre::HlmsPbsDatablock&>(*material.Datablock);
			pbs.setDiffuse(Ogre::Vector3(color.R, color.G, color.B));
			break;
		}
		default:
			throw std::invalid_argument("Unsupported material type");
		}
	}

	void OgreMaterialSystem::SetRoughness(MaterialHandle handle, float roughness)
	{
		MaterialRecord& material = Require(handle);
		if (material.Type != MaterialType::PBS)
			throw std::runtime_error("Roughness is supported only for PBS");
		if (roughness <= 0.0f || roughness > 1.0f)
			throw std::invalid_argument("Roughness must be in (0, 1]");

		auto& pbs = dynamic_cast<Ogre::HlmsPbsDatablock&>(*material.Datablock);
		pbs.setRoughness(roughness);
	}

	void OgreMaterialSystem::SetMetalness(MaterialHandle handle, float metalness)
	{
		MaterialRecord& material = Require(handle);
		if (material.Type != MaterialType::PBS)
			throw std::runtime_error("Metalness is supported only for PBS");
		if (metalness < 0.0f || metalness > 1.0f)
			throw std::invalid_argument("Metalness must be in [0, 1]");

		auto& pbs = dynamic_cast<Ogre::HlmsPbsDatablock&>(*material.Datablock);
		pbs.setMetalness(metalness);
	}

    void OgreMaterialSystem::SetReceiveShadows(MaterialHandle material, bool enabled)
    {
        MaterialRecord& record = Require(material);
        if (record.Type != MaterialType::PBS)
            throw std::runtime_error("ReceiveShadows is supported only for PBS materials");
        static_cast<Ogre::HlmsPbsDatablock*>(record.Datablock)->setReceiveShadows(enabled);
    }

	void OgreMaterialSystem::SetTexture(MaterialHandle handle, MaterialTexture slot, const std::string& filename)
	{
		MaterialRecord& material = Require(handle);
		if (material.Type == MaterialType::Unlit)
		{
			if (slot != MaterialTexture::BaseColor)
				throw std::runtime_error("Unsupported Unlit texture slot");

			auto& unlit = dynamic_cast<Ogre::HlmsUnlitDatablock&>(*material.Datablock);
			unlit.setTexture(0, Ogre::String(filename));
			return;
		}
		if (material.Type != MaterialType::PBS)
			throw std::invalid_argument("Unsupported material type");

		auto& pbs = dynamic_cast<Ogre::HlmsPbsDatablock&>(*material.Datablock);
		Ogre::PbsTextureTypes ogreSlot;
		switch (slot)
		{
		case MaterialTexture::BaseColor: ogreSlot = Ogre::PBSM_DIFFUSE; break;
		case MaterialTexture::Normal:    ogreSlot = Ogre::PBSM_NORMAL; break;
		case MaterialTexture::Roughness: ogreSlot = Ogre::PBSM_ROUGHNESS; break;
		case MaterialTexture::Metalness: ogreSlot = Ogre::PBSM_METALLIC; break;
		default: throw std::invalid_argument("Unsupported texture slot");
		}
		pbs.setTexture(ogreSlot, Ogre::String(filename));
	}
}
