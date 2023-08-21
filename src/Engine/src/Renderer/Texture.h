//
// Created by XuriAjiva on 02.08.2023.
//

#pragma once

#include "defines.h"
#include "webgpu/webgpu.hpp"

namespace Ajiva {
    namespace Renderer {

        class AJ_API Texture {
        public:
            wgpu::TextureFormat textureFormat;
            wgpu::Extent3D size; //todo already present in texture!

            wgpu::Texture texture;
            wgpu::TextureView view;
            wgpu::TextureAspect aspect;

            Texture(wgpu::Texture texture, wgpu::TextureView textureView, Ref<wgpu::Queue> queue,
                    wgpu::TextureFormat textureFormat, wgpu::TextureAspect aspect, wgpu::Extent3D textureSize,
                    bool cleanUp = true);

            void SwapBackingTexture(const Ref<Texture> &other);

            ~Texture();

            void Destroy();

            [[nodiscard]] u64 GetVersion();

            void
            WriteTexture(const void *data, size_t length, wgpu::Extent3D writeSize = {0, 0, 0}, uint32_t mipLevel = 0);

            void WriteTextureMips(const void *data, size_t length, uint32_t mipLevelCount);

            AJ_INLINE void SetCleanUp(bool pCleanUp) { Texture::cleanUp = pCleanUp; }

        private:
            void SwapBackingTextureInternal();
            bool cleanUp = true;
            Ref<wgpu::Queue> queue;
            Ref<Texture> toSwap = nullptr;
            u64 version = INVALID_ID_U64;
        };

    } // Ajiva
} // Renderer

