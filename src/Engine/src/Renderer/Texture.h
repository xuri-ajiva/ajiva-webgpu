//
// Created by XuriAjiva on 02.08.2023.
//

#pragma once

#include "defines.h"
#include "webgpu/webgpu.hpp"

namespace Ajiva {
    namespace Renderer {

        class Texture {
            bool cleanUp = true;
            Ref<wgpu::Queue> queue;
        public:
            wgpu::TextureFormat textureFormat;
            wgpu::Extent3D size;

            wgpu::Texture texture;
            wgpu::TextureView view;
            wgpu::TextureAspect aspect;

            Texture(wgpu::Texture texture, wgpu::TextureView textureView, Ref<wgpu::Queue> queue,
                    wgpu::TextureFormat textureFormat, wgpu::TextureAspect aspect, wgpu::Extent3D textureSize, bool cleanUp = true);

            ~Texture();

            void WriteTexture(const void *data, size_t length);
        };

    } // Ajiva
} // Renderer

