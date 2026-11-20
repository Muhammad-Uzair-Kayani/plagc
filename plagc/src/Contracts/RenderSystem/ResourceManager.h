#pragma once
#include "Core/Core.h"

#include <cstdint>
#include <string>

namespace plagc
{
    using ResourceHandle = std::uint64_t;
    constexpr ResourceHandle InvalidResource = 0;

    enum class TextureUsage
    {
        ColorSRGB,
        Linear,
        Normal
    };

    class ResourceManager
    {
    public:
        virtual ~ResourceManager() = default;

        static ResourceManager& Get();
        static void Shutdown();

        virtual void AddResourceLocation(const std::string& path, const std::string& group = "General", bool recursive = false) = 0;

        virtual ResourceHandle LoadMesh(const std::string& name, const std::string& group = "General") = 0;
        virtual ResourceHandle LoadTexture(const std::string& name, const std::string& group = "General", TextureUsage usage = TextureUsage::ColorSRGB) = 0;

        virtual ResourceHandle GetMesh(const std::string& name, const std::string& group = "General") const = 0;
        virtual ResourceHandle GetTexture(const std::string& name, const std::string& group = "General") const = 0;
        virtual bool HasResource(ResourceHandle handle) const = 0;
        virtual void ReleaseResource(ResourceHandle handle) = 0;

    protected:
        ResourceManager() = default;
    };
}
