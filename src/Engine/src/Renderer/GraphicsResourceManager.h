//
// Created by XuriAjiva on 22.08.2023.
//

#pragma once

#include "defines.h"
#include "Renderer/Buffer.h"
#include "Renderer/Texture.h"
#include "Resource/Loader.h"

#include <vector>
#include <map>

namespace Ajiva::Renderer {
    class GraphicsResourceManager {
        friend class Ajiva::Resource::Loader;
    public:
        GraphicsResourceManager() = default;

        explicit GraphicsResourceManager(Ref<GpuContext> context, Ref<Resource::Loader> loader)
                : context(std::move(context)), loader(std::move(loader)) {}

        std::string Statistics();

        Ref<Texture> GetTexture(const std::filesystem::path &path, uint32_t mipLevelCount = 0);



    private:
        Ref<GpuContext> context;
        Ref<Resource::Loader> loader;
        std::map<std::string, Ref<Texture>> textures;
        std::vector<Ref<Buffer>> buffers; //will be removed in favor of mesh / uniform / lightning
    };

} // Ajiva
// Renderer
