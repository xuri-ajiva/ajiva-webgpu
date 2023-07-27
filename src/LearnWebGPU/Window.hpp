//
// Created by XuriAjiva on 27.07.2023.
//

#pragma once

#include "webgpu/webgpu.hpp"
#include <GLFW/glfw3.h>
#include <functional>
#include <thread>

namespace Ajiva {

    class Window {
        GLFWwindow *window;
        int m_width;
        int m_height;
        bool cloesed = false;
    public:
        explicit Window(int width = 800, int height = 600)
                : m_width(width), m_height(height) {
            if (!glfwInit()) {
                std::cerr << "Could not initialize GLFW!" << std::endl;
                return;
            }

        }


        ~Window() {
            if (window)
                glfwDestroyWindow(window);
            glfwTerminate();
        }

        [[nodiscard]] GLFWwindow *GetWindow() const {
            return window;
        }


        [[nodiscard]] int GetWidth() const {
            return m_width;
        }

        [[nodiscard]] int GetHeight() const {
            return m_height;
        }

        [[nodiscard]] bool IsClosed() const {
            return cloesed;
        }

        void RequestClose() {
            cloesed = true;
        }


        [[nodiscard]] std::function<wgpu::Surface(wgpu::Instance)> CreateSurfaceFunk() {
            return [this](wgpu::Instance instance) -> wgpu::Surface {
                return glfwGetWGPUSurface(instance, window);
            };
        }

        void Run() {
            auto thread = std::thread([this]() {
                glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
                glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
                window = glfwCreateWindow(m_width, m_height, "Learn WebGPU", NULL, NULL);
                if (!window) {
                    std::cerr << "Could not open window!" << std::endl;
                    glfwTerminate();
                    return;
                }
                glfwMakeContextCurrent(window);
                while (!glfwWindowShouldClose(window)) {
                    glfwPollEvents();
                    //sleep
                    glfwWaitEventsTimeout(0.01);

                    if (cloesed) {
                        glfwSetWindowShouldClose(window, GLFW_TRUE);
                    }
                }
                cloesed = true;
            });
            thread.detach();
        }
    };
}

