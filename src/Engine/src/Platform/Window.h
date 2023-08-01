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

namespace Ajiva {
    class Window {

    public:
        explicit Window(int width = 800, int height = 600, bool dedicatedThread = true);

        ~Window();

        [[nodiscard]] std::function<wgpu::Surface(wgpu::Instance)> CreateSurfaceFunk();

        [[nodiscard]] inline GLFWwindow *GetWindow() const {
            return window;
        }

        [[nodiscard]] inline int GetWidth() const {
            return m_width;
        }

        [[nodiscard]] inline int GetHeight() const {
            return m_height;
        }

        [[nodiscard]] bool IsClosed();

        inline void RequestClose() {
            running = false;
        }

        static void glfw_error_callback(int error, const char *description) {
            std::cerr << "GLFW Error " << error << ": " << description << std::endl;
        }

        void Create();

        void Run();

    private:
        bool running = false;
        GLFWwindow *window{};
        int m_width;
        int m_height;
        bool m_cloesed = false;
        bool m_dedicatedThread = true;
    };
}

