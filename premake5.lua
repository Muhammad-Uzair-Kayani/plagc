-- plagc/premake5.lua
-- Shared workspace settings. Project-specific settings live in each subfolder.

ROOT_DIR = path.getabsolute(_SCRIPT_DIR)

workspace "plagc"
    location (ROOT_DIR)
    architecture "x86_64"
    system "windows"
    systemversion "latest"
    configurations { "Debug", "Release", "Dist" }
    startproject "Sandbox"

    -- Applies to both projects: Visual Studio uses /std:c++latest.
    cppdialect "C++latest"

    -- Consistent MSVC CRT across the static library and executable.
    -- /MDd in Debug, /MD in Release/Dist; this does not prevent plagc.lib
    -- from being statically linked into Sandbox.
    staticruntime "Off"

    filter "configurations:Debug"
        defines { "_DEBUG" }
        runtime "Debug"
        symbols "On"
        optimize "Off"             -- MSVC /Od

    filter "configurations:Release"
        defines { "NDEBUG" }
        runtime "Release"
        optimize "Speed"           -- MSVC /O2

    filter "configurations:Dist"
        defines { "NDEBUG" }
        runtime "Release"
        optimize "Speed"           -- MSVC /O2

    filter {}

-- Both project scripts are included from the root workspace script.
include "plagc"
include "Sandbox"
