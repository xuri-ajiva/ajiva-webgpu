//
// Created by XuriAjiva on 13.08.2023.
//

#include "Camera.h"

namespace Ajiva::Renderer {
    void EventCamera::Init() {
        if (init) return;
        this->events.push_back(eventSystem->Add(Core::MouseMove, this, &EventCamera::onMouseMove));
        this->events.push_back(eventSystem->Add(Core::MouseButtonUp, this, &EventCamera::onMouseButtonUp));
        this->events.push_back(eventSystem->Add(Core::MouseButtonDown, this, &EventCamera::onMouseButtonDown));
        this->events.push_back(eventSystem->Add(Core::MouseScroll, this, &EventCamera::onScroll));
        this->events.push_back(eventSystem->Add(Core::KeyDown, this, &EventCamera::onKeyDown));
        this->events.push_back(eventSystem->Add(Core::KeyUp, this, &EventCamera::onKeyUp));
        init = true;
    }

    void EventCamera::onKeyDown(AJ_EVENT_PARAMETERS) {
        onKey(event.key.key, true);
    }

    void EventCamera::onKeyUp(AJ_EVENT_PARAMETERS) {
        onKey(event.key.key, false);
    }

    void OrbitCamera::UpdateViewMatrix() {
        position = vec3(cos(angles.x) * cos(angles.y),
                        sin(angles.x) * cos(angles.y),
                        sin(angles.y))
                   * std::exp(-zoom);
        position += orbitCenter;
        viewMatrix = glm::lookAt(position, vec3(0.0f), vec3(0, 0, 1));
    }

    void OrbitCamera::onMouseMove(AJ_EVENT_PARAMETERS) {
        if (active) {
            vec2 currentMouse = vec2(-event.mouse.move.x, event.mouse.move.y);
            vec2 delta = (currentMouse - startMouse) * sensitivity;
            angles = startAngles + delta;
            // Clamp to avoid going too far when orbitting up/down
            angles.y = glm::clamp(angles.y, -PI / 2 + 1e-5f, PI / 2 - 1e-5f);
            UpdateViewMatrix();
        }
    }

    void OrbitCamera::onMouseButtonUp(AJ_EVENT_PARAMETERS) {
        if (event.mouse.click.button == GLFW_MOUSE_BUTTON_LEFT) {
            active = false;
        }
    }

    void OrbitCamera::onScroll(AJ_EVENT_PARAMETERS) {
        zoom += scrollSensitivity * (float) event.mouse.wheel.yOffset;
        zoom = glm::clamp(zoom, -2.0f, 2.0f);
        UpdateViewMatrix();
    }

    void OrbitCamera::onMouseButtonDown(AJ_EVENT_PARAMETERS) {
        if (event.mouse.click.button == GLFW_MOUSE_BUTTON_LEFT) {
            active = true;
            startMouse = vec2(-event.mouse.click.x, event.mouse.click.y);
            startAngles = OrbitCamera::angles;
        }
    }

    void FreeCamera::onMouseMoved(float xRel, float yRel) {
        angles.x += xRel * mouseSensitivity;
        angles.y -= yRel * mouseSensitivity;
        angles.y = glm::clamp(angles.y, -89.0f, 89.0f);

        front.x = cos(glm::radians(angles.y)) * sin(glm::radians(angles.x));
        front.y = cos(glm::radians(angles.y)) * cos(glm::radians(angles.x));
        front.z = sin(glm::radians(angles.y));
        front = glm::normalize(front);
        viewMatrix = glm::lookAt(position, position + front, up);
    }

    void FreeCamera::translate(glm::vec3 v) {
        position += v;
        viewMatrix = glm::lookAt(position, position + front, up);
    }

    void FreeCamera::Update() {
        acceleration *= 0.9f;
        if (forward_down)
            acceleration += front * speed;
        if (backward_down)
            acceleration -= front * speed;
        if (left_down)
            acceleration -= glm::cross(front, up) * speed;
        if (right_down)
            acceleration += glm::cross(front, up) * speed;
        if (up_down)
            acceleration += up * speed;
        if (down_down)
            acceleration -= up * speed;

        if (glm::length(acceleration) < 0.001f) return;
        position += acceleration;
        viewMatrix = glm::lookAt(position, position + front, up);
    }

    void FreeCamera::onMouseMove(AJ_EVENT_PARAMETERS) {
        if (active) {
            onMouseMoved(event.mouse.move.dx, event.mouse.move.dy);
        }
    }

    void FreeCamera::onMouseButtonUp(AJ_EVENT_PARAMETERS) {
        if (event.mouse.click.button == GLFW_MOUSE_BUTTON_LEFT) {
            active = false;
        }
    }

    void FreeCamera::onMouseButtonDown(AJ_EVENT_PARAMETERS) {
        if (event.mouse.click.button == GLFW_MOUSE_BUTTON_LEFT) {
            active = true;
        }
    }

    void FreeCamera::onKey(u32 key, bool down) {
        EventCamera::onKey(key, down);
        switch (key) {
            case GLFW_KEY_W:
                forward_down = down;
                break;
            case GLFW_KEY_S:
                backward_down = down;
                break;
            case GLFW_KEY_A:
                left_down = down;
                break;
            case GLFW_KEY_D:
                right_down = down;
                break;
            case GLFW_KEY_SPACE:
                up_down = down;
                break;
            case GLFW_KEY_LEFT_SHIFT:
                down_down = down;
                break;
            default:
                break;
        }
    }

    void FreeCamera::onScroll(AJ_EVENT_PARAMETERS) {
        speed += event.mouse.wheel.yOffset * 0.01f;
        speed = glm::clamp(speed, 0.01f, 5.0f);
    }

    void FreeCamera::Init() {
        EventCamera::Init();
        onMouseMoved(0, 0);
    }
}