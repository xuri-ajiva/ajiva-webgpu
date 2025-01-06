//
// Created by XuriAjiva on 15.08.2023.
//

#pragma once

#include "defines.h"
#include "Core/Layer.h"

#include "imgui.h"
//#include "Application.h"
#include "RenderPipelineLayer.h"
#include "Platform/Window.h"
#include "Camera.h"

namespace Ajiva::Renderer
{
    class ImGuiLayer : public Ajiva::Core::Layer
    {
    public:
        ImGuiLayer(Ref<Platform::Window> window, Ref<GpuContext> context, Ref<Core::EventSystem> eventSystem,
                   Ref<Renderer::RenderPipelineLayer> pipeline, Ref<Renderer::FreeCamera> camara);

        bool Attached() override;

        void Detached() override;

        void BeforeRender(Core::UpdateInfo frameInfo, Core::RenderTarget target) override;

        void AfterRender(Core::UpdateInfo frameInfo, Core::RenderTarget target) override;

        void Render(Core::UpdateInfo updateInfo, Core::RenderTarget target) override;

    private:
        Ref<Ajiva::Platform::Window> window;
        Ref<Ajiva::Renderer::GpuContext> context;
        Ref<Ajiva::Core::EventSystem> eventSystem;

        std::vector<Ref<Ajiva::Core::IListener>> events;

        bool OnMouse(AJ_EVENT_PARAMETERS);
        bool OnKey(AJ_EVENT_PARAMETERS);

        Ref<Renderer::FreeCamera> camara;
        Ref<Renderer::RenderPipelineLayer> pipeline;
        bool show_demo_window = true;
        bool show_lightning_window = true;
        bool show_camera_window = true;
        bool app_log_open = true;
        bool show_overlay = true;

        void ShowLightningWindow();
        void ShowCameraWindow();

        void RenderIntern(Core::RenderTarget target);

        void ShowOverlay();
    };
}
