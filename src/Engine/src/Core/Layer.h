//
// Created by XuriAjiva on 15.08.2023.
//

#pragma once

#include "defines.h"
#include <string>
#include "webgpu/webgpu.hpp"

namespace Ajiva::Core {
    struct FrameInfo {
        u64 FrameNumber;
        f64 FrameDelta;
        f64 TotalTime;
    };

    class AJ_API Layer {
    public:
        explicit Layer(std::string name) : name(std::move(name)) {}

        virtual ~Layer() = default;

        virtual void Attached() {}

        virtual void Detached() {}

        virtual void BeforeRender() {}

        virtual void Render(FrameInfo frameInfo) {}

        virtual void AfterRender(wgpu::RenderPassEncoder renderpass) {}

        [[nodiscard]] inline const std::string &Name() const {
            return name;
        }

        [[nodiscard]] inline bool IsEnabled() const {
            return enabled;
        }

    private:
        std::string name;
        bool enabled = true;

    };

} // Ajiva
// Core
