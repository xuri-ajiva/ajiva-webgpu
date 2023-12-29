//
// Created by XuriAjiva on 15.08.2023.
//

#pragma once

#include "defines.h"
#include <string>
#include "webgpu/webgpu.hpp"

namespace Ajiva::Core
{
    struct UpdateInfo
    {
        u64 FrameNumber;
        f64 FrameDelta;
        f64 TotalTime;
    };

    struct RenderTarget
    {
        wgpu::TextureView texture;
        wgpu::Extent3D extent; //todo include start offset
    };

    class AJ_API Layer
    {
    public:
        explicit Layer(std::string name) : name(std::move(name))
        {
        }

        Layer() = default;

        virtual ~Layer() = default;

        virtual bool Attached() { return false; }

        virtual void Detached()
        {
        }

        virtual void BeforeRender(UpdateInfo frameInfo, RenderTarget target)
        {
        }

        virtual void Render(UpdateInfo frameInfo, RenderTarget target)
        {
        }

        virtual void AfterRender(UpdateInfo frameInfo, RenderTarget target)
        {
        }

        virtual void BeforeUpdate(UpdateInfo frameInfo)
        {
        }

        virtual void Update(UpdateInfo frameInfo)
        {
        }

        virtual void AfterUpdate(UpdateInfo frameInfo)
        {
        }

        [[nodiscard]] inline const std::string& Name() const
        {
            return name;
        }

        [[nodiscard]] inline bool IsEnabled() const
        {
            return enabled;
        }

    private:
        std::string name;
        bool enabled = true;
    };
} // Ajiva
// Core
