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
    struct CameraState {
        // angles.x is the rotation of the camera around the global vertical axis, affected by mouse.x
        // angles.y is the rotation of the camera around its local horizontal axis, affected by mouse.y
        vec2 angles = {0.8f, 0.5f};
        // zoom is the position of the camera along its local forward axis, affected by the scroll wheel
        float zoom = -1.2f;
    };

    struct DragState {
        // Whether a drag action is ongoing (i.e., we are between mouse press and mouse release)
        bool active = false;
        // The position of the mouse at the beginning of the drag action
        vec2 startMouse;
        // The camera state at the beginning of the drag action
        CameraState startCameraState;

        // Constant settings
        float sensitivity = 0.01f;
        float scrollSensitivity = 0.1f;
    };

    class AJ_API Camera {
        bool init = false;
    public:
        Camera() = default;

        explicit Camera(const Ref<Ajiva::Core::EventSystem> &eventSystem)
                : eventSystem(eventSystem), viewMatrix(glm::lookAt(vec3(0, 0, 1), vec3(0, 0, 0), vec3(0, 0, 1))) {}

        void Init();

        ~Camera() {
            if (!init) return;
        }

        glm::mat4 viewMatrix;

        void updateViewMatrix();

        void onMouseMove(AJ_EVENT_PARAMETERS);

        void onMouseButtonDown(AJ_EVENT_PARAMETERS);

        void onMouseButtonUp(AJ_EVENT_PARAMETERS);

        void onScroll(AJ_EVENT_PARAMETERS);

    private:
        CameraState m_cameraState;
        DragState m_drag;
        Ref<Ajiva::Core::EventSystem> eventSystem;
        std::vector<Ref<Core::IListener>> events;
    };
}
