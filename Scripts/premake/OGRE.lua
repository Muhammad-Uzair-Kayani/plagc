
-- Ogre-Next 3.0.0: Windows x64, Vulkan, static Release.

OGRE_DIR = path.join(THIRD_PARTY_DIR, "ogre-next")
OGRE_BUILD_DIR = path.join(ROOT_DIR, "build/ogre-next")
OGRE_LIB_DIR = path.join(OGRE_BUILD_DIR, "lib/Release")

VULKAN_SDK_DIR = os.getenv("VULKAN_SDK")

if not VULKAN_SDK_DIR then
    error("VULKAN_SDK environment variable is missing")
end

if not os.isfile(path.join(
    OGRE_LIB_DIR, "RenderSystem_VulkanStatic.lib")) then
    error("Build Ogre-Next Vulkan Release first")
end

function useOgreHeaders()
    externalincludedirs {
        path.join(OGRE_DIR, "OgreMain/include"),
        path.join(OGRE_DIR, "Components/Hlms/Common/include"),
        path.join(OGRE_DIR, "Components/Hlms/Pbs/include"),
        path.join(OGRE_DIR, "Components/Hlms/Unlit/include"),
        path.join(OGRE_DIR, "RenderSystems/Vulkan/include"),
        path.join(OGRE_BUILD_DIR, "include"),
        path.join(VULKAN_SDK_DIR, "Include")
    }
end

function linkOgreVulkan()
    filter "system:windows"
        libdirs {
            OGRE_LIB_DIR,
            path.join(VULKAN_SDK_DIR, "Lib")
        }

    filter { "system:windows", "configurations:Release" }
        links {
            "RenderSystem_VulkanStatic",
            "OgreNextHlmsPbsStatic",
            "OgreNextHlmsUnlitStatic",
            "OgreNextMainStatic",
            "shaderc_combined",
            "vulkan-1"
        }

    filter { "system:windows", "configurations:Dist" }
        links {
            "RenderSystem_VulkanStatic",
            "OgreNextHlmsPbsStatic",
            "OgreNextHlmsUnlitStatic",
            "OgreNextMainStatic",
            "shaderc_combined",
            "vulkan-1"
        }

    filter {}
end
