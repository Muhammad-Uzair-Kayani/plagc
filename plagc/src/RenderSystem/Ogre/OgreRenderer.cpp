#include "pch.h"
#include "OgreRenderer.h"

#include "Core/Config.h"
#include "Core/Core.h"

#include <string>

#include <OgreRenderSystem.h>
#include <OgreAbiUtils.h>
#include <OgreVulkanPlugin.h>

#ifdef _WIN32 

#define GLFW_EXPOSE_NATIVE_WIN32
#include "Platforms/Windows/WindowsWindow.h"

#elif LINUX

#define GLFW_EXPOSE_NATIVE_X11

#endif

#include <GLFW/glfw3native.h>


namespace plagc
{

	OgreRenderer::OgreRenderer(Window* window)
	{
		const Ogre::AbiCookie cookie = Ogre::generateAbiCookie();

		m_Root = MakeUnique<Ogre::Root>(&cookie, "", "", "ogre.log", "plagc");
		m_Plugin = MakeUnique<Ogre::VulkanPlugin>();
		m_Root->installPlugin(m_Plugin.get(), nullptr);

		Ogre::RenderSystem* renderSystem = nullptr;

		switch (GET_RENDERING_API())
		{
		case RenderingAPI::Vulkan:
			renderSystem = m_Root->getRenderSystemByName("Vulkan Rendering Subsystem");
			break;

		default:
			throw std::runtime_error("No Rendering Backend found");
		}

		if (!renderSystem)
			throw std::runtime_error("Rendering Backend Missing");

		CORE_DEBUG("{0} Backend Initialized", renderSystem->getName());

		m_Root->setRenderSystem(renderSystem);
		m_Root->initialise(false);

		Init(window);

		// OGRE owns these objects — store as non-owning raw pointers
		m_SceneManager = m_Root->createSceneManager(Ogre::ST_GENERIC, 1);

		m_Camera = m_SceneManager->createCamera("TestCamera");
		
		m_CompositorManager2 = m_Root->getCompositorManager2();

		const Ogre::String workspaceName("MinimalClearWorkspace");

		m_CompositorManager2->addWorkspace(
			m_SceneManager,
			m_Window->getTexture(),
			m_Camera,
			workspaceName,
			true
		);
	}


	OgreRenderer::~OgreRenderer()
	{
		Shutdown();
	}

	void OgreRenderer::OnUpdate()
	{
		m_Root->renderOneFrame();
	}

	void OgreRenderer::Init(Window* window)
	{

		if (!window)
			throw std::runtime_error("Window is Required to start renderer");

		Ogre::NameValuePairList paras;

		switch (GET_PLATFROM())
		{
		case Platform::Windows:
		{
			auto windowsWindow = dynamic_cast<WindowsWindow*>(window);

			HWND windowHandle = glfwGetWin32Window(windowsWindow->GetNativeWindow());

			if (!windowHandle)
				throw std::runtime_error("Failed to extract WindowsWindow Handler");

			auto handleValue = reinterpret_cast<std::uintptr_t>(windowHandle);

			auto OgreParameterVal = std::to_string(handleValue);

			paras["externalWindowHandle"] = OgreParameterVal;

			auto windowsWindowData = windowsWindow->GetWindowsWindowData();

			m_Window = m_Root->createRenderWindow(
				windowsWindowData->Name,
				windowsWindowData->Width,
				windowsWindowData->Height,
				windowsWindowData->Fullscreen,
				&paras
			);

			break;
		}

		default:

			throw std::runtime_error("Platform Not Supported Yet By Ogre");

			break;
		}

		if (!m_Window)
			throw std::runtime_error("Ogre unable to register back end to Native Window");


	}

	void OgreRenderer::Shutdown()
	{
		
	}

}