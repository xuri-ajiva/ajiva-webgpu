//
// Created by XuriAjiva on 15.08.2023.
//

#pragma once

#include "defines.h"
#include "Core/Layer.h"

#include "imgui.h"
#include "Application.h"

namespace Ajiva::Renderer {
    class ImGuiLayer : public Ajiva::Core::Layer {
    public:
        ImGuiLayer(Ref<Platform::Window> window, Ref<GpuContext> context, Ref<Core::EventSystem> eventSystem,
                   LightningUniform *pUniform);

        void Attached() override;

        void Detached() override;

        void BeforeRender() override;

        void Render(Core::FrameInfo frameInfo) override;

        void AfterRender(wgpu::RenderPassEncoder renderPass) override;

    private:
        Ref<Ajiva::Platform::Window> window;
        Ref<Ajiva::Renderer::GpuContext> context;
        Ref<Ajiva::Core::EventSystem> eventSystem;

        std::vector<Ref<Ajiva::Core::IListener>> events;

        bool OnMouse(AJ_EVENT_PARAMETERS);

        LightningUniform *pUniform;
        bool show_demo_window = true;
        bool show_lightning_window = true;
        bool app_log_open = true;

        void ShowLightningWindow();
    };

}
