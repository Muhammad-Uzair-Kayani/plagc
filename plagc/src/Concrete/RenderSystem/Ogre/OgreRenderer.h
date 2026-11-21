#pragma once
#include "Contracts/RenderSystem/Renderer.h"

#include <cstdint>
#include <string>

namespace Ogre
{
    class Root;
    class VulkanPlugin;
    class Window;
    class CompositorManager2;
    class CompositorWorkspace;
}

namespace plagc
{
    class OgreRenderer final : public Renderer
    {
    public:
        ~OgreRenderer() override;

        void SetScene(SceneManager& scene, Camera& camera) override;
        void SetScene(SceneManager& scene, Camera& camera, const WorkspaceConfig& config) override;
        void ClearScene() override;
        void OnWindowResize() override;
        void OnUpdate() override;

    private:
        explicit OgreRenderer(Window* window);
        void InitWindow(Window* window);
        void CreateShadowNode(const std::string& name, const ShadowSettings& settings);
        void Shutdown();
        friend class RendererBuilder;

    private:
        // Keep the statically installed plugin alive until Root has shut down.
        Owner<Ogre::VulkanPlugin> m_VulkanPlugin;
        Owner<Ogre::Root> m_Root;
        Ogre::Window* m_RenderWindow = nullptr;
        Ogre::CompositorManager2* m_Compositor = nullptr;
        Ogre::CompositorWorkspace* m_Workspace = nullptr;
        std::string m_WorkspaceDefinition;
        std::string m_ShadowNodeDefinition;
        std::uint64_t m_NextWorkspace = 1;
    };
}
