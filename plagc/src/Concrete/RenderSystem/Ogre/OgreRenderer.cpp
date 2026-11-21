#include "pch.h"
#include "OgreRenderer.h"
#include "Concrete/RenderSystem/Ogre/SceneManager.h"
#include "Concrete/RenderSystem/Ogre/Camera.h"
#include "Concrete/RenderSystem/Ogre/MaterialSystem.h"
#include "Core/Config.h"
#include "Core/Core.h"

#include <Ogre.h>
#include <OgreAbiUtils.h>
#include <OgreVulkanPlugin.h>
#include <Compositor/OgreCompositorManager2.h>
#include <Compositor/OgreCompositorWorkspace.h>
#include <Compositor/OgreCompositorShadowNode.h>
#include <Compositor/OgreCompositorShadowNodeDef.h>
#include <OgreLight.h>
#include <OgreRenderSystem.h>
#include <OgreWindow.h>

#include <cstdint>
#include <stdexcept>
#include <string>

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include "Concrete/Platforms/Windows/WindowsWindow.h"
#endif
#include <GLFW/glfw3native.h>

namespace plagc
{
    OgreRenderer::OgreRenderer(Window* window)
    {
        if (!window) throw std::invalid_argument("Window is required to create OgreRenderer");
        if (Ogre::Root::getSingletonPtr()) throw std::runtime_error("An Ogre Root already exists");

        try
        {
            const Ogre::AbiCookie cookie = Ogre::generateAbiCookie();
            m_Root = MakeUnique<Ogre::Root>(&cookie, "", "", "ogre.log", "plagc");
            m_VulkanPlugin = MakeUnique<Ogre::VulkanPlugin>();
            m_Root->installPlugin(m_VulkanPlugin.get(), nullptr);

            Ogre::RenderSystem* renderSystem = nullptr;
            switch (GET_RENDERING_API())
            {
            case RenderingAPI::Vulkan:
                renderSystem = m_Root->getRenderSystemByName("Vulkan Rendering Subsystem");
                break;
            default:
                throw std::runtime_error("Rendering API not supported yet");
            }

            if (!renderSystem) throw std::runtime_error("Vulkan RenderSystem is unavailable");
            m_Root->setRenderSystem(renderSystem);
            m_Root->initialise(false);
            InitWindow(window);

            m_Compositor = m_Root->getCompositorManager2();
            if (!m_Compositor) throw std::runtime_error("Ogre CompositorManager2 is unavailable");
            m_Compositor->createBasicWorkspaceDef("plagc.BasicWorkspace", Ogre::ColourValue(0.15f, 0.25f, 0.40f), Ogre::IdString());
            CORE_DEBUG("{0} Backend Initialized", renderSystem->getName());
        }
        catch (...)
        {
            Shutdown();
            throw;
        }
    }

    void OgreRenderer::InitWindow(Window* window)
    {
#ifdef _WIN32
        auto* nativeWindow = dynamic_cast<WindowsWindow*>(window);
        if (!nativeWindow || !nativeWindow->GetNativeWindow()) throw std::runtime_error("Expected a valid GLFW WindowsWindow");

        HWND hwnd = glfwGetWin32Window(nativeWindow->GetNativeWindow());
        if (!hwnd) throw std::runtime_error("Unable to obtain native Win32 window handle");

        const auto* data = nativeWindow->GetWindowsWindowData();
        if (!data) throw std::runtime_error("WindowsWindow data is unavailable");

        Ogre::NameValuePairList params;
        params["externalWindowHandle"] = std::to_string(reinterpret_cast<std::uintptr_t>(hwnd));
        m_RenderWindow = m_Root->createRenderWindow(data->Name, data->Width, data->Height, data->Fullscreen, &params);
        if (!m_RenderWindow) throw std::runtime_error("Failed to create Ogre render window");
#else
        throw std::runtime_error("External GLFW window integration is currently implemented only for Windows");
#endif
    }

    void OgreRenderer::CreateShadowNode(const std::string& name, const ShadowSettings& settings)
    {
        const auto validResolution = [](std::uint32_t size) { return size > 0 && size <= 8192; };
        if ((settings.DirectionalResolution && !validResolution(settings.DirectionalResolution)) ||
            (settings.MaxSpotLights && !validResolution(settings.SpotResolution)) ||
            (settings.MaxPointLights && !validResolution(settings.PointResolution)))
            throw std::invalid_argument("Invalid shadow-map resolution");
        if (settings.DirectionalResolution && (settings.PssmSplits < 2 || settings.PssmSplits > 4))
            throw std::invalid_argument("PSSM requires 2 to 4 splits");
        if (settings.MaxSpotLights + settings.MaxPointLights > 16 ||
            (!settings.DirectionalResolution && !settings.MaxSpotLights && !settings.MaxPointLights))
            throw std::invalid_argument("Invalid shadow-map light capacity");
        if (!(settings.PssmLambda >= 0.0f && settings.PssmLambda <= 1.0f) ||
            !(settings.SplitPadding >= 0.0f) ||
            !(settings.SplitBlend >= 0.0f && settings.SplitBlend <= 1.0f) ||
            !(settings.SplitFade >= 0.0f && settings.SplitFade <= 1.0f))
            throw std::invalid_argument("Invalid PSSM parameters");
        if (settings.Filter == ShadowFilter::ESM && !settings.EsmK)
            throw std::invalid_argument("ESM K must be positive");

        Ogre::ShadowNodeHelper::ShadowParamVec params;
        std::uint8_t atlas = 0;
        if (settings.DirectionalResolution)
        {
            Ogre::ShadowNodeHelper::ShadowParam shadow{};
            shadow.technique = Ogre::SHADOWMAP_PSSM;
            shadow.numPssmSplits = settings.PssmSplits;
            shadow.atlasId = atlas++;
            shadow.addLightType(Ogre::Light::LT_DIRECTIONAL);
            for (std::uint8_t split = 0; split < settings.PssmSplits; ++split)
            {
                const auto size = settings.DirectionalResolution;
                shadow.resolution[split] = Ogre::ShadowNodeHelper::Resolution(size, size);
                shadow.atlasStart[split] = Ogre::ShadowNodeHelper::Resolution((split % 2) * size, (split / 2) * size);
            }
            params.push_back(shadow);
        }

        for (std::uint8_t i = 0; i < settings.MaxSpotLights; ++i)
        {
            Ogre::ShadowNodeHelper::ShadowParam shadow{};
            shadow.technique = Ogre::SHADOWMAP_FOCUSED;
            shadow.atlasId = atlas++;
            shadow.addLightType(Ogre::Light::LT_SPOTLIGHT);
            shadow.resolution[0] = Ogre::ShadowNodeHelper::Resolution(settings.SpotResolution, settings.SpotResolution);
            shadow.atlasStart[0] = Ogre::ShadowNodeHelper::Resolution(0, 0);
            params.push_back(shadow);
        }

        for (std::uint8_t i = 0; i < settings.MaxPointLights; ++i)
        {
            Ogre::ShadowNodeHelper::ShadowParam shadow{};
            shadow.technique = Ogre::SHADOWMAP_FOCUSED;
            shadow.atlasId = atlas++;
            shadow.addLightType(Ogre::Light::LT_POINT);
            shadow.resolution[0] = Ogre::ShadowNodeHelper::Resolution(settings.PointResolution, settings.PointResolution);
            shadow.atlasStart[0] = Ogre::ShadowNodeHelper::Resolution(0, 0);
            params.push_back(shadow);
        }

        Ogre::ShadowNodeHelper::createShadowNodeWithSettings(m_Compositor,
            m_Root->getRenderSystem()->getCapabilities(), name, params, settings.Filter == ShadowFilter::ESM,
            settings.PointResolution, settings.PssmLambda, settings.SplitPadding, settings.SplitBlend, settings.SplitFade);
    }

    void OgreRenderer::SetScene(SceneManager& scene, Camera& camera)
    {
        SetScene(scene, camera, WorkspaceConfig{});
    }

    void OgreRenderer::SetScene(SceneManager& scene, Camera& camera, const WorkspaceConfig& config)
    {
        auto* ogreScene = dynamic_cast<OgreSceneManager*>(&scene);
        auto* ogreCamera = dynamic_cast<OgreCamera*>(&camera);
        if (!ogreScene || !ogreCamera) throw std::invalid_argument("OgreRenderer requires Ogre scene and camera implementations");

        Ogre::SceneManager* nativeScene = ogreScene->GetOgreSceneManager();
        Ogre::Camera* nativeCamera = ogreCamera->GetOgreCamera();
        if (!nativeScene || !nativeCamera || nativeCamera->getSceneManager() != nativeScene)
            throw std::invalid_argument("Camera must belong to the supplied SceneManager");

        const bool shadows = config.Shadows.has_value();
        const std::string workspaceName = shadows ? "plagc.ShadowWorkspace." + std::to_string(m_NextWorkspace++) : "plagc.BasicWorkspace";
        const std::string shadowName = shadows ? "plagc.ShadowNode." + std::to_string(m_NextWorkspace - 1) : "";
        Ogre::CompositorWorkspace* newWorkspace = nullptr;

        try
        {
            if (shadows)
            {
                CORE_INFO("Shadow debug: creating shadow node");
                CreateShadowNode(shadowName, *config.Shadows);
                CORE_INFO("Shadow debug: shadow node created");
                m_Compositor->createBasicWorkspaceDef(workspaceName,
                    Ogre::ColourValue(0.15f, 0.25f, 0.40f), Ogre::IdString(shadowName));
                CORE_INFO("Shadow debug: workspace definition created");
            }

            // Create before replacing the old workspace so failures leave it active.
            CORE_INFO("Shadow debug: instantiating workspace");
            newWorkspace = m_Compositor->addWorkspace(nativeScene, m_RenderWindow->getTexture(), nativeCamera, workspaceName, true);
            CORE_INFO("Shadow debug: workspace instantiated");
            if (!newWorkspace || !newWorkspace->isValid())
                throw std::runtime_error("Ogre compositor returned an invalid workspace");

            CORE_INFO("Shadow debug: applying HLMS shadow settings");
            ogreScene->GetOgreMaterialSystem()->ApplyShadowSettings(shadows ? *config.Shadows : ShadowSettings{});
            CORE_INFO("Shadow debug: HLMS shadow settings applied");
        }
        catch (...)
        {
            if (newWorkspace) m_Compositor->removeWorkspace(newWorkspace);
            if (shadows && m_Compositor->hasWorkspaceDefinition(Ogre::IdString(workspaceName)))
                m_Compositor->removeWorkspaceDefinition(Ogre::IdString(workspaceName));
            if (shadows && m_Compositor->hasNodeDefinition(Ogre::IdString(workspaceName + "_Node")))
                m_Compositor->removeNodeDefinition(Ogre::IdString(workspaceName + "_Node"));
            if (shadows && m_Compositor->hasShadowNodeDefinition(Ogre::IdString(shadowName)))
                m_Compositor->removeShadowNodeDefinition(Ogre::IdString(shadowName));
            throw;
        }

        CORE_INFO("Shadow debug: replacing previous workspace");
        ClearScene();
        m_Workspace = newWorkspace;
        m_WorkspaceDefinition = shadows ? workspaceName : "";
        m_ShadowNodeDefinition = shadowName;
    }

    void OgreRenderer::ClearScene()
    {
        if (m_Workspace && m_Compositor) m_Compositor->removeWorkspace(m_Workspace);
        m_Workspace = nullptr;
        if (m_Compositor && !m_WorkspaceDefinition.empty())
            m_Compositor->removeWorkspaceDefinition(Ogre::IdString(m_WorkspaceDefinition));
        if (m_Compositor && !m_WorkspaceDefinition.empty() &&
            m_Compositor->hasNodeDefinition(Ogre::IdString(m_WorkspaceDefinition + "_Node")))
            m_Compositor->removeNodeDefinition(Ogre::IdString(m_WorkspaceDefinition + "_Node"));
        if (m_Compositor && !m_ShadowNodeDefinition.empty())
            m_Compositor->removeShadowNodeDefinition(Ogre::IdString(m_ShadowNodeDefinition));
        m_WorkspaceDefinition.clear();
        m_ShadowNodeDefinition.clear();
    }

    void OgreRenderer::OnWindowResize()
    {
        if (m_RenderWindow) m_RenderWindow->windowMovedOrResized();
    }

    void OgreRenderer::OnUpdate()
    {
        if (!m_Workspace) return;
        if (!m_Workspace->isValid()) throw std::runtime_error("Cannot update an invalid Ogre workspace");
        CORE_DEBUG("Shadow debug: entering renderOneFrame");
        if (!m_Root->renderOneFrame()) throw std::runtime_error("Ogre renderOneFrame failed");
        CORE_DEBUG("Shadow debug: renderOneFrame completed");
    }

    OgreRenderer::~OgreRenderer()
    {
        Shutdown();
    }

    void OgreRenderer::Shutdown()
    {
        if (!m_Root) return;
        ClearScene();
        if (m_RenderWindow)
        {
            m_Root->getRenderSystem()->destroyRenderWindow(m_RenderWindow);
            m_RenderWindow = nullptr;
        }
        m_Compositor = nullptr;
        m_Root.reset();
        m_VulkanPlugin.reset();
    }
}
