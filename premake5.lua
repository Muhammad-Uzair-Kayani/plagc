-- plagc workspace. Dependencies are built with Premake (no vendor CMake).
ROOT_DIR = path.getabsolute(_SCRIPT_DIR)
THIRD_PARTY_DIR = path.join(ROOT_DIR, "third-party")

GLFW_DIR   = path.join(THIRD_PARTY_DIR, "glfw")
SPDLOG_DIR = path.join(THIRD_PARTY_DIR, "spdlog")

if not os.isfile(path.join(GLFW_DIR, "include/GLFW/glfw3.h")) then
    error("Missing GLFW submodule. Run: git submodule update --init --recursive")
end
if not os.isfile(path.join(SPDLOG_DIR, "include/spdlog/spdlog.h")) then
    error("Missing spdlog submodule. Run: git submodule update --init --recursive")
end

workspace "plagc"
    location (ROOT_DIR)
    architecture "x86_64"
    configurations { "Debug", "Release", "Dist" }
    startproject "Sandbox"
    staticruntime "Off"

    -- Do not set system = windows globally. Native VS targets Windows;
    -- native gmake2 on Linux targets Linux.
    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        defines { "_DEBUG" }
        runtime "Debug"
        symbols "On"
        optimize "Off"

    filter "configurations:Release"
        defines { "NDEBUG" }
        runtime "Release"
        optimize "Speed"

    filter "configurations:Dist"
        defines { "NDEBUG", "DIST" }
        runtime "Release"
        optimize "Speed"

    filter {}

group "Dependencies"
include "Scripts/premake/GLFW.lua"
group ""
include "plagc"
include "Sandbox"
