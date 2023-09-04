//
// Created by XuriAjiva on 04.09.2023.
//

#include "RenderPipelineLayer.h"

#include "Resource/FilesNames.hpp"
#include "glm/ext.hpp"

namespace Ajiva::Renderer {
    bool Renderer::RenderPipelineLayer::Attached() {
        Layer::Attached();

        {
            uniforms = {
                    .time = 1.0f,
            };

            lightningUniform = {
                    .lights = {
                            {.position = {0.5f, 0.5f, 1.0f, 0}, .color = {1.0f, 1.0f, 1.0f, 1.0f}},
                            {.position = {1.0f, 1.0f, 1.0f, 0}, .color = {1.0f, 0.0f, 0.0f, 1.0f}},
                            {.position = {1.0f, 0.0f, 1.0f, 0}, .color = {0.0f, 1.0f, 0.0f, 1.0f}},
                            {.position = {0.0f, 1.0f, 1.0f, 0}, .color = {0.0f, 0.0f, 1.0f, 1.0f}},
                    },
                    .ambient = {0.5f, 0.5f, 0.5f, 1.0f},
                    .hardness = 32.0f,
                    .kd = 0.8f,
                    .ks = 0.2f,
            };
        }

        {
            constexpr int NumInstances = 100;
            instanceData.reserve(NumInstances * NumInstances);
            for (int i = 0; i < NumInstances; ++i) {
                for (int j = 0; j < NumInstances; ++j) {
                    instanceData.push_back(
                            {
                                    .modelMatrix = glm::translate(glm::mat4(1.0f),
                                                                  glm::vec3(i * 2.1f, j * 2.1f, 1.0f)),
                                    .color = glm::vec4(1.0f, 0.0f, 1.0f / static_cast<float>(NumInstances), 1.0f),
                            });
                }
            }
        }

        // Vertex fetch
        // We now have 2 attributes
        std::vector<wgpu::VertexAttribute> vertexAttribs = {
                WGPUVertexAttribute{ //todo macro this
                        .format = wgpu::VertexFormat::Float32x3,
                        .offset = offsetof(Ajiva::Renderer::VertexData, position),
                        .shaderLocation = 0,
                },
                WGPUVertexAttribute{
                        .format = wgpu::VertexFormat::Float32x3,
                        .offset = offsetof(Ajiva::Renderer::VertexData, normal),
                        .shaderLocation = 1,
                },
                WGPUVertexAttribute{
                        .format = wgpu::VertexFormat::Float32x3,
                        .offset = offsetof(Ajiva::Renderer::VertexData, color),
                        .shaderLocation = 2,
                },
                WGPUVertexAttribute{
                        .format = wgpu::VertexFormat::Float32x2,
                        .offset = offsetof(Ajiva::Renderer::VertexData, uv),
                        .shaderLocation = 3,
                },
        };

        //todo move to member??? or not
        Ref<wgpu::ShaderModule> shaderModule = context->CreateShaderModuleFromCode(
                loader->LoadFile(Ajiva::Resource::Files::shader_wgsl));

        // Create the depth texture
        // AUTO create if size differs: BuildDepthTexture();

/*    auto depthTexture = context->CreateTexture(TextureFormat::Depth24Plus, {static_cast<uint32_t>(window->GetWidth()),
                                                                           static_cast<uint32_t>(window->GetHeight()),
                                                                           1});*/

        {
            uniformBuffer = context->CreateFilledBuffer(&uniforms, sizeof(Ajiva::Renderer::UniformData),
                                                        wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform,
                                                        "Uniform Buffer");
            bindGroupBuilder.PushBuffer(uniformBuffer);

            const int mipLevelCount = 8;
            auto sampler = context->CreateSampler(wgpu::AddressMode::Repeat, wgpu::FilterMode::Linear,
                                                  wgpu::CompareFunction::Undefined, 0.0f, 1.0f * mipLevelCount,
                                                  "Sampler");
            bindGroupBuilder.PushSampler(sampler);

            auto texture = graphicsResourceManager->GetTexture(
                    Ajiva::Resource::Files::Textures::cobblestone_floor_08_diff_2k_jpg);
            if (!texture) {
                AJ_FAIL("Could not load texture!");
                return false;
            }
            bindGroupBuilder.PushTexture(texture);

            texture = loader->LoadTextureAsync(Ajiva::Resource::Files::Textures::cobblestone_floor_08_nor_gl_2k_png,
                                               *context, mipLevelCount);
            if (!texture) {
                AJ_FAIL("Could not load texture!");
                return false;
            }
            bindGroupBuilder.PushTexture(texture);

            lightningUniformBuffer = context->CreateFilledBuffer(&uniforms,
                                                                 sizeof(Ajiva::Renderer::LightningUniform),
                                                                 wgpu::BufferUsage::CopyDst |
                                                                 wgpu::BufferUsage::Uniform,
                                                                 "Lightning Uniform Buffer");
            bindGroupBuilder.PushBuffer(lightningUniformBuffer);


            bindGroupBuilder.BuildBindGroupLayout();
            CreateDepthTexture({1, 1, 1});
            renderPipeline = context->CreateRenderPipeline(shaderModule,
                                                           std::vector{*bindGroupBuilder.bindGroupLayout},
                                                           vertexAttribs,
                                                           depthTexture->textureFormat);

        }

        bool success = loader->LoadGeometryFromObj(Ajiva::Resource::Files::Objects::plane_obj, vertexData,
                                                   indexData);
        if (!success) {
            std::cerr << "Could not load geometry!" << std::endl;
            return false;
        }


        vertexBuffer = context->CreateFilledBuffer(vertexData.data(),
                                                   vertexData.size() * sizeof(Ajiva::Renderer::VertexData),
                                                   wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex,
                                                   "Vertex Buffer");
        /*auto indexBuffer = context->CreateFilledBuffer(indexData.data(), indexData.size() * sizeof(uint16_t),
                                                      wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index,
                                                      "Index Buffer");*/


        instanceBuffer = context->CreateFilledBuffer(instanceData.data(),
                                                     instanceData.size() * sizeof(Ajiva::Renderer::InstanceData),
                                                     wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex,
                                                     "Instance Buffer");


        return true;
    }

    void Renderer::RenderPipelineLayer::CheckTarget(Core::RenderTarget target) {
        //check if depth texture is big enough
        if (depthTexture->size.width != target.extent.width ||
            depthTexture->size.height != target.extent.height) {
            CreateDepthTexture({target.extent.width, target.extent.height, 1});
        }
    }

    void Renderer::RenderPipelineLayer::Detached() {
        Layer::Detached();
        //TODO
    }

    void Renderer::RenderPipelineLayer::Render(Core::UpdateInfo frameInfo, Core::RenderTarget target) {
        Layer::Render(frameInfo, target);

        CheckTarget(target);


        wgpu::CommandEncoder encoder = context->CreateCommandEncoder();

        wgpu::RenderPassEncoder renderPass = context->CreateRenderPassEncoder(encoder, target.texture,
                                                                              depthTexture->view,
                                                                              {0.4, 0.4, 0.4, 1.0});
        renderPass.setPipeline(*renderPipeline);
        renderPass.setVertexBuffer(0, vertexBuffer->buffer, 0, vertexBuffer->size);
        renderPass.setVertexBuffer(1, instanceBuffer->buffer, 0, instanceBuffer->size);
        /* renderPass.setIndexBuffer(indexBuffer->buffer, wgpu::IndexFormat::Uint16, 0,
                                   indexData.size() * sizeof(uint16_t));*/
        renderPass.setBindGroup(0, *bindGroupBuilder.bindGroup, 0, nullptr);
        //renderPass.drawIndexed(indexData.size(), 1, 0, 0, 0);
        renderPass.draw(vertexBuffer->size / sizeof(Ajiva::Renderer::VertexData), instanceData.size(), 0, 0);

        renderPass.end();

        context->SubmitEncoder(encoder);
    }

    void Renderer::RenderPipelineLayer::Update(Core::UpdateInfo frameInfo) {
        Layer::Update(frameInfo);
        bindGroupBuilder.UpdateBindings();


        uniforms.time = frameInfo.TotalTime;
/*        uniforms.modelMatrix = glm::rotate(mat4x4(1.0), uniforms.time, vec3(0.0, 0.0, 1.0)) *
                               glm::translate(mat4x4(1.0), vec3(0.5, 0.0, 0.0)) *
                               glm::scale(mat4x4(1.0), vec3(0.8f));*/
        uniforms.viewMatrix = viewMatrix();
        uniforms.projectionMatrix = projectionMatrix();
        uniforms.worldPos = worldPos();
        uniformBuffer->UpdateBufferData(&uniforms, sizeof(Ajiva::Renderer::UniformData));

        lightningUniformBuffer->UpdateBufferData(&lightningUniform, sizeof(Ajiva::Renderer::LightningUniform));
    }
}