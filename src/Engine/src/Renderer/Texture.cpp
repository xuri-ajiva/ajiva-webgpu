//
// Created by XuriAjiva on 02.08.2023.
//

#include "Texture.h"

namespace Ajiva {
    namespace Renderer {
        Texture::Texture(wgpu::Texture texture, wgpu::TextureView textureView, wgpu::TextureFormat textureFormat,
                         wgpu::Extent3D textureSize, bool cleanUp)
                : textureFormat(textureFormat), textureSize(textureSize), texture(texture),
                  textureView(textureView), cleanUp(cleanUp) {}

        Texture::~Texture() {
            if (cleanUp) {
                textureView.release();
                texture.destroy();
                texture.release();
            }
        }
    } // Ajiva
} // Renderer