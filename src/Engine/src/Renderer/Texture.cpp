//
// Created by XuriAjiva on 02.08.2023.
//

#include "Texture.h"
#include "Core/Logger.h"

namespace Ajiva {
    namespace Renderer {
        Texture::Texture(wgpu::Texture texture, wgpu::TextureView textureView, Ref<wgpu::Queue> queue,
                         wgpu::TextureFormat textureFormat, wgpu::TextureAspect aspect, wgpu::Extent3D textureSize,
                         bool cleanUp)
                : textureFormat(textureFormat), size(textureSize), texture(texture),
                  view(textureView), cleanUp(cleanUp), queue(queue), aspect(aspect) {}

        Texture::~Texture() {
            if (cleanUp) {
                view.release();
                texture.destroy();
                texture.release();
            }
        }

        void Texture::WriteTexture(const void *data, size_t length) {
            using namespace wgpu;
            // Arguments telling which part of the texture we upload to
            // (together with the last argument of writeTexture)
            ImageCopyTexture destination;
            destination.texture = texture;
            destination.mipLevel = 0;
            destination.origin = {0, 0, 0}; // equivalent of the offset argument of Queue::writeBuffer
            destination.aspect = aspect;

            // Arguments telling how the C++ side pixel memory is laid out
            TextureDataLayout source;
            source.offset = 0;
            auto pixelSize = 0;
            if (textureFormat == TextureFormat::RGBA8Unorm) {
                pixelSize = 4;
            } else {
                AJ_FAIL("TextureFormat not supported!");
            }
            source.bytesPerRow = pixelSize * size.width;
            source.rowsPerImage = size.height;

            queue->writeTexture(destination, data, length, source, size);
        }
    } // Ajiva
} // Renderer