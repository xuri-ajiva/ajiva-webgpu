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


namespace Ajiva::Platform {

    struct WindowConfig {
        i16 X = 200;
        i16 Y = 100;
        i16 Width = 800;
        i16 Height = 600;
        bool DedicatedThread;
        std::string Name;
        std::function<void(u16, u16)> ResizeCallback;
    };

    class Window {
    public:
        static bool Init();
        static void Shutdown();

        explicit Window(const WindowConfig &config);

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
        GLFWwindow *window{};
        bool m_cloesed = false;

        bool CreateWindow();

        inline static bool IsGLFWInitialized;
    };
}