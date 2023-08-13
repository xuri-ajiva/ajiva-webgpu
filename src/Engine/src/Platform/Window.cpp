//
// Created by XuriAjiva on 31.07.2023.
//

#include "Window.h"
#include "Core/Logger.h"


namespace Ajiva::Platform {
    Window::Window(const WindowConfig &config) : config(config) {
    }

    Window::~Window() {
        if (window)
            glfwDestroyWindow(window);
    }

    std::function<wgpu::Surface(wgpu::Instance)> Window::CreateSurfaceFunk() {
        return [this](wgpu::Instance instance) -> wgpu::Surface {
            return glfwGetWGPUSurface(instance, window);
        };
    }

    bool Window::CreateWindow() {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, config.ResizeCallback ? GLFW_TRUE : GLFW_FALSE);
        window = glfwCreateWindow(config.Width, config.Height, config.Name.c_str(), nullptr, nullptr);
        glfwSetWindowPos(window, config.X, config.Y);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, [](GLFWwindow *pWindow, int width, int height) {
            auto *windowClass = reinterpret_cast<Window *>(glfwGetWindowUserPointer(pWindow));
            windowClass->config.Width = static_cast<u16>(width);
            windowClass->config.Height = static_cast<u16>(height);
            if (windowClass->config.ResizeCallback)
                windowClass->config.ResizeCallback(width, height);
        });
        if (!window) {
            PLOG_ERROR << "Could not create GLFW window!";
            glfwTerminate();
            return false;
        }
        return true;
    }

    void Window::Create() {
        if (config.DedicatedThread) {
            auto thread = std::thread([this]() {
                if (!CreateWindow()) {
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
            if (!CreateWindow())
                return;
        }
    }

    void Window::Run() {
        running = true;
    }

    bool Window::IsClosed() {
        if (config.DedicatedThread)
            return m_cloesed;
        if (!running)
            return true;
        glfwPollEvents();
        return glfwWindowShouldClose(window);
    }

     bool Window::Init() {
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit()) {
            PLOG_ERROR << "Could not initialize GLFW!";
            return false;
        } else {
            PLOG_VERBOSE << "GLFW initialized, IsGLFWInitialized: " << IsGLFWInitialized;
            IsGLFWInitialized = true;
            PLOG_INFO << "GLFW initialized, IsGLFWInitialized: " << IsGLFWInitialized;
        }
        return true;
    }

    void Window::Shutdown() {
        if (IsGLFWInitialized)
            glfwTerminate();
    }
}
