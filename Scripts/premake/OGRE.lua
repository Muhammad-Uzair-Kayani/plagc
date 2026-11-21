-- Ogre-Next 3.0 source projects. Windows x64 / Vulkan, built by Premake.
-- No CMake Ogre .lib files. Shaderc still comes from the Vulkan SDK.
OGRE_DIR = path.join(THIRD_PARTY_DIR, "ogre-next")
VULKAN_SDK_DIR = os.getenv("VULKAN_SDK")
OGRE_GENERATED_DIR = path.join(ROOT_DIR, "Scripts/premake/generated/ogre")

local function checkFile(file, message)
    if not os.isfile(file) then error(message .. ": " .. file, 0) end
end

local function ogreIncludes()
    return {
        path.join(OGRE_DIR, "OgreMain/include"),
        path.join(OGRE_DIR, "OgreMain/include/Animation"),
        path.join(OGRE_DIR, "OgreMain/include/CommandBuffer"),
        path.join(OGRE_DIR, "OgreMain/include/Compositor"),
        path.join(OGRE_DIR, "OgreMain/include/Compute"),
        path.join(OGRE_DIR, "OgreMain/include/Hash"),
        path.join(OGRE_DIR, "OgreMain/include/Math"),
        path.join(OGRE_DIR, "OgreMain/include/Math/Array"),
        path.join(OGRE_DIR, "OgreMain/include/Math/Array/SSE2/Single"),
        path.join(OGRE_DIR, "OgreMain/include/Math/Simple"),
        path.join(OGRE_DIR, "OgreMain/include/Math/Simple/C"),
        path.join(OGRE_DIR, "OgreMain/include/Threading"),
        path.join(OGRE_DIR, "OgreMain/include/Win32"),
        path.join(OGRE_DIR, "Components/Hlms/Common/include"),
        path.join(OGRE_DIR, "Components/Hlms/Pbs/include"),
        path.join(OGRE_DIR, "Components/Hlms/Unlit/include"),
        path.join(OGRE_DIR, "RenderSystems/Vulkan/include"),
        path.join(OGRE_DIR, "RenderSystems/Vulkan/include/Windowing"),
        path.join(THIRD_PARTY_DIR, "rapidjson/include"),
        path.join(VULKAN_SDK_DIR, "Include")
    }
end

local function loadText(file)
    local f = assert(io.open(file, "rb"))
    local contents = f:read("*a")
    f:close()
    return contents
end

local function saveText(file, contents)
    os.mkdir(path.getdirectory(file))
    local f = assert(io.open(file, "wb"))
    f:write(contents)
    f:close()
end

-- Generate OgreBuildSettings.h for each Premake config from Ogre's own template.
-- Unknown template substitutions are rejected rather than silently misconfigured.
-- For migrating this exact existing 3.0 build, the previously generated CMake
-- header is used as a fallback if the template has options we cannot translate.
local function createOgreSettings(config)
    local outfile = path.join(OGRE_GENERATED_DIR, config .. "/include/OgreBuildSettings.h")
    local template = path.join(OGRE_DIR, "CMake/Templates/OgreBuildSettings.h.in")
    local previous = path.join(ROOT_DIR, "build/ogre-next/include/OgreBuildSettings.h")
    local debugMode = config == "Debug" and "2" or "0"
    local values = {
        OGRE_STATIC = "1", OGRE_STATIC_LIB = "1", OGRE_USE_NEW_PROJECT_NAME = "1",
        OGRE_BUILD_RENDERSYSTEM_VULKAN = "1", OGRE_BUILD_RENDERSYSTEM_GL3PLUS = "0",
        OGRE_BUILD_RENDERSYSTEM_D3D11 = "0", OGRE_BUILD_RENDERSYSTEM_GLES2 = "0",
        OGRE_VULKAN_WINDOW_WIN32 = "1", OGRE_VULKAN_WINDOW_NULL = "0",
        OGRE_BUILD_COMPONENT_HLMS_PBS = "1", OGRE_BUILD_COMPONENT_HLMS_UNLIT = "1",
        OGRE_BUILD_COMPONENT_MESHLODGENERATOR = "0", OGRE_BUILD_COMPONENT_OVERLAY = "0",
        OGRE_BUILD_COMPONENT_ATMOSPHERE = "0", OGRE_BUILD_COMPONENT_PLANAR_REFLECTIONS = "0",
        OGRE_BUILD_COMPONENT_SCENE_FORMAT = "0", OGRE_BUILD_PLUGIN_PFX = "0",
        OGRE_BUILD_PLUGIN_PFX2 = "0", OGRE_BUILD_COMPONENT_TERRAIN = "0",
        OGRE_BUILD_COMPONENT_PROPERTY = "0", OGRE_BUILD_COMPONENT_PAGING = "0",
        OGRE_CONFIG_DOUBLE = "0", OGRE_CONFIG_NODE_INHERIT_TRANSFORM = "0",
        OGRE_CONFIG_THREADS = "0", OGRE_CONFIG_THREAD_PROVIDER = "0",
        OGRE_THREAD_SUPPORT = "0", OGRE_THREAD_PROVIDER = "0",
        OGRE_CONFIG_ENABLE_STBI = "1", OGRE_CONFIG_ENABLE_DDS = "1",
        OGRE_CONFIG_ENABLE_JSON = "1", OGRE_CONFIG_ENABLE_FREEIMAGE = "0",
        OGRE_CONFIG_ENABLE_ZIP = "0", OGRE_CONFIG_ENABLE_ETC = "0",
        OGRE_CONFIG_ENABLE_PVRTC = "0", OGRE_CONFIG_ENABLE_ASTC = "0",
        OGRE_CONFIG_ENABLE_MESHLOD = "0", OGRE_CONFIG_STATIC_LINK_CRT = "0",
        OGRE_CONFIG_CONTAINERS_USE_CUSTOM_ALLOCATOR = "0",
        OGRE_CONFIG_STRING_USE_CUSTOM_ALLOCATOR = "0",
        OGRE_CONFIG_ALLOCATOR = "0", OGRE_CONFIG_MEMTRACK_DEBUG = "0",
        OGRE_CONFIG_MEMTRACK_RELEASE = "0", OGRE_ASSERT_MODE = "0",
        OGRE_PROFILING = "0", OGRE_PROFILING_PROVIDER = "0",
        OGRE_DEBUG_MODE = debugMode, OGRE_DEBUG_LEVEL_DEBUG = "2",
        OGRE_DEBUG_LEVEL_RELEASE = "0", OGRE_DEBUG_LEVEL_DIST = "0",
        OGRE_PLATFORM_X64 = "1", OGRE_TEST_BIG_ENDIAN = "0",
        OGRE_USE_SIMD = "1", OGRE_SIMD_SSE2 = "1",
        OGRE_VERSION_MAJOR = "3", OGRE_VERSION_MINOR = "0", OGRE_VERSION_PATCH = "0",
        OGRE_NEXT = "OgreNext", OGRE_NEXT_PREFIX = "OGRE-Next",
        OGRE_CONFIG_ENABLE_FINE_LIGHT_MASK_GRANULARITY = "0",
        OGRE_CONFIG_ENABLE_LIGHT_OBB_RESTRAINT = "0",
        OGRE_CONFIG_ENABLE_TBB_SCHEDULER = "0",
        OGRE_CONFIG_ENABLE_VIEWPORT_ORIENTATIONMODE = "0",
        OGRE_CONFIG_RENDERDOC_INTEGRATION = "0"
    }
    local generated = nil
    if os.isfile(template) then
        local content = loadText(template)
        content = content:gsub("#cmakedefine01%s+([%w_]+)", function(k)
            return "#define " .. k .. " " .. ((values[k] and values[k] ~= "0") and "1" or "0")
        end)
        content = content:gsub("#cmakedefine%s+([%w_]+)[^\r\n]*", function(k)
            return (values[k] and values[k] ~= "0") and ("#define " .. k .. " " .. values[k]) or ("/* #undef " .. k .. " */")
        end)
        local missing = {}
        content = content:gsub("@([%w_]+)@", function(k)
            if values[k] then return values[k] end
            missing[#missing + 1] = k
            return "@" .. k .. "@"
        end)
        if #missing == 0 then generated = content
        else print("[Ogre] Template has unsupported substitutions: " .. table.concat(missing, ", ")) end
    end
    if not generated and os.isfile(previous) then
        -- This is only a header, never a prebuilt binary. Keeps compatibility
        -- with the configuration previously used to build your existing engine.
        generated = loadText(previous)
        generated = generated:gsub("(#define%s+OGRE_DEBUG_MODE%s+)%d+", "%1" .. debugMode)
        print("[Ogre] Using legacy generated settings header for " .. config .. " (migration fallback)")
    end
    if not generated then
        error("Could not generate OgreBuildSettings.h from the Ogre 3.0 template. " ..
              "Upload CMake/Templates/OgreBuildSettings.h.in so its substitutions can be mapped, " ..
              "or retain build/ogre-next/include/OgreBuildSettings.h once for migration.", 0)
    end
    -- Define this once in the generated header, not again via /D (C4005).
    generated = generated:gsub("/%* #undef OGRE_STATIC_LIB %*/", "#define OGRE_STATIC_LIB 1")
    if not generated:find("#define%s+OGRE_STATIC_LIB") then
        generated = "#define OGRE_STATIC_LIB 1\n" .. generated
    end
    saveText(outfile, generated)
end

-- Vulkan has a SECOND generated build header in addition to OgreBuildSettings.h.
-- Generate it from the vendor template when available; a minimal Win32-only
-- header is sufficient for this pinned Windows/Vulkan-only configuration.
local function createVulkanSettings(config)
    local target = path.join(OGRE_GENERATED_DIR, config .. "/include/OgreVulkanBuildSettings.h")
    local candidates = {
        path.join(OGRE_DIR, "RenderSystems/Vulkan/include/OgreVulkanBuildSettings.h.in"),
        path.join(OGRE_DIR, "RenderSystems/Vulkan/OgreVulkanBuildSettings.h.in"),
        path.join(OGRE_DIR, "RenderSystems/Vulkan/CMake/OgreVulkanBuildSettings.h.in"),
        path.join(OGRE_DIR, "CMake/Templates/OgreVulkanBuildSettings.h.in")
    }
    local template
    for _, candidate in ipairs(candidates) do
        if os.isfile(candidate) then template = candidate break end
    end
    if not template then
        -- Ogre-Next 3.0 uses this feature flag to select its Win32 surface.
        -- Do not define any of the disabled backends even as 0: #ifdef is used.
        saveText(target, "#pragma once\n#define OGRE_VULKAN_WINDOW_WIN32 1\n")
        print("[Ogre] Generated minimal Win32 Vulkan settings for " .. config)
        return
    end
    local values = {
        OGRE_VULKAN_WINDOW_WIN32 = "1", OGRE_VULKAN_WINDOW_NULL = "0",
        OGRE_VULKAN_WINDOW_XCB = "0", OGRE_VULKAN_WINDOW_X11 = "0",
        OGRE_VULKAN_WINDOW_ANDROID = "0", OGRE_VULKAN_USE_SWAPPY = "0"
    }
    local content = loadText(template)
    content = content:gsub("#cmakedefine01%s+([%w_]+)", function(k)
        return "#define " .. k .. " " .. (values[k] or "0")
    end)
    content = content:gsub("#cmakedefine%s+([%w_]+)[^\r\n]*", function(k)
        return values[k] == "1" and ("#define " .. k .. " 1") or ("/* #undef " .. k .. " */")
    end)
    content = content:gsub("@([%w_]+)@", function(k)
        if values[k] == nil then error("Unsupported Vulkan template value: " .. k, 0) end
        return values[k]
    end)
    saveText(target, content)
end

-- os.matchfiles + an explicit platform filter is more dependable than
-- recursive files + removefiles. The previous glob included Android and C
-- math alongside Win32 and SSE2, which explains most of the build log.
local function selectedCpp(directory, reject)
    local sources = {}
    for _, file in ipairs(os.matchfiles(path.join(directory, "**.cpp"))) do
        local relative = path.getrelative(directory, file):gsub("\\", "/"):lower()
        if not reject(relative) then sources[#sources + 1] = file end
    end
    table.sort(sources)
    if #sources == 0 then error("No C++ source files selected from " .. directory, 0) end
    return sources
end

local function rejectMainSource(rel)
    -- Only the Windows x64 (SSE2 / single precision) build is supported.
    for _, folder in ipairs({
        "android/", "emscripten/", "glx/", "ios/", "osx/", "macos/",
        "linux/", "unix/", "gtk/", "x11/", "wayland/", "nacl/", "winrt/",
        "math/array/c/", "math/array/neon/", "math/array/arm/",
        "math/array/sse2/double/", "math/array/avx/", "math/array/avx2/"
    }) do
        if rel:sub(1, #folder) == folder then return true end
    end
    -- The selected work queue/thread provider is Win32/standard, not TBB or pthreads.
    if rel:match("^threading/") and
       (rel:find("pthreads", 1, true) or rel:find("tbb", 1, true)) then return true end
    -- WinRT and MinGW implementations are not the Windows desktop/MSVC implementation.
    if rel:match("^win32/") and
       (rel:find("winrt", 1, true) or rel:find("mingw", 1, true)) then return true end
    -- Optional codec and POSIX utilities are disabled in OgreBuildSettings.h.
    local name = rel:match("([^/]+)$") or rel
    if name:match("^ogrefreeimagecodec") or name == "ogresearchops.cpp" or
       name == "ogrezip.cpp" or name == "ogreconfigdialognoop.cpp" or
       name == "ogrefilesystemlayernoop.cpp" then return true end
    return false
end

local function rejectVulkanSource(rel)
    for _, folder in ipairs({
        "android/", "linux/", "ios/", "osx/", "macos/",
        "windowing/android/", "windowing/linux/", "windowing/xcb/",
        "windowing/x11/", "windowing/null/", "windowing/sdl2/",
        "windowing/ios/", "windowing/macos/", "windowing/wayland/"
    }) do
        if rel:sub(1, #folder) == folder then return true end
    end
    return false
end

function useOgreHeaders()
    externalincludedirs(ogreIncludes())
    filter "configurations:Debug"
        externalincludedirs { path.join(OGRE_GENERATED_DIR, "Debug/include") }
    filter "configurations:Release"
        externalincludedirs { path.join(OGRE_GENERATED_DIR, "Release/include") }
    filter "configurations:Dist"
        externalincludedirs { path.join(OGRE_GENERATED_DIR, "Dist/include") }
    filter {}
end

function linkOgreVulkan()
    -- Ordinary Premake project dependencies. No precompiled Ogre libraries.
    links { "RenderSystem_Vulkan", "OgreNextHlmsPbs", "OgreNextHlmsUnlit", "OgreNextHlmsCommon", "OgreNextMain" }
    dependson { "RenderSystem_Vulkan", "OgreNextHlmsPbs", "OgreNextHlmsUnlit", "OgreNextHlmsCommon", "OgreNextMain" }
    filter "system:windows"
        libdirs { path.join(VULKAN_SDK_DIR, "Lib") }
        links { "shaderc_combined", "vulkan-1" }
    filter {}
end

function copyOgreMediaToSandbox()
    local script = path.join(ROOT_DIR, "Scripts/tools/CopyOgreMedia.ps1")
    local from = path.join(OGRE_DIR, "Samples/Media")
    postbuildcommands {
        'powershell -NoProfile -ExecutionPolicy Bypass -File "' .. script ..
        '" -SourceRoot "' .. from .. '" -TargetRoot "%{cfg.targetdir}/Media"'
    }
end

local function ogreProject(name)
    project(name)
        location (path.join(ROOT_DIR, "Scripts/premake"))
        kind "StaticLib"
        language "C++"
        cppdialect "C++17"
        staticruntime "Off"
        characterset "MBCS" -- OgreDynLib uses the ANSI Win32 APIs, not Unicode TCHARs
        floatingpoint "Fast"
        targetdir (path.join(ROOT_DIR, "bin/%{cfg.buildcfg}/%{prj.name}"))
        objdir (path.join(ROOT_DIR, "bin-int/%{cfg.buildcfg}/%{prj.name}"))
        enablepch "Off"
        useOgreHeaders()
        defines { "_CRT_SECURE_NO_WARNINGS", "NOMINMAX" } -- OGRE_STATIC_LIB is in OgreBuildSettings.h
        filter "action:vs*"
            buildoptions { "/utf-8", "/bigobj", "/Zc:__cplusplus", "/wd4251", "/wd4275", "/wd4996" }
        filter "configurations:Debug"
            symbols "On"
            optimize "Off"
        filter "configurations:Release"
            symbols "On" -- release sources remain debuggable
            optimize "Speed"
        filter "configurations:Dist"
            symbols "Off"
            optimize "Full"
        filter {}
end

-- SPIRV-Reflect is a C implementation, not a C++ source. Our Vulkan
-- *.cpp selection intentionally cannot include it. Compile the vendored
-- spirv_reflect.c alongside the other Vulkan files, rather than expecting
-- shaderc_combined.lib to provide these unrelated API symbols.
local function findSpirvReflectSource()
    local sources = os.matchfiles(path.join(OGRE_DIR, "RenderSystems/Vulkan/**/spirv_reflect.c"))
    table.sort(sources)
    if #sources ~= 1 then
        error("Expected exactly one vendored spirv_reflect.c under " ..
              path.join(OGRE_DIR, "RenderSystems/Vulkan") ..
              "; found " .. tostring(#sources) ..
              ". Ensure Ogre-Next's SPIRV-Reflect sources/submodules are checked out " ..
              "(git submodule update --init --recursive).", 0)
    end
    return sources[1]
end

function createOgreProjects()
    if os.host() ~= "windows" then
        error("This Ogre source recipe targets Windows x64 / Vulkan only; GLFW's Linux support remains unchanged.", 0)
    end
    if not VULKAN_SDK_DIR or VULKAN_SDK_DIR == "" then
        error("Missing VULKAN_SDK environment variable", 0)
    end
    checkFile(path.join(OGRE_DIR, "OgreMain/include/OgrePrerequisites.h"), "Missing Ogre-Next source")
    checkFile(path.join(THIRD_PARTY_DIR, "rapidjson/include/rapidjson/document.h"), "Missing RapidJSON")
    checkFile(path.join(VULKAN_SDK_DIR, "Include/vulkan/vulkan.h"), "Missing Vulkan SDK")
    checkFile(path.join(VULKAN_SDK_DIR, "Include/shaderc/shaderc.h"), "Missing shaderc headers")
    checkFile(path.join(VULKAN_SDK_DIR, "Lib/shaderc_combined.lib"), "Missing shaderc_combined")
    for _, config in ipairs({ "Debug", "Release", "Dist" }) do
        createOgreSettings(config)
        createVulkanSettings(config)
    end

    group "Dependencies/Ogre-Next"
    ogreProject("OgreNextMain")
        files {
            path.join(OGRE_DIR, "OgreMain/include/**.h")
        }
        files(selectedCpp(path.join(OGRE_DIR, "OgreMain/src"), rejectMainSource))

    ogreProject("OgreNextHlmsCommon")
        files {
            path.join(OGRE_DIR, "Components/Hlms/Common/include/**.h"),
            path.join(OGRE_DIR, "Components/Hlms/Common/src/**.cpp")
        }
        links { "OgreNextMain" }

    ogreProject("OgreNextHlmsPbs")
        files {
            path.join(OGRE_DIR, "Components/Hlms/Pbs/include/**.h"),
            path.join(OGRE_DIR, "Components/Hlms/Pbs/src/**.cpp")
        }
        links { "OgreNextHlmsCommon", "OgreNextMain" }

    ogreProject("OgreNextHlmsUnlit")
        files {
            path.join(OGRE_DIR, "Components/Hlms/Unlit/include/**.h"),
            path.join(OGRE_DIR, "Components/Hlms/Unlit/src/**.cpp")
        }
        links { "OgreNextHlmsCommon", "OgreNextMain" }

    ogreProject("RenderSystem_Vulkan")
        local spirvReflectSource = findSpirvReflectSource()
        files {
            path.join(OGRE_DIR, "RenderSystems/Vulkan/include/**.h"),
            spirvReflectSource
        }
        files(selectedCpp(path.join(OGRE_DIR, "RenderSystems/Vulkan/src"), rejectVulkanSource))
        -- Needed by spirv_reflect.c for its matching header and bundled SPIR-V headers.
        includedirs {
            path.getdirectory(spirvReflectSource),
            path.join(OGRE_DIR, "RenderSystems/Vulkan/include/SPIRV-Reflect"),
            path.join(OGRE_DIR, "RenderSystems/Vulkan/include/SPIRV-Reflect/include")
        }
        links { "OgreNextMain" }
    group ""
end
