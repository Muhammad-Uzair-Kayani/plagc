-- Engine: static C++ library with its own PCH and access to GLFW/spdlog.
project "plagc"
    location (path.join(ROOT_DIR, "plagc"))
    kind "StaticLib"
    language "C++"
    cppdialect "C++latest"
    buildoptions { "/utf-8" }

    targetdir (path.join(ROOT_DIR, "bin/%{cfg.buildcfg}/%{prj.name}"))
    objdir (path.join(ROOT_DIR, "bin-int/%{cfg.buildcfg}/%{prj.name}"))

    files {
        path.join(ROOT_DIR, "plagc/src/**.h"),
        path.join(ROOT_DIR, "plagc/src/**.hpp"),
        path.join(ROOT_DIR, "plagc/src/**.hxx"),
        path.join(ROOT_DIR, "plagc/src/**.inl"),
        path.join(ROOT_DIR, "plagc/src/**.c"),
        path.join(ROOT_DIR, "plagc/src/**.cc"),
        path.join(ROOT_DIR, "plagc/src/**.cpp"),
        path.join(ROOT_DIR, "plagc/src/**.cxx"),
        path.join(ROOT_DIR, "plagc/src/**.ixx"),
        path.join(ROOT_DIR, "plagc/header/**.h"),
        path.join(ROOT_DIR, "plagc/header/**.hpp"),
        path.join(ROOT_DIR, "plagc/header/**.hxx"),
        path.join(ROOT_DIR, "plagc/header/**.inl"),
        path.join(ROOT_DIR, "plagc/pch/pch.h"),
        path.join(ROOT_DIR, "plagc/pch/pch.cpp")
    }

    includedirs {
        path.join(ROOT_DIR, "plagc/src"),
        path.join(ROOT_DIR, "plagc/header"),
        path.join(ROOT_DIR, "plagc/pch"),
        path.join(GLFW_DIR, "include"),
        path.join(SPDLOG_DIR, "include")
    }

    -- spdlog is header-only by default (uses bundled fmt).
    -- This explicitly selects the header-only mode for engine translation units.
    defines { "SPDLOG_HEADER_ONLY", "GLFW_INCLUDE_NONE", "SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE" }

    enablepch "On"
    pchheader "pch.h"
    pchsource (path.join(ROOT_DIR, "plagc/pch/pch.cpp"))

    -- Project dependency on GLFW; final static-link happens in Sandbox.
    links { "GLFW" }

    filter "files:**.c"
        enablepch "Off"

    filter "files:**.ixx"
        enablepch "Off"

    -- Entry point is compiled into the engine without forcing the PCH.
    filter "files:**/Entrypoint.cpp"
        enablepch "Off"

    filter {}
