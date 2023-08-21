//
// Created by XuriAjiva on 13.08.2023.
//

#pragma once

#include "defines.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "GLFW/glfw3.h"
#include "Core/EventSystem.h"
#include "Core/Logger.h"

namespace Ajiva::Renderer {
    using glm::vec3;
    using glm::vec2;

    struct Projection {
        float fov = 45.0f;
        float aspect = 1.0f;
        float near = 0.1f;
        float far = 1000.0f;

        [[nodiscard]] glm::mat4 Build() const {
            return glm::perspective(fov, aspect, near, far);
        }
    };

    class AJ_API Camera {
    public:
        Camera() = default;

        explicit Camera(glm::mat4 viewMatrix) : viewMatrix(viewMatrix) {}

        virtual void Update() = 0;

        virtual void translate(glm::vec3 v) {
            position += v;
            viewMatrix = glm::translate(viewMatrix, v * -1.0f);
        }

        glm::vec3 position{0};
        glm::mat4 viewMatrix{0};
    protected:

    };

    class EventCamera : public Camera {
    public:
        virtual void Init();

        EventCamera() = default;

        explicit EventCamera(const Ref<Core::EventSystem> &eventSystem)
                : Camera(glm::lookAt(glm::vec3(0, 0, 1), glm::vec3(0, 0, 0), glm::vec3(0, 0, 1))),
                  eventSystem(eventSystem) {}

    protected:
        virtual void onMouseMove(AJ_EVENT_PARAMETERS) = 0;

        virtual void onMouseButtonUp(AJ_EVENT_PARAMETERS) = 0;

        virtual void onScroll(AJ_EVENT_PARAMETERS) = 0;

        virtual void onMouseButtonDown(AJ_EVENT_PARAMETERS) = 0;

        virtual void onKeyDown(AJ_EVENT_PARAMETERS);

        virtual void onKeyUp(AJ_EVENT_PARAMETERS);

        virtual void onKey(u32 key, bool down) {};

        bool init = false;
        Ref<Ajiva::Core::EventSystem> eventSystem;
        std::vector<Ref<Core::IListener>> events;
    };

    class AJ_API OrbitCamera : public EventCamera {
    public:
        ~OrbitCamera() {
            if (!init) return;
        }

        OrbitCamera() = default;

        explicit OrbitCamera(const Ref<Core::EventSystem> &eventSystem)
                : EventCamera(eventSystem) {}

        void UpdateViewMatrix();

        void Update() override {
            //   UpdateViewMatrix();
        }

        void Init() override {
            EventCamera::Init();
            UpdateViewMatrix();
        }

        void translate(glm::vec3 v) override {
            orbitCenter += v;
            viewMatrix = glm::translate(viewMatrix, v * -1.0f);
        }

    protected:
        void onMouseMove(AJ_EVENT_PARAMETERS) override;

        void onMouseButtonUp(AJ_EVENT_PARAMETERS) override;

        void onScroll(AJ_EVENT_PARAMETERS) override;

        void onMouseButtonDown(AJ_EVENT_PARAMETERS) override;

    private:
        vec3 orbitCenter = {0, 0, 0};
        bool active = false;
        vec2 angles = {0.8f, 0.5f};
        float scrollSensitivity = 0.1f;
        float sensitivity = 0.01f;
        vec2 startAngles = {0, 0};
        vec2 startMouse = {0, 0};
        float zoom = -1.2f;
    };

    class FreeCamera : public EventCamera {
        friend class ImGuiLayer;
    public:
        explicit FreeCamera(const Ref<Core::EventSystem> &eventSystem)
                : EventCamera(eventSystem) {}

        void Init() override;

        void onMouseMoved(float xRel, float yRel);

        void Update() override;

        void translate(glm::vec3 v) override;

    private:
        glm::vec2 angles = {0, 0};
        glm::vec3 front = glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);
        float mouseSensitivity = 0.3f;
        bool active = false;
        vec3 acceleration = {0, 0, 0};
        float speed = 0.1f;

        void onMouseMove(AJ_EVENT_PARAMETERS) override;

        void onMouseButtonUp(AJ_EVENT_PARAMETERS) override;

        void onScroll(AJ_EVENT_PARAMETERS) override;

        void onMouseButtonDown(AJ_EVENT_PARAMETERS) override;

        void onKey(u32 key, bool down) override;

    private:
        bool forward_down = false;
        bool backward_down = false;
        bool left_down = false;
        bool right_down = false;
        bool up_down = false;
        bool down_down = false;
        Ref<Ajiva::Core::EventSystem> eventSystem;
        std::vector<Ref<Core::IListener>> events;
    };
}
