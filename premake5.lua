-- plagc: root workspace configuration.
-- All third-party dependencies are located under root/third-party.
ROOT_DIR = path.getabsolute(_SCRIPT_DIR)
THIRD_PARTY_DIR = path.join(ROOT_DIR, "third-party")

GLFW_DIR = path.join(THIRD_PARTY_DIR, "glfw")
SPDLOG_DIR = path.join(THIRD_PARTY_DIR, "spdlog")

if not os.isfile(path.join(GLFW_DIR, "include/GLFW/glfw3.h")) then
    error("GLFW submodule not found at " .. GLFW_DIR .. ". Add or initialize the submodule.")
end
if not os.isfile(path.join(SPDLOG_DIR, "include/spdlog/spdlog.h")) then
    error("spdlog submodule not found at " .. SPDLOG_DIR .. ". Add or initialize the submodule.")
end

workspace "plagc"
    location (ROOT_DIR)
    architecture "x86_64"
    system "windows"
    systemversion "latest"
    configurations { "Debug", "Release", "Dist" }
    startproject "Sandbox"

    -- All projects use the same MSVC CRT: /MDd for Debug, /MD for Release/Dist.
    -- Do not put cppdialect here: GLFW is C code, not C++.
    staticruntime "Off"

    filter "system:windows"
	defines {"_WIN32"}

    filter "configurations:Debug"
        defines { "_DEBUG" }
        runtime "Debug"
        symbols "On"
        optimize "Off"

    filter "configurations:Release"
        defines { "NDEBUG" }
        runtime "Release"
        optimize "Speed"         -- MSVC /O2

    filter "configurations:Dist"
        defines { "NDEBUG", "DIST" }
        runtime "Release"
        optimize "Speed"         -- MSVC /O2

    filter {}

-- GLFW is built from source. spdlog is header-only, so it needs no project.
group "Dependencies"
include "Scripts/premake/GLFW.lua"
group ""
include "plagc"
include "Sandbox"
