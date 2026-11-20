#pragma once
#include "plagc.h"
#include <chrono>
#include <cstddef>
#include <string>
#include <vector>

namespace sandbox
{
    struct RenderingTestAssets
    {
        std::string MeshDirectory;
        std::string TextureDirectory;
        std::string MeshFile;
        std::string AlbedoFile;
        std::string NormalFile;
        std::string RoughnessFile;
        std::string MetalnessFile;
        std::size_t InstanceCount = 256;
    };

    class RenderingStressTest final
    {
    public:
        RenderingStressTest(plagc::Renderer& renderer, plagc::MaterialSystem& materials,
                            plagc::SceneManager& mainScene, plagc::SceneManager& secondaryScene);
        ~RenderingStressTest();

        void Initialize(const RenderingTestAssets& assets);
        void OnUpdate();
        void Shutdown();

    private:
        plagc::Renderer& m_Renderer;
        plagc::MaterialSystem& m_Materials;
        plagc::SceneManager& m_MainScene;
        plagc::SceneManager& m_SecondaryScene;

        plagc::Camera* m_MainCamera = nullptr;
        plagc::Camera* m_SecondaryCamera = nullptr;
        plagc::ResourceHandle m_Mesh = plagc::InvalidResource;
        plagc::ResourceHandle m_Albedo = plagc::InvalidResource;
        plagc::ResourceHandle m_Normal = plagc::InvalidResource;
        plagc::ResourceHandle m_Roughness = plagc::InvalidResource;
        plagc::ResourceHandle m_Metalness = plagc::InvalidResource;
        plagc::MaterialHandle m_Unlit = plagc::InvalidMaterial;
        plagc::MaterialHandle m_Pbs = plagc::InvalidMaterial;
        plagc::SceneNodeHandle m_Parent = plagc::InvalidSceneNode;
        plagc::SceneNodeHandle m_Child = plagc::InvalidSceneNode;
        plagc::SceneNodeHandle m_SecondaryNode = plagc::InvalidSceneNode;
        plagc::SceneItemHandle m_Sample = plagc::InvalidSceneItem;
        plagc::SceneItemHandle m_SecondaryItem = plagc::InvalidSceneItem;
        std::vector<plagc::SceneNodeHandle> m_StressNodes;
        std::vector<plagc::SceneItemHandle> m_StressItems;
        std::chrono::steady_clock::time_point m_LastFpsLog{};
        unsigned int m_Frame = 0;
        bool m_Initialized = false;
    };
}
