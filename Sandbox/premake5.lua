-- plagc/Sandbox/premake5.lua
-- Sandbox links to plagc.lib but does NOT build/use the engine PCH.

project "Sandbox"
    location (path.join(ROOT_DIR, "Sandbox"))
    kind "ConsoleApp"
    language "C++"

    targetdir (path.join(ROOT_DIR, "bin/%{cfg.buildcfg}/%{prj.name}"))
    objdir (path.join(ROOT_DIR, "bin-int/%{cfg.buildcfg}/%{prj.name}"))
    debugdir (ROOT_DIR)

    files {
        path.join(ROOT_DIR, "Sandbox/src/**.h"),
        path.join(ROOT_DIR, "Sandbox/src/**.hpp"),
        path.join(ROOT_DIR, "Sandbox/src/**.hxx"),
        path.join(ROOT_DIR, "Sandbox/src/**.inl"),
        path.join(ROOT_DIR, "Sandbox/src/**.c"),
        path.join(ROOT_DIR, "Sandbox/src/**.cc"),
        path.join(ROOT_DIR, "Sandbox/src/**.cpp"),
        path.join(ROOT_DIR, "Sandbox/src/**.cxx"),
        path.join(ROOT_DIR, "Sandbox/src/**.ixx")
    }

    includedirs {
        path.join(ROOT_DIR, "Sandbox/src"),
        path.join(ROOT_DIR, "plagc/header"), -- permits #include <plagc.h>
        path.join(ROOT_DIR, "plagc/src")     -- supports headers included by plagc.h
    }

    enablepch "Off"

    -- Links the engine library for the matching build config and
    -- automatically makes Sandbox depend on the plagc project.
    links { "plagc" }

    filter "configurations:Dist"
        kind "WindowedApp"

    -- In Dist, suppress the console window while keeping int main().
    filter { "system:windows", "configurations:Dist" }
        entrypoint "mainCRTStartup"

    filter {}
