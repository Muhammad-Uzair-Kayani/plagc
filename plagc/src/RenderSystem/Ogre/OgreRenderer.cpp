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


//For Testing Purposes
// HLMS Unlit & PBS
#include "OgreHlmsUnlit.h"
#include "OgreHlmsUnlitDatablock.h"
#include "OgreHlmsPbs.h"
#include "OgreHlmsManager.h"
// Resources, Archives & Scene Items
#include "OgreResourceGroupManager.h"
#include "OgreArchiveManager.h"
#include "OgreItem.h"
#include "OgreSceneNode.h"
//To be Removed

namespace plagc
{

	OgreRenderer::OgreRenderer(Window* window)
	{
		const Ogre::AbiCookie cookie = Ogre::generateAbiCookie();

		m_Root = MakeUnique<Ogre::Root>(&cookie, "", "", "ogre.log", "plagc");
		
		auto plugin = new Ogre::VulkanPlugin();
		
		m_Root->installPlugin(plugin, nullptr);

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

		m_OgreFactory = MakeUnique<OgreFatcory>();

		Init(window);

		InitWorkspace();
		
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

			m_OgreFactory->m_RenderWindow = m_Root->createRenderWindow(
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

		if (!m_OgreFactory->m_RenderWindow)
			throw std::runtime_error("Ogre unable to register back end to Native Window");


	}

	void OgreRenderer::InitWorkspace()
	{
		//Creating a default Scene Manager
		m_OgreFactory->m_SceneManager = m_Root->createSceneManager(Ogre::ST_GENERIC, 1u, "plagc.BasicScene");


		//Adding that camera to that scene and setting it's props
		m_OgreFactory->m_Camera = m_OgreFactory->m_SceneManager->createCamera("BasicScene.Camera");
		m_OgreFactory->m_Camera->setPosition(Ogre::Vector3(0.0f, 5.0f, 15.0f));
		m_OgreFactory->m_Camera->lookAt(Ogre::Vector3(0.0f, 0.0f, 0.0f));
		m_OgreFactory->m_Camera->setNearClipDistance(0.2f);
		m_OgreFactory->m_Camera->setFarClipDistance(1000.0f);
		m_OgreFactory->m_Camera->setAutoAspectRatio(true);

		//Creating a basic compositor manager
		m_OgreFactory->m_Compositor = m_Root->getCompositorManager2();

		//Creating Workspace
		//Workspace Needs WokrspaceDef
		//Workspace def needs name which we already have
		//And assigning that workspace to compositor manager
		m_OgreFactory->m_Compositor->createBasicWorkspaceDef(m_OgreFactory->m_WorkspaceName, Ogre::ColourValue(0.15f, 0.25f, 0.40f), Ogre::IdString());
		m_OgreFactory->m_Workspace = m_OgreFactory->m_Compositor->addWorkspace(
			m_OgreFactory->m_SceneManager,
			m_OgreFactory->m_RenderWindow->getTexture(),
			m_OgreFactory->m_Camera,
			m_OgreFactory->m_WorkspaceName,
			true
		);


		//=======================================================================================
		//                   BASIC 3D RENDERING CUBE TEST TO BE REMOVED                        //
		//=======================================================================================
	
		const Ogre::String mediaPath = "C:/Personals/Game Engine Development/Development/plagc/third-party/ogre-next/Samples/Media/";

		auto& resources = Ogre::ResourceGroupManager::getSingleton();
		auto& archives = Ogre::ArchiveManager::getSingleton();

		// Register the sample mesh archive (recursive = true for nested DebugPack folder).
		resources.addResourceLocation(
			mediaPath + "packs/DebugPack",
			"FileSystem",
			"General",
			true
		);

		// --- 1. Register HLMS Unlit ---
		Ogre::String unlitMainFolder;
		Ogre::StringVector unlitLibraryFolders;
		Ogre::HlmsUnlit::getDefaultPaths(unlitMainFolder, unlitLibraryFolders);

		Ogre::Archive* unlitMainArchive = archives.load(
			mediaPath + unlitMainFolder,
			"FileSystem",
			true
		);

		Ogre::ArchiveVec unlitLibraries;
		for (const auto& folder : unlitLibraryFolders)
		{
			unlitLibraries.push_back(
				archives.load(mediaPath + folder, "FileSystem", true)
			);
		}

		auto* unlit = OGRE_NEW Ogre::HlmsUnlit(unlitMainArchive, &unlitLibraries);
		m_Root->getHlmsManager()->registerHlms(unlit);

		// --- 2. Register HLMS PBS (Required so HlmsManager has a fallback default datablock) ---
		Ogre::String pbsMainFolder;
		Ogre::StringVector pbsLibraryFolders;
		Ogre::HlmsPbs::getDefaultPaths(pbsMainFolder, pbsLibraryFolders);

		Ogre::Archive* pbsMainArchive = archives.load(
			mediaPath + pbsMainFolder,
			"FileSystem",
			true
		);

		Ogre::ArchiveVec pbsLibraries;
		for (const auto& folder : pbsLibraryFolders)
		{
			pbsLibraries.push_back(
				archives.load(mediaPath + folder, "FileSystem", true)
			);
		}

		auto* pbs = OGRE_NEW Ogre::HlmsPbs(pbsMainArchive, &pbsLibraries);
		m_Root->getHlmsManager()->registerHlms(pbs);

		// --- 3. Initialize Resource Groups ---
		resources.initialiseAllResourceGroups(false);

		// --- 4. Create Cube Item & Attach to SceneNode ---
		Ogre::Item* cube = m_OgreFactory->m_SceneManager->createItem(
			"Cube_d.mesh",
			Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
			Ogre::SCENE_DYNAMIC
		);

		Ogre::SceneNode* cubeNode = m_OgreFactory->m_SceneManager
			->getRootSceneNode(Ogre::SCENE_DYNAMIC)
			->createChildSceneNode(Ogre::SCENE_DYNAMIC);

		cubeNode->attachObject(cube);
		cubeNode->setPosition(0.0f, 0.0f, 0.0f);

		// --- 5. Create & Assign Yellow Unlit Datablock ---
		Ogre::HlmsDatablock* material = unlit->createDatablock(
			"plagc.CubeMaterial",
			"plagc.CubeMaterial",
			Ogre::HlmsMacroblock(),
			Ogre::HlmsBlendblock(),
			Ogre::HlmsParamVec()
		);

		auto* unlitMaterial = static_cast<Ogre::HlmsUnlitDatablock*>(material);
		unlitMaterial->setUseColour(true);
		unlitMaterial->setColour(Ogre::ColourValue(1.0f, 0.8f, 0.1f));

		cube->setDatablock(material);

	}

	void OgreRenderer::OnUpdate()
	{
		if (!m_Root->renderOneFrame())
			throw std::runtime_error("Failed to Render Ogre Scene");
	}

	OgreRenderer::~OgreRenderer()
	{
		Shutdown();
	}

	void OgreRenderer::Shutdown()
	{
		// 1. Remove compositor workspace.
		if (m_OgreFactory->m_Workspace)
		{
			m_OgreFactory->m_Compositor->removeWorkspace(m_OgreFactory->m_Workspace);
			m_OgreFactory->m_Workspace = nullptr;
		}

		// 2. Destroy camera.
		if (m_OgreFactory->m_Camera)
		{
			m_OgreFactory->m_SceneManager->destroyCamera(m_OgreFactory->m_Camera);
			m_OgreFactory->m_Camera = nullptr;
		}

		// 3. Destroy SceneManager.
		if (m_OgreFactory->m_SceneManager)
		{
			m_Root->destroySceneManager(m_OgreFactory->m_SceneManager);
			m_OgreFactory->m_SceneManager = nullptr;
		}

		// 4. Destroy Ogre render window.
		if (m_OgreFactory->m_RenderWindow)
		{
			m_Root->getRenderSystem()
				->destroyRenderWindow(m_OgreFactory->m_RenderWindow);

			m_OgreFactory->m_RenderWindow = nullptr;
		}

		// 5. Destroy Root and remaining Ogre infrastructure.
		m_OgreFactory->m_Compositor = nullptr;
		m_Root.reset();
	}

}