//
// Created by XuriAjiva on 05.09.2023.
//

#pragma once

#include "defines.h"

#include <vector>
#include <map>
#include "Structures.h"
#include "Buffer.h"
#include "GpuContext.h"
#include "Core/Layer.h"

namespace Ajiva::Renderer
{
    class InstanceModelManager;

    class GraphicsResourceManager;

    class Model
    {
    public:
        Model(u64 id, const std::vector<Ajiva::Renderer::VertexData>& vertexData,
              const std::vector<u16>& indexData, const Ref<Buffer>& vertexBuffer, const Ref<Buffer>& indexBuffer);

        Model() = default;

    public:
        u64 id;

        std::vector<Ajiva::Renderer::VertexData> vertexData;
        std::vector<u16> indexData;

        Ref<Ajiva::Renderer::Buffer> vertexBuffer = nullptr;
        Ref<Ajiva::Renderer::Buffer> indexBuffer = nullptr;
        friend InstanceModelManager;
        friend GraphicsResourceManager;
    };

    struct InstanceModelData
    {
        Ref<Model> model;
        std::vector<Ajiva::Renderer::InstanceData> instanceData;
        std::vector<bool> modified;

        Ref<Ajiva::Renderer::Buffer> instanceBuffer = nullptr; //todo allow for more than one buffer bc buffer limit is 268435456 (256MiB)
    };

    struct ModelInstance
    {
        ModelInstance() = default;

        Ref<InstanceModelData> model = nullptr;
        u64 instanceIndex = 0;

        Ajiva::Renderer::InstanceData& data() const
        {
            return model->instanceData[instanceIndex];
        }
    };

    class InstanceModelManager
    {
    public:
        explicit InstanceModelManager(const Ref<GpuContext>& context) : context(context)
        {
        }

        Scope<ModelInstance> CreateInstance(const Ref<Model>& model)
        {
            //find InstaceModelData
            auto& instanceModelData = models[model->id];
            //if not found create
            if (!instanceModelData)
            {
                instanceModelData = CreateRef<InstanceModelData>();
                instanceModelData->model = model;
            }
            //create Instance
            auto instance = CreateScope<ModelInstance>();
            instance->model = instanceModelData;
            instance->instanceIndex = instanceModelData->instanceData.size();
            //add InstanceData
            auto newData = instanceModelData->instanceData.emplace_back();
            auto size = sizeof(InstanceData) * instanceModelData->instanceData.capacity();
            if (!instanceModelData->instanceBuffer || instanceModelData->instanceBuffer->size != size)
            {
                instanceModelData->instanceBuffer = context->CreateFilledBuffer(instanceModelData->instanceData.data(),
                    size,
                    wgpu::BufferUsage::CopyDst |
                    wgpu::BufferUsage::Vertex); //todo check if usage is correct
            }
            else
            {
                instanceModelData->instanceBuffer->UpdateBufferData(&newData, sizeof(InstanceData),
                                                                    sizeof(InstanceData) * instance->instanceIndex);
            }
            return instance;
        }

        //update for data
        void Update()
        {
            for (auto& model : models)
            {
                if (!model.second->instanceBuffer)
                    continue;
                //todo check if data is modified / batch block update
                auto size = sizeof(InstanceData) * model.second->instanceData.capacity();
                model.second->instanceBuffer->UpdateBufferData(model.second->instanceData.data(), size);
            }
        }

        static void RenderModel(wgpu::RenderPassEncoder renderPass, const Ref<InstanceModelData>& model)
        {
            renderPass.setVertexBuffer(0, model->model->vertexBuffer->buffer, 0, model->model->vertexBuffer->size);

            //TODO       foreach instance buffer -> set -> draw
            renderPass.setVertexBuffer(1, model->instanceBuffer->buffer, 0, model->instanceBuffer->size);
            /* renderPass.setIndexBuffer(model->model->indexBuffer->buffer, wgpu::IndexFormat::Uint16, 0,
                                       model->indexData.size() * sizeof(uint16_t));*/
            //renderPass.drawIndexed(model->model->indexData.size(), 1, 0, 0, 0);
            renderPass.draw(model->model->vertexBuffer->size / sizeof(Ajiva::Renderer::VertexData),
                            model->instanceData.size(), 0, 0);
        }

        void Render(wgpu::RenderPassEncoder renderPass)
        {
            for (auto& model : models)
            {
                RenderModel(renderPass, model.second);
            }
        }

        //model id -> modelInstanceData
        std::map<u64, Ref<InstanceModelData>> models;

    private:
        Ref<GpuContext> context;
    };
} // Ajiva::Renderer
