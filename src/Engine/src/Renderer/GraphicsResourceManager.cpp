//
// Created by XuriAjiva on 22.08.2023.
//

#include "GraphicsResourceManager.h"


namespace Ajiva::Renderer
{
    Ref<Texture> GraphicsResourceManager::GetTexture(const std::filesystem::path& path, uint32_t mipLevelCount)
    {
        auto str = path.string();
        if (textures.contains(str))
        {
            return textures[str];
        }
        else
        {
            auto texture = loader->LoadTextureAsync(path, *context, mipLevelCount);
            textures[str] = texture;
            return texture;
        }
    }

    std::string GraphicsResourceManager::Statistics()
    {
        std::stringstream ss;
        ss << "GraphicsResourceManager: " << std::endl;
        ss << "  Textures: " << textures.size() << std::endl;
        u64 size = 0;
        for (auto& texture : textures)
        {
            size += texture.second->size.width * texture.second->size.height * 4;
        }
        ss << "  Textures Size: " << size << std::endl;
        ss << "  Buffers: " << buffers.size() << std::endl;
        size = 0;
        for (auto& buffer : buffers)
        {
            size += buffer->size;
        }
        ss << "  Buffers Size: " << size << std::endl;
        return ss.str();
    }
} // Ajiva
