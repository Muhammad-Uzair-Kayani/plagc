#include "pch.h"
#include "OgreRenderer.h"
#include "Concrete/RenderSystem/Ogre/SceneManager.h"
#include "Concrete/RenderSystem/Ogre/Camera.h"
#include "Core/Config.h"
#include "Core/Core.h"

#include <Ogre.h>
#include <OgreAbiUtils.h>
#include <OgreVulkanPlugin.h>
#include <Compositor/OgreCompositorManager2.h>
#include <OgreWindow.h>

#include <cstdint>
#include <stdexcept>
#include <string>

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include "Concrete/Platforms/Windows/WindowsWindow.h"
#include <GLFW/glfw3native.h>
#endif

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

    void OgreRenderer::SetScene(SceneManager& scene, Camera& camera)
    {
        auto* ogreScene = dynamic_cast<OgreSceneManager*>(&scene);
        auto* ogreCamera = dynamic_cast<OgreCamera*>(&camera);
        if (!ogreScene || !ogreCamera) throw std::invalid_argument("OgreRenderer requires Ogre scene and camera implementations");

        Ogre::SceneManager* nativeScene = ogreScene->GetOgreSceneManager();
        Ogre::Camera* nativeCamera = ogreCamera->GetOgreCamera();
        if (!nativeScene || !nativeCamera || nativeCamera->getSceneManager() != nativeScene)
            throw std::invalid_argument("Camera must belong to the supplied SceneManager");

        // Create first so a failed bind leaves the previous workspace intact.
        auto* newWorkspace = m_Compositor->addWorkspace(nativeScene, m_RenderWindow->getTexture(), nativeCamera, "plagc.BasicWorkspace", true);
        ClearScene();
        m_Workspace = newWorkspace;
    }

    void OgreRenderer::ClearScene()
    {
        if (m_Workspace && m_Compositor) m_Compositor->removeWorkspace(m_Workspace);
        m_Workspace = nullptr;
    }

    void OgreRenderer::OnWindowResize()
    {
        if (m_RenderWindow) m_RenderWindow->windowMovedOrResized();
    }

    void OgreRenderer::OnUpdate()
    {
        if (!m_Workspace) return;
        if (!m_Root->renderOneFrame()) throw std::runtime_error("Ogre renderOneFrame failed");
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
