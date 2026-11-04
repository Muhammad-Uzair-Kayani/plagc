-- Premake recipe for the GLFW Win32 static library.
-- This file is outside the Git submodule: no modifications to upstream sources.
-- Mirrors the Windows source set from upstream GLFW's src/CMakeLists.txt.
project "GLFW"
    location (path.join(ROOT_DIR, "Scripts/premake"))
    kind "StaticLib"
    language "C"
    cdialect "C11"               -- MSVC supports /std:c11; GLFW needs C99 features
    targetname "glfw3"

    targetdir (path.join(ROOT_DIR, "bin/%{cfg.buildcfg}/%{prj.name}"))
    objdir (path.join(ROOT_DIR, "bin-int/%{cfg.buildcfg}/%{prj.name}"))

    files {
        -- Common GLFW implementation + optional context backends and null backend.
        path.join(GLFW_DIR, "src/context.c"),
        path.join(GLFW_DIR, "src/init.c"),
        path.join(GLFW_DIR, "src/input.c"),
        path.join(GLFW_DIR, "src/monitor.c"),
        path.join(GLFW_DIR, "src/platform.c"),
        path.join(GLFW_DIR, "src/vulkan.c"),
        path.join(GLFW_DIR, "src/window.c"),
        path.join(GLFW_DIR, "src/egl_context.c"),
        path.join(GLFW_DIR, "src/osmesa_context.c"),
        path.join(GLFW_DIR, "src/null_init.c"),
        path.join(GLFW_DIR, "src/null_monitor.c"),
        path.join(GLFW_DIR, "src/null_window.c"),
        path.join(GLFW_DIR, "src/null_joystick.c"),

        -- Windows shared services and Win32/WGL backend.
        path.join(GLFW_DIR, "src/win32_module.c"),
        path.join(GLFW_DIR, "src/win32_time.c"),
        path.join(GLFW_DIR, "src/win32_thread.c"),
        path.join(GLFW_DIR, "src/win32_init.c"),
        path.join(GLFW_DIR, "src/win32_joystick.c"),
        path.join(GLFW_DIR, "src/win32_monitor.c"),
        path.join(GLFW_DIR, "src/win32_window.c"),
        path.join(GLFW_DIR, "src/wgl_context.c"),

        -- Show headers in Visual Studio without compiling unrelated platform sources.
        path.join(GLFW_DIR, "src/**.h"),
        path.join(GLFW_DIR, "include/GLFW/**.h")
    }

    includedirs {
        path.join(GLFW_DIR, "include"),
        path.join(GLFW_DIR, "src")
    }

    -- Private to compiling GLFW! Never define _GLFW_WIN32 or GLFW_DLL in clients.
    defines { "_GLFW_WIN32", "UNICODE", "_UNICODE", "_CRT_SECURE_NO_WARNINGS" }
    enablepch "Off"
