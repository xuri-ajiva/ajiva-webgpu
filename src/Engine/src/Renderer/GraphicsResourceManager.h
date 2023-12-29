//
// Created by XuriAjiva on 22.08.2023.
//

#pragma once

#include "defines.h"
#include "Renderer/Buffer.h"
#include "Renderer/Texture.h"
#include "Resource/Loader.h"
#include "Model.h"

#include <vector>
#include <map>

namespace Ajiva::Renderer
{
    class GraphicsResourceManager
    {
        friend class Ajiva::Resource::Loader;

    public:
        GraphicsResourceManager() = default;

        explicit GraphicsResourceManager(Ref<GpuContext> context, Ref<Resource::Loader> loader)
            : context(std::move(context)), loader(std::move(loader))
        {
        }

        std::string Statistics();

        Ref<Texture> GetTexture(const std::filesystem::path& path, uint32_t mipLevelCount = 0);

        Ref<Model> GetModel(const std::filesystem::path &path) {
            const std::string &key = path.string();
            Ref<Model> model = models[key];
            if (model) {
                return model;
            }
            model = CreateRef<Model>();
            bool success = loader->LoadGeometryFromObj(path, model->vertexData, model->indexData);

            if (!success)
            {
                std::cerr << "Could not load geometry!" << std::endl;
                return nullptr;
            }
            model->vertexBuffer = context->CreateFilledBuffer(model->vertexData.data(),
                                                              model->vertexData.size() *
                                                              sizeof(
                                                                  Ajiva::Renderer::VertexData),
                                                              wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex,
                                                              "Vertex Buffer");
            /*auto indexBuffer = context->CreateFilledBuffer(indexData.data(), indexData.size() * sizeof(uint16_t),
                                                          wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index,
                                                          "Index Buffer");*/
            models[key] = model;
            return model;
        }

    private:
        Ref<GpuContext> context;
        Ref<Resource::Loader> loader;
        std::map<std::string, Ref<Texture>> textures;
        std::map<std::string, Ref<Model>> models;
        std::vector<Ref<Buffer>> buffers; //will be removed in favor of mesh / uniform / lightning
    };
} // Ajiva
// Renderer
