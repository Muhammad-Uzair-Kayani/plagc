
#pragma once

#include "Contracts/RenderSystem/MaterialSystem.h"

#include <OgreHlmsManager.h>
#include <OgreHlmsUnlit.h>
#include <OgreHlmsPbs.h>

#include <cstdint>
#include <string>
#include <unordered_map>

namespace plagc
{
	class OgreMaterialSystem : public MaterialSystem
	{
	public:
		OgreMaterialSystem();
		~OgreMaterialSystem() override;

		MaterialHandle RegisterMaterial(
			MaterialType type,
			const std::string& name
		) override;

		MaterialHandle GetMaterial(
			const std::string& name
		) const override;

		void DestroyMaterial(
			MaterialHandle material
		) override;

		void SetColor(
			MaterialHandle material,
			const MaterialColor& color
		) override;

		void SetRoughness(
			MaterialHandle material,
			float roughness
		) override;

		void SetMetalness(
			MaterialHandle material,
			float metalness
		) override;

		void SetTexture(
			MaterialHandle material,
			MaterialTexture slot,
			const std::string& filename
		) override;

		// Backend-only: used later by OgreSceneManager
		// when assigning a material to an Ogre::SubItem.
		Ogre::HlmsDatablock* GetOgreDatablock(
			MaterialHandle material
		) const;

	private:
		struct MaterialRecord
		{
			MaterialType Type;
			std::string Name;
			Ogre::HlmsDatablock* Datablock = nullptr;
		};

		MaterialRecord& Require(
			MaterialHandle material
		);

		const MaterialRecord& Require(
			MaterialHandle material
		) const;

	private:
		Ogre::HlmsManager* m_HlmsManager = nullptr;
		Ogre::HlmsPbs* m_HlmsPbs = nullptr;
		Ogre::HlmsUnlit* m_HlmsUnlit = nullptr;

		std::unordered_map<
			MaterialHandle,
			MaterialRecord
		> m_Materials;

		std::unordered_map<
			std::string,
			MaterialHandle
		> m_MaterialsByName;

		MaterialHandle m_NextHandle = 1;
	};
}
