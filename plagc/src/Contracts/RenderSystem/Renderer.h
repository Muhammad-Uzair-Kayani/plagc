#pragma once
#include "Core/Core.h"
#include "Contracts/Platforms/Window.h"

namespace plagc
{
    class Camera;
    class SceneManager;

    class Renderer
    {
    public:
        virtual ~Renderer() = default;

        virtual void SetScene(SceneManager& scene, Camera& camera) = 0;
        virtual void ClearScene() = 0;
        virtual void OnWindowResize() = 0;
        virtual void OnUpdate() = 0;

    protected:
        Renderer() = default;
    };

    class RendererBuilder
    {
    public:
        RendererBuilder& WindowHandle(Window* window) { r_Window = window; return *this; }
        Owner<Renderer> Build();

    private:
        Owner<Renderer> BuildRenderer();
        Window* r_Window = nullptr;
    };
}
