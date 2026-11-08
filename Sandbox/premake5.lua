-- Client executable; creates plagc::Application and links the engine.
project "Sandbox"
    location (path.join(ROOT_DIR, "Sandbox"))
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++latest"

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
        path.join(ROOT_DIR, "plagc/header"),
        path.join(ROOT_DIR, "plagc/src"),  -- temporary: public header dependencies
        path.join(GLFW_DIR, "include"),
        path.join(SPDLOG_DIR, "include")
    }

    defines {
        "GLFW_INCLUDE_NONE",
        "SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE"
    }
    enablepch "Off"

    links { "plagc", "GLFW" }

    filter "action:vs*"
        buildoptions { "/utf-8" }

    filter "system:windows"
        links { "user32", "gdi32", "shell32", "imm32" }

    -- Windows GUI subsystem only for Dist. Do NOT force mainCRTStartup.
    filter { "system:windows", "configurations:Dist" }
        kind "WindowedApp"

    -- Linux X11 dependencies for GLFW built as a static library.
    filter "system:linux"
        links { "X11", "Xrandr", "Xinerama", "Xcursor", "Xi", "pthread", "dl", "m" }

    filter {}
