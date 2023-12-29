//
// Created by XuriAjiva on 22.08.2023.
//

#pragma once

#include "webgpu/webgpu.hpp"
#include "Core/Layer.h"
#include "Renderer/GpuContext.h"
#include "Resource/Loader.h"
#include "Renderer/BindGroupBuilder.h"
#include "GraphicsResourceManager.h"
#include "Model.h"

namespace Ajiva
{
    class Application;
}

namespace Ajiva::Renderer
{
    using namespace glm;

    class RenderPipelineLayer : public Core::Layer
    {
        friend Ajiva::Application;

    public:
        RenderPipelineLayer() = default;

        RenderPipelineLayer(const Ref<Renderer::GpuContext>& context, const Ref<Resource::Loader>& loader,
                            Ref<Renderer::GraphicsResourceManager> graphicsResourceManager,
                            std::function<mat4x4()> viewMatrix, std::function<mat4x4()> projectionMatrix,
                            std::function<vec3()> worldPos)
            : Layer("RenderPipeline"), context(context), loader(loader),
              graphicsResourceManager(std::move(graphicsResourceManager)),
              viewMatrix(std::move(viewMatrix)), projectionMatrix(std::move(projectionMatrix)),
              worldPos(std::move(worldPos))
        {
            bindGroupBuilder = BindGroupBuilder(context, loader);
            instanceModelManager = CreateRef<InstanceModelManager>(context);
        }

        ~RenderPipelineLayer() override = default;


        bool Attached() override;

        void Detached() override;


        void CheckTarget(Core::RenderTarget target);

        void CreateDepthTexture(const Extent3D& size)
        {
            if (size.width < 1 || size.height < 1)
            {
                PLOG_WARNING << "Depth Texture size is 0!";
                return;
            }
            depthTexture = context->CreateDepthTexture(size);
            PLOG_INFO << "Depth Texture " << depthTexture->texture << " " << depthTexture->view << " "
                      << depthTexture->size.width << "x" << depthTexture->size.height;
        }

        void Render(Core::UpdateInfo frameInfo, Core::RenderTarget target) override;

        void Update(Core::UpdateInfo frameInfo) override;

        Ajiva::Renderer::LightningUniform lightningUniform = {};

    private:
        Ref<Renderer::GpuContext> context;
        Ref<Resource::Loader> loader; //todo Remove
        Ref<Renderer::GraphicsResourceManager> graphicsResourceManager;

        Renderer::BindGroupBuilder bindGroupBuilder;
        std::function<mat4x4()> viewMatrix;
        std::function<mat4x4()> projectionMatrix;
        std::function<vec3()> worldPos;


        Ref<Ajiva::Renderer::Texture> depthTexture = nullptr;
        Ref<wgpu::RenderPipeline> renderPipeline = nullptr;

        Ajiva::Renderer::UniformData uniforms = {};

        Ref<Ajiva::Renderer::Buffer> uniformBuffer = nullptr;
        Ref<Ajiva::Renderer::Buffer> lightningUniformBuffer = nullptr;

        std::vector<Ref<ModelInstance>> modelInstances;

        Ref<InstanceModelManager> instanceModelManager = nullptr;
    };
} // PBR
