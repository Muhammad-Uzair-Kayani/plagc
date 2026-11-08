-- Build the GLFW submodule directly with Premake.
-- This recipe targets Win32 on Windows and X11 on Linux.
-- Linux Wayland support can be added later as a separate backend.
project "GLFW"
    location (path.join(ROOT_DIR, "Scripts/premake"))
    kind "StaticLib"
    language "C"
    cdialect "C11"
    targetname "glfw3"

    targetdir (path.join(ROOT_DIR, "bin/%{cfg.buildcfg}/%{prj.name}"))
    objdir (path.join(ROOT_DIR, "bin-int/%{cfg.buildcfg}/%{prj.name}"))

    files {
        -- Shared GLFW implementation and null backend.
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
        path.join(GLFW_DIR, "src/**.h"),
        path.join(GLFW_DIR, "include/GLFW/**.h")
    }

    includedirs {
        path.join(GLFW_DIR, "include"),
        path.join(GLFW_DIR, "src")
    }
    enablepch "Off"

    filter "system:windows"
        files {
            path.join(GLFW_DIR, "src/win32_module.c"),
            path.join(GLFW_DIR, "src/win32_time.c"),
            path.join(GLFW_DIR, "src/win32_thread.c"),
            path.join(GLFW_DIR, "src/win32_init.c"),
            path.join(GLFW_DIR, "src/win32_joystick.c"),
            path.join(GLFW_DIR, "src/win32_monitor.c"),
            path.join(GLFW_DIR, "src/win32_window.c"),
            path.join(GLFW_DIR, "src/wgl_context.c")
        }
        defines { "_GLFW_WIN32", "UNICODE", "_UNICODE", "_CRT_SECURE_NO_WARNINGS" }

    filter "system:linux"
        files {
            path.join(GLFW_DIR, "src/posix_module.c"),
            path.join(GLFW_DIR, "src/posix_time.c"),
            path.join(GLFW_DIR, "src/posix_thread.c"),
            path.join(GLFW_DIR, "src/posix_poll.c"),
            path.join(GLFW_DIR, "src/linux_joystick.c"),
            path.join(GLFW_DIR, "src/x11_init.c"),
            path.join(GLFW_DIR, "src/x11_monitor.c"),
            path.join(GLFW_DIR, "src/x11_window.c"),
            path.join(GLFW_DIR, "src/xkb_unicode.c"),
            path.join(GLFW_DIR, "src/glx_context.c")
        }
        defines { "_GLFW_X11" }
        buildoptions { "-pthread" }

    filter {}
