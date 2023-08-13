//
// Created by XuriAjiva on 31.07.2023.
//


#pragma once

#include "webgpu/webgpu.hpp"
#include "GLFW/glfw3.h"
#include <functional>
#include <thread>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_wgpu.h"

#include "defines.h"
#include "glfw3webgpu.h"
#include "plog/Log.h"
#include "Core/EventSystem.h"


namespace Ajiva::Platform {

    struct WindowConfig {
        i32 X = 200;
        i32 Y = 100;
        u32 Width = 800;
        u32 Height = 600;
        bool DedicatedThread;
        std::string Name;
    };

    class Window {
    public:
        static bool Init();

        static void Shutdown();

        Window() = default;

        explicit Window(const WindowConfig &config, Ref<Core::EventSystem> eventSystem);

        ~Window();

        [[nodiscard]] std::function<wgpu::Surface(wgpu::Instance)> CreateSurfaceFunk();

        [[nodiscard]] inline GLFWwindow *GetWindow() const {
            return window;
        }

        [[nodiscard]] inline int GetWidth() const {
            return config.Width;
        }

        [[nodiscard]] inline int GetHeight() const {
            return config.Height;
        }

        [[nodiscard]] bool IsClosed();

        inline void RequestClose() {
            running = false;
        }

        static void glfw_error_callback(int error, const char *description) {
            PLOG_ERROR << "GLFW Error: " << error << " " << description;
        }

        void Create();

        void Run();

    private:
        WindowConfig config;
        bool running = false;
        GLFWwindow *window = nullptr;
        bool m_cloesed = false;
        Ref<Core::EventSystem> eventSystem;

        bool CreateWindow();

        inline static bool IsGLFWInitialized;
    };
}