#pragma once
#include "Core/Core.h"

#include <cstdint>
#include <string>

namespace plagc
{
	enum class MaterialType
	{
		Unlit,
		PBS
	};

	enum class MaterialTexture
	{
		BaseColor,
		Normal,
		Roughness,
		Metalness
	};

	struct MaterialColor
	{
		float R = 1.0f;
		float G = 1.0f;
		float B = 1.0f;
		float A = 1.0f;
	};

	using MaterialHandle = std::uint64_t;

	constexpr MaterialHandle InvalidMaterial = 0;

	class MaterialSystem
	{
	public:
		virtual ~MaterialSystem() = default;

		virtual MaterialHandle RegisterMaterial( MaterialType type, const std::string& name) = 0;

		virtual MaterialHandle GetMaterial( const std::string& name ) const = 0;

		virtual void DestroyMaterial( MaterialHandle material) = 0;

		virtual void SetColor( MaterialHandle material, const MaterialColor& color ) = 0;

		virtual void SetRoughness( MaterialHandle material, float roughness ) = 0;

		virtual void SetMetalness( MaterialHandle material, float metalness ) = 0;

		virtual void SetTexture( MaterialHandle material, MaterialTexture slot, const std::string& filename ) = 0;
	};

    Owner<MaterialSystem> CreateMaterialSystem(const std::string& hlmsPath);
}