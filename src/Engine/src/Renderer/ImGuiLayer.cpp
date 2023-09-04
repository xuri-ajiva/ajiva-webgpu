//
// Created by XuriAjiva on 15.08.2023.
//

#include "ImGuiLayer.h"

#include <utility>

#include "imgui_impl_glfw.h"
#include "imgui_impl_wgpu.h"
#include "RenderPipelineLayer.h"

namespace Ajiva::Renderer {
    ImGuiLayer::ImGuiLayer(Ref<Platform::Window> window, Ref<GpuContext> context, Ref<Core::EventSystem> eventSystem,
                           Ref<Renderer::RenderPipelineLayer> pipeline, Ref<Renderer::FreeCamera> camara)
            : Layer("ImGuiLayer"), window(std::move(window)), context(std::move(context)),
              eventSystem(std::move(eventSystem)), pipeline(std::move(pipeline)), camara(std::move(camara)) {
        //catch events as soon as possible
        this->events.push_back(this->eventSystem->Add(Core::MouseButtonDown, this, &ImGuiLayer::OnMouse));
        this->events.push_back(this->eventSystem->Add(Core::MouseButtonUp, this, &ImGuiLayer::OnMouse));
        this->events.push_back(this->eventSystem->Add(Core::MouseScroll, this, &ImGuiLayer::OnMouse));
        this->events.push_back(this->eventSystem->Add(Core::MouseMove, this, &ImGuiLayer::OnMouse));
        this->events.push_back(this->eventSystem->Add(Core::KeyDown, this, &ImGuiLayer::OnKey));
        this->events.push_back(this->eventSystem->Add(Core::KeyUp, this, &ImGuiLayer::OnKey));
    }

    static inline void ImGuiColorStyle();

    bool ImGuiLayer::Attached() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        [[maybe_unused]] ImGuiIO &io = ImGui::GetIO();
        //ImGui::StyleColorsDark();
        ImGuiColorStyle();
        //ImGui::StyleColorsLight();
        // Setup Platform/Renderer backends
        if (!ImGui_ImplGlfw_InitForOther(window->GetWindow(), true)) {
            PLOG_ERROR << "ImGui_ImplGlfw_InitForOther failed";
            return false;
        }
        if (!ImGui_ImplWGPU_Init(*context->device, 3, context->swapChainFormat, WGPUTextureFormat_Undefined)) {
            PLOG_ERROR << "ImGui_ImplWGPU_Init failed";
            return false;
        }
        return true;
    }

    void ImGuiLayer::Detached() {
        //todo: static init / shutdown?
        ImGui_ImplGlfw_Shutdown();
        ImGui_ImplWGPU_Shutdown();
    }

    void ImGuiLayer::Render(Core::UpdateInfo updateInfo, Core::RenderTarget target) {
        Layer::Render(updateInfo, target);

        //TODO io.DisplaySize = ImVec2((float) window.GetWidth(), (float) window.GetHeight());
        ImGui_ImplWGPU_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);
        if (app_log_open)
            Core::ShowAppLog(&app_log_open);

        if (show_lightning_window)
            ShowLightningWindow();

        if (show_camera_window)
            ShowCameraWindow();

        ImGui::Render();

        RenderIntern(target);
    }

    void ImGuiLayer::ShowCameraWindow() {
        ImGui::Begin("Camera", &show_camera_window);
        ImGui::SeparatorText("Translation");
        ImGui::DragFloat3("Position", (float *) &camara->position, .05f, -10.0f, 10.0f);
        ImGui::InputFloat3("Acceleration", (float *) &camara->acceleration, "%.3f", ImGuiInputTextFlags_ReadOnly);
        ImGui::InputFloat3("Front", (float *) &camara->front, "%.3f", ImGuiInputTextFlags_ReadOnly);
        ImGui::InputFloat3("Up", (float *) &camara->up, "%.3f", ImGuiInputTextFlags_ReadOnly);
        ImGui::SeparatorText("Rotation");
        ImGui::InputFloat2("Angles", (float *) &camara->angles, "%.3f", ImGuiInputTextFlags_ReadOnly);
        ImGui::SeparatorText("Mouse");
        ImGui::DragFloat("Mouse Sensitivity", &camara->mouseSensitivity, .01f, 0.0f, 10.0f);
        ImGui::DragFloat("Speed", &camara->speed, .005f, 0.0f, 5.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
        ImGui::SeparatorText("Keys");
        ImGui::Checkbox("Forward", &camara->forward_down);
        ImGui::SameLine();
        ImGui::Checkbox("Right", &camara->right_down);
        ImGui::SameLine();
        ImGui::Checkbox("Up", &camara->up_down);
        ImGui::Checkbox("Backward", &camara->backward_down);
        ImGui::SameLine();
        ImGui::Checkbox("Left", &camara->left_down);
        ImGui::SameLine();
        ImGui::Checkbox("Down", &camara->down_down);
        ImGui::End();
    }

    void ImGuiLayer::ShowLightningWindow() {
        auto pUniform = &pipeline->lightningUniform;

        ImGui::Begin("Lightning", &show_lightning_window);
        static bool alpha_preview = true;
        static bool alpha_half_preview = false;
        static bool drag_and_drop = true;
        static bool options_menu = true;
        ImGui::SeparatorText("Options");
        ImGui::Checkbox("With Alpha Preview", &alpha_preview);
        ImGui::Checkbox("With Half Alpha Preview", &alpha_half_preview);
        ImGui::Checkbox("With Drag and Drop", &drag_and_drop);
        ImGui::Checkbox("With Options Menu", &options_menu);
        ImGuiColorEditFlags misc_flags = (drag_and_drop ? 0 : ImGuiColorEditFlags_NoDragDrop) |
                                         (alpha_half_preview ? ImGuiColorEditFlags_AlphaPreviewHalf
                                                             : (alpha_preview ? ImGuiColorEditFlags_AlphaPreview : 0)) |
                                         (options_menu ? 0 : ImGuiColorEditFlags_NoOptions);
        ImGui::SeparatorText("Inline color editor");
        ImGui::ColorEdit4("Ambient", (float *) &pUniform->ambient, misc_flags);
        ImGui::DragFloat("Hardness", &pUniform->hardness, 0.05, 0.0001f, 128.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
        ImGui::SliderFloat("Kd", &pUniform->kd, 0.0f, 2.0f);
        ImGui::SliderFloat("Ks", &pUniform->ks, 0.0f, 2.0f);
        for (int i = 0; i < glm::countof(pUniform->lights); ++i) {
            ImGui::SeparatorText(("Light " + std::to_string(i)).c_str());
            ImGui::ColorEdit3(("Color##C" + std::to_string(i)).c_str(),
                              (float *) &pUniform->lights[i].color, misc_flags);
            ImGui::DragFloat(("Intensity##C" + std::to_string(i)).c_str(),
                             (float *) &pUniform->lights[i].color.w, .05f, 0.0f, 10.0f);
            ImGui::DragFloat3(("Position##C" + std::to_string(i)).c_str(),
                              (float *) &pUniform->lights[i].position, .05f, -10.0f, 10.0f);
        }

        ImGui::End();
    }


    bool ImGuiLayer::OnMouse(AJ_EVENT_PARAMETERS) {
        ImGuiIO &io = ImGui::GetIO();
        if (io.WantCaptureMouse) {
            return true; //stop propagation
        }
        return false;
    }
    bool ImGuiLayer::OnKey(AJ_EVENT_PARAMETERS) {
        ImGuiIO &io = ImGui::GetIO();
        if (io.WantCaptureKeyboard) {
            return true; //stop propagation
        }
        return false;
    }

    void ImGuiLayer::RenderIntern(Core::RenderTarget target) {
        wgpu::CommandEncoder encoder = context->CreateCommandEncoder();

        wgpu::RenderPassDescriptor renderPassDesc{};
        wgpu::RenderPassColorAttachment renderPassColorAttachment;
        renderPassColorAttachment.view = target.texture;
        renderPassColorAttachment.resolveTarget = nullptr;
        renderPassColorAttachment.loadOp = wgpu::LoadOp::Load;
        renderPassColorAttachment.storeOp = wgpu::StoreOp::Store;
        renderPassColorAttachment.clearValue = {0.4, 0.4, 0.4, 1.0};
        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &renderPassColorAttachment;

        renderPassDesc.timestampWriteCount = 0;
        renderPassDesc.timestampWrites = nullptr;
        auto renderPass = encoder.beginRenderPass(renderPassDesc);
        ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPass);
        renderPass.end();
        context->SubmitEncoder(encoder);
    }


    static inline void ImGuiColorStyle() {
        ImGuiStyle &style = ImGui::GetStyle();
        style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
        style.Colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.08f, 0.50f, 0.72f, 1.00f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
        style.Colors[ImGuiCol_Separator] = style.Colors[ImGuiCol_Border];
        style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.41f, 0.42f, 0.44f, 1.00f);
        style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.29f, 0.30f, 0.31f, 0.67f);
        style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        style.Colors[ImGuiCol_Tab] = ImVec4(0.08f, 0.08f, 0.09f, 0.83f);
        style.Colors[ImGuiCol_TabHovered] = ImVec4(0.33f, 0.34f, 0.36f, 0.83f);
        style.Colors[ImGuiCol_TabActive] = ImVec4(0.23f, 0.23f, 0.24f, 1.00f);
        style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
        style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
        //style.Colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.59f, 0.98f, 0.70f);
        //style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
        style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
        style.GrabRounding = style.FrameRounding = 2.3f;
    }

}