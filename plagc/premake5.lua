-- plagc/plagc/premake5.lua
-- The engine compiles to plagc.lib; PCH is enabled ONLY for this project.

project "plagc"
    location (path.join(ROOT_DIR, "plagc"))
    kind "StaticLib"
    language "C++"

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

        -- Public engine API, including plagc/header/plagc.h.
        path.join(ROOT_DIR, "plagc/header/**.h"),
        path.join(ROOT_DIR, "plagc/header/**.hpp"),
        path.join(ROOT_DIR, "plagc/header/**.hxx"),
        path.join(ROOT_DIR, "plagc/header/**.inl"),

        -- PCH files live next to src, not inside it.
        path.join(ROOT_DIR, "plagc/pch/pch.h"),
        path.join(ROOT_DIR, "plagc/pch/pch.cpp")
    }

    includedirs {
        path.join(ROOT_DIR, "plagc/src"),
        path.join(ROOT_DIR, "plagc/header"),
        path.join(ROOT_DIR, "plagc/pch")
    }

    enablepch "On"
    pchheader "pch.h"  -- Exact include used by each engine .cpp: #include "pch.h"
    pchsource (path.join(ROOT_DIR, "plagc/pch/pch.cpp"))

    -- C and C++ module-interface sources must not use the C++ PCH.
    filter "files:**.c"
        enablepch "Off"

    filter "files:**.ixx"
        enablepch "Off"

    filter "files:**/Entrypoint.cpp"
        enablepch "Off"

    filter {}
