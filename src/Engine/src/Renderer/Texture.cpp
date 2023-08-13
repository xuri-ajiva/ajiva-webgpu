//
// Created by XuriAjiva on 02.08.2023.
//

#include "Texture.h"
#include "Core/Logger.h"
#include "Resource/ResourceManager.h"
#include <utility>
#include <vector>

namespace Ajiva::Renderer {
    Texture::Texture(wgpu::Texture texture, wgpu::TextureView textureView, Ref<wgpu::Queue> queue,
                     wgpu::TextureFormat textureFormat, wgpu::TextureAspect aspect, wgpu::Extent3D textureSize,
                     bool cleanUp)
            : textureFormat(textureFormat), size(textureSize), texture(texture),
              view(textureView), cleanUp(cleanUp), queue(std::move(queue)), aspect(aspect) {
        AJ_RegisterCreated(this, typeid(Texture));
    }

    Texture::~Texture() {
        if (cleanUp) {
            view.release();
            texture.destroy();
            texture.release();
            AJ_RegisterDestroyed(this, typeid(Texture));
        }
    }

    void Texture::WriteTexture(const void *data, size_t length, wgpu::Extent3D writeSize, uint32_t mipLevel) {
        using namespace wgpu;
        if (!writeSize.width || !writeSize.height || !writeSize.depthOrArrayLayers) {
            PLOG_INFO << "Texture Write Size not set, using default size";
            writeSize.depthOrArrayLayers = size.depthOrArrayLayers;
            writeSize.height = size.height >> mipLevel;
            writeSize.width = size.width >> mipLevel;
        }
        if (writeSize.width > size.width >> mipLevel) {
            PLOG_ERROR << "writeSize.width > size.width >> mipLevel (" << writeSize.width << " > "
                       << (size.width >> mipLevel) << ") correcting...";
            writeSize.width = size.width >> mipLevel;
        }
        if (writeSize.height > size.height >> mipLevel) {
            PLOG_ERROR << "writeSize.height > size.height >> mipLevel (" << writeSize.height << " > "
                       << (size.height >> mipLevel) << ") correcting...";
            writeSize.height = size.height >> mipLevel;
        }
        // Arguments telling which part of the texture we upload to
        // (together with the last argument of writeTexture)
        ImageCopyTexture destination;
        destination.texture = texture;
        destination.mipLevel = mipLevel;
        destination.origin = {0, 0,
                              0}; // equivalent of the offset argument of Queue::writeBuffer //todo add parameter?
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
        source.bytesPerRow = pixelSize * writeSize.width;
        source.rowsPerImage = writeSize.height;

        queue->writeTexture(destination, data, length, source, writeSize);
    }

    void Texture::WriteTextureMips(const void *data, size_t length, uint32_t mipLevelCount) {
        using namespace wgpu;
        if (length != 4 * size.width * size.height) {
            PLOG_ERROR << "length != 4 * size.width * size.height (" << length << " != "
                       << (4 * size.width * size.height) << ")";
        }
        std::vector<u8> previousLevelPixels(4 * size.width * size.height);

        //Level 0 Original Texture
        memcpy(previousLevelPixels.data(), data, previousLevelPixels.size());
        // Upload the current level
        WriteTexture(previousLevelPixels.data(), previousLevelPixels.size(), size, 0);

        Extent3D mipLevelSize = {size.width / 2, size.height / 2, size.depthOrArrayLayers},
                previousMipLevelSize = size;

        for (uint32_t level = 1; level < mipLevelCount; ++level) {
            // Pixel data for the current level
            std::vector<u8> pixels(4 * mipLevelSize.width * mipLevelSize.height);
            // Create mip level data
            for (uint32_t i = 0; i < mipLevelSize.width; ++i) {
                for (uint32_t j = 0; j < mipLevelSize.height; ++j) {
                    u8 *p = &pixels[4 * (j * mipLevelSize.width + i)];
                    // Get the corresponding 4 pixels from the previous level
                    u8 *p00 = &previousLevelPixels[4 * ((2 * j + 0) * previousMipLevelSize.width + (2 * i + 0))];
                    u8 *p01 = &previousLevelPixels[4 * ((2 * j + 0) * previousMipLevelSize.width + (2 * i + 1))];
                    u8 *p10 = &previousLevelPixels[4 * ((2 * j + 1) * previousMipLevelSize.width + (2 * i + 0))];
                    u8 *p11 = &previousLevelPixels[4 * ((2 * j + 1) * previousMipLevelSize.width + (2 * i + 1))];
                    // Average
                    p[0] = (p00[0] + p01[0] + p10[0] + p11[0]) / 4;
                    p[1] = (p00[1] + p01[1] + p10[1] + p11[1]) / 4;
                    p[2] = (p00[2] + p01[2] + p10[2] + p11[2]) / 4;
                    p[3] = (p00[3] + p01[3] + p10[3] + p11[3]) / 4;
                }
            }
            // Upload the current level
            WriteTexture(pixels.data(), pixels.size(), mipLevelSize, level);

            previousLevelPixels = std::move(pixels);
            previousMipLevelSize = mipLevelSize;
            mipLevelSize.width /= 2;
            mipLevelSize.height /= 2;

            if (mipLevelSize.width == 0) {
                PLOG_WARNING << "mipLevelSize.width == 0";
                mipLevelSize.width = 1;
            }
            if (mipLevelSize.height == 0) {
                PLOG_WARNING << "mipLevelSize.height == 0";
                mipLevelSize.height = 1;
            }

        }
    }

} // Ajiva::Renderer