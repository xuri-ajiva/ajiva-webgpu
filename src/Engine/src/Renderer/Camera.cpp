//
// Created by XuriAjiva on 13.08.2023.
//

#include "Camera.h"

namespace Ajiva::Renderer {
    void Camera::Init() {
        this->events.push_back(eventSystem->Add(Core::EventCode::MouseMove, this, &Camera::onMouseMove));
        this->events.push_back(eventSystem->Add(Core::EventCode::MouseButtonUp, this, &Camera::onMouseButtonUp));
        this->events.push_back(eventSystem->Add(Core::EventCode::MouseButtonDown, this, &Camera::onMouseButtonDown));
        this->events.push_back(eventSystem->Add(Core::EventCode::MouseScroll, this, &Camera::onScroll));
        updateViewMatrix();
        init = true;
    }

    void Camera::updateViewMatrix() {
        float cx = cos(m_cameraState.angles.x);
        float sx = sin(m_cameraState.angles.x);
        float cy = cos(m_cameraState.angles.y);
        float sy = sin(m_cameraState.angles.y);
        vec3 position = vec3(cx * cy, sx * cy, sy) * std::exp(-m_cameraState.zoom);
        viewMatrix = glm::lookAt(position, vec3(0.0f), vec3(0, 0, 1));
    }

    void Camera::onMouseMove(Ajiva::Core::EventCode code, void *sender, const Core::EventContext &event) {
        if (m_drag.active) {
            vec2 currentMouse = vec2(-event.mouse.move.x, event.mouse.move.y);
            vec2 delta = (currentMouse - m_drag.startMouse) * m_drag.sensitivity;
            m_cameraState.angles = m_drag.startCameraState.angles + delta;
            // Clamp to avoid going too far when orbitting up/down
            m_cameraState.angles.y = glm::clamp(m_cameraState.angles.y, -PI / 2 + 1e-5f, PI / 2 - 1e-5f);
            updateViewMatrix();
        }
    }

    void Camera::onMouseButtonUp(Ajiva::Core::EventCode code, void *sender, const Core::EventContext &event) {
        if (event.mouse.click.button == GLFW_MOUSE_BUTTON_LEFT) {
            m_drag.active = false;
        }
    }

    void Camera::onScroll(Ajiva::Core::EventCode code, void *sender, const Core::EventContext &event) {
        m_cameraState.zoom += m_drag.scrollSensitivity * (float) event.mouse.wheel.yOffset;
        m_cameraState.zoom = glm::clamp(m_cameraState.zoom, -2.0f, 2.0f);
        updateViewMatrix();
    }

    void Camera::onMouseButtonDown(Ajiva::Core::EventCode code, void *sender, const Core::EventContext &event) {
        if (event.mouse.click.button == GLFW_MOUSE_BUTTON_LEFT) {
            m_drag.active = true;
            m_drag.startMouse = vec2(-event.mouse.click.x, event.mouse.click.y);
            m_drag.startCameraState = m_cameraState;
        }
    }
}