-- Executable/client project. Links engine and GLFW, but has no PCH of its own.
project "Sandbox"
    location (path.join(ROOT_DIR, "Sandbox"))
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++latest"
    buildoptions { "/utf-8" }

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
        path.join(ROOT_DIR, "plagc/header"),    -- #include <plagc.h>
        path.join(ROOT_DIR, "plagc/src"),       -- transitive public headers for now
        path.join(GLFW_DIR, "include"),        -- optional direct GLFW experiments
        path.join(SPDLOG_DIR, "include")       -- optional direct spdlog usage
    }

    defines {"GLFW_INCLUDE_NONE", "SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE" }
    enablepch "Off"

    -- Explicit final link: engine + GLFW static archive + Win32 system libraries.
    -- Static libraries do not absorb the other .lib's object code.
    links { "plagc", "GLFW", "user32", "gdi32", "shell32", "imm32" }

    filter "configurations:Dist"
        kind "WindowedApp"

    -- Keep engine-owned int main() without a console window in Dist.
    filter { "system:windows", "configurations:Dist" }
        entrypoint "mainCRTStartup"

    filter {}
