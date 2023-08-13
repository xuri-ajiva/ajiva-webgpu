/*
//
// Created by XuriAjiva on 13.08.2023.
//

#pragma once

#include "GpuContext.h"
#include "Platform/Window.h"

namespace Ajiva::Renderer {
    class Renderer {
    public:
        explicit Renderer(const GpuContext &context, const Window &window) : context(context), window(window) {}
        void Init();

        void Render();

        void Finish();

        ~Renderer();

    private:
        GpuContext context = {};
        Ajiva::Window window;
        wgpu::SwapChain swapChain = nullptr;
    };
}
*/
