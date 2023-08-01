//
// Created by XuriAjiva on 31.07.2023.
//

#include "Window.h"
#include "Core/Logger.h"


namespace Ajiva {
    Window::~Window() {
        if (window)
            glfwDestroyWindow(window);
        glfwTerminate();
    }


    Window::Window(int width, int height, bool dedicatedThread)
            : m_width(width), m_height(height), m_dedicatedThread(dedicatedThread) {
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit()) {
            PLOG_ERROR << "Could not initialize GLFW!";
            return;
        }
    }

    std::function<wgpu::Surface(wgpu::Instance)> Window::CreateSurfaceFunk() {
        return [this](wgpu::Instance instance) -> wgpu::Surface {
            return glfwGetWGPUSurface(instance, window);
        };
    }

    bool CreateWindow(GLFWwindow **window, int m_width, int m_height) {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        *window = glfwCreateWindow(m_width, m_height, "Ajiva Engine", NULL, NULL);
        if (!*window) {
            PLOG_ERROR << "Could not create GLFW window!";
            glfwTerminate();
            return false;
        }
        return true;
    }

    void Window::Create() {
        if (m_dedicatedThread) {
            auto thread = std::thread([this]() {
                if (!CreateWindow(&window, m_width, m_height)) {
                    m_cloesed = true;
                    return;
                }

                while (!running) {
                    // do not freeze the window
                    glfwPollEvents();
                }

                while (!glfwWindowShouldClose(window)) {
                    glfwPollEvents();
                    //glfwWaitEventsTimeout(0.01);

                    if (!running) {
                        glfwSetWindowShouldClose(window, GLFW_TRUE);
                    }
                }
                m_cloesed = true;
            });
            thread.detach();
            while (!window && !m_cloesed) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            PLOG_INFO << "Window created!";
        } else {
            if (!CreateWindow(&window, m_width, m_height))
                return;
        }
    }

    void Window::Run() {
        running = true;
    }

    bool Window::IsClosed() {
        if (m_dedicatedThread)
            return m_cloesed;
        if (!running)
            return true;
        glfwPollEvents();
        return glfwWindowShouldClose(window);
    }
}
