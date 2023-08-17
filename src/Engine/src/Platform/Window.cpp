//
// Created by XuriAjiva on 31.07.2023.
//

#include "Window.h"
#include "Core/Logger.h"


namespace Ajiva::Platform {
    Window::Window(const WindowConfig &config, Ref<Core::EventSystem> eventSystem) : config(config),
                                                                                     eventSystem(eventSystem) {
    }

    Window::~Window() {
        if (window) {
            glfwDestroyWindow(window);
        }
    }

    std::function<wgpu::Surface(wgpu::Instance)> Window::CreateSurfaceFunk() {
        return [this](wgpu::Instance instance) -> wgpu::Surface {
            return glfwGetWGPUSurface(instance, window);
        };
    }

    bool Window::CreateWindow() {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        window = glfwCreateWindow(static_cast<i32>(config.Width), static_cast<i32>(config.Height), config.Name.c_str(),
                                  nullptr, nullptr);
        glfwSetWindowPos(window, config.X, config.Y);
        glfwSetWindowUserPointer(window, this);

#define GLFW_USER_PTR_CHECK() \
            auto *windowClass = reinterpret_cast<Window *>(glfwGetWindowUserPointer(pWindow)); \
            if (!windowClass) \
                return;   \
            auto ev = windowClass->eventSystem;

        glfwSetFramebufferSizeCallback(window, [](GLFWwindow *pWindow, int width, int height) {
            GLFW_USER_PTR_CHECK()
            windowClass->config.Width = static_cast<u32>(width);
            windowClass->config.Height = static_cast<u32>(height);
            if (ev->FireEvent(Core::EventCode::FramebufferResize, windowClass, {
                    .framebufferSize = {.width = windowClass->config.Width, .height = windowClass->config.Height}}))
                return;
        });
        glfwSetKeyCallback(window, [](GLFWwindow *pWindow, int key, int scancode, int action, int mods) {
            GLFW_USER_PTR_CHECK()
            auto eventCore = Core::EventCode::None;
            if (action == GLFW_PRESS)
                eventCore = Core::EventCode::KeyDown;
            else if (action == GLFW_RELEASE)
                eventCore = Core::EventCode::KeyUp;
            else return;
            if (ev->FireEvent(eventCore, windowClass,
                              {.key = {.key = key, .scancode = scancode, .action = action, .mods = mods}}))
                return;
        });
        glfwSetMouseButtonCallback(window, [](GLFWwindow *pWindow, int button, int action, int mods) {
            GLFW_USER_PTR_CHECK()
            auto eventCore = Core::EventCode::None;
            if (action == GLFW_PRESS)
                eventCore = Core::EventCode::MouseButtonDown;
            else if (action == GLFW_RELEASE)
                eventCore = Core::EventCode::MouseButtonUp;
            else return;
            f64 x, y;
            glfwGetCursorPos(pWindow, &x, &y);
            if (ev->FireEvent(eventCore, windowClass,
                              {.mouse = {.click = {.x = static_cast<i32>(x), .y = static_cast<i32>(y), .button = button, .mods = mods}}}))
                return;
        });
        glfwSetCursorPosCallback(window, [](GLFWwindow *pWindow, f64 x, f64 y) {
            GLFW_USER_PTR_CHECK()
            Core::EventContext context = {.mouse = {.move = {.x = static_cast<i32>(x), .y = static_cast<i32>(y)}}};
            if (ev->FireEvent(Core::EventCode::MouseMove, windowClass, context))
                return;
        });
        glfwSetScrollCallback(window, [](GLFWwindow *pWindow, f64 xOffset, f64 yOffset) {
            GLFW_USER_PTR_CHECK()
            if (ev->FireEvent(Core::EventCode::MouseScroll, windowClass, {
                    .mouse = {.wheel={
                            .xOffset = xOffset,
                            .yOffset = yOffset
                    }}}))
                return;
        });
#undef GLFW_USER_PTR_CHECK

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
