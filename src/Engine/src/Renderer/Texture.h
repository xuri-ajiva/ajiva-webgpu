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
        public:
            wgpu::TextureFormat textureFormat;
            wgpu::Extent3D textureSize;

            wgpu::Texture texture;
            wgpu::TextureView textureView;

            Texture(wgpu::Texture texture, wgpu::TextureView textureView, wgpu::TextureFormat textureFormat,
                    wgpu::Extent3D textureSize, bool cleanUp = true);

            ~Texture();
        };

    } // Ajiva
} // Renderer

