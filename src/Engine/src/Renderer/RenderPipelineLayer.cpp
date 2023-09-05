//
// Created by XuriAjiva on 04.09.2023.
//

#include "RenderPipelineLayer.h"

#include "Resource/FilesNames.hpp"
#include "glm/ext.hpp"
#include "imgui.h"
#include <random>

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
        auto plane = graphicsResourceManager->GetModel(Ajiva::Resource::Files::Objects::cube_obj);

        //auto model = instanceModelManager->CreateInstance(plane);

/*        model-> instanceBuffer = context->CreateFilledBuffer(model->instanceData.data(),
                                                             model->instanceData.size() * sizeof(Ajiva::Renderer::InstanceData),
                                                             wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex,
                                                             "Instance Buffer");*/

        {
            constexpr int NumInstances = 100;
            //instanceData.reserve(NumInstances * NumInstances);
            for (int i = 0; i < NumInstances; ++i) {
                for (int j = 0; j < NumInstances; ++j) {
                    Scope<ModelInstance> data = instanceModelManager->CreateInstance(plane);
/*                    instanceData.push_back(
                            {
                                    .modelMatrix = glm::translate(glm::mat4(1.0f),
                                                                  glm::vec3(i * 2.1f, j * 2.1f, 1.0f)),
                                    .color = glm::vec4(1.0f, 0.0f, 1.0f / static_cast<float>(NumInstances), 1.0f),
                            });*/
                    data->model->instanceData[data->instanceIndex] = {
                            .modelMatrix = glm::translate(glm::mat4(1.0f),
                                                          glm::vec3(i * 2.1f, j * 2.1f, 1.0f)),
                            .color = glm::vec4(1.0f, 0.0f, 1.0f / static_cast<float>(NumInstances), 1.0f),
                    };
                    modelInstances.push_back(std::move(data));
                }
            }
        }
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
        renderPass.setBindGroup(0, *bindGroupBuilder.bindGroup, 0, nullptr);//todo move to mesh/Instance??

        instanceModelManager->Render(renderPass);

        renderPass.end();
        context->SubmitEncoder(encoder);

        if (!ImGui::Begin("Planes")) {
            ImGui::End();
            return;
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> pos(0.0f, 100.0f);
        std::uniform_real_distribution<float> color(0.0f, 1.0f);

        if (ImGui::Button("Randomize Positions")) {
            for (auto &modelInstance: modelInstances) {
                modelInstance->data().modelMatrix = glm::translate(
                        glm::mat4(1.0f),
                        glm::vec3(pos(gen), pos(gen), pos(gen)));
            }
        }
        ImGui::SameLine();
        if(ImGui::Button("Random Rotation")){
            for (auto &modelInstance: modelInstances) {
                modelInstance->data().modelMatrix = glm::rotate(
                        modelInstance->data().modelMatrix,
                        glm::radians(pos(gen)),
                        glm::vec3(pos(gen), pos(gen), pos(gen)));
            }
        }
        ImGui::SameLine();
        if(ImGui::Button("Random Scale")){
            for (auto &modelInstance: modelInstances) {
                auto scale = color(gen);
                modelInstance->data().modelMatrix[0][0] = scale;
                modelInstance->data().modelMatrix[1][1] = scale;
                modelInstance->data().modelMatrix[2][2] = scale;
            }
        }

        if (ImGui::Button("Randomize Colors")) {
            for (auto &modelInstance: modelInstances) {
                modelInstance->data().color = glm::vec4(color(gen), color(gen), color(gen), 1.0f);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Gradient Colors")) {
            for (auto &modelInstance: modelInstances) {
                modelInstance->data().color = glm::vec4(
                        modelInstance->data().modelMatrix[3][0] / 100.0f,
                        modelInstance->data().modelMatrix[3][1] / 100.0f,
                        modelInstance->data().modelMatrix[3][2] / 100.0f,
                        1.0f);
            }
        }

        ImGuiListClipper clipper;

        clipper.Begin(modelInstances.size());
        while (clipper.Step()) {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
                auto &modelInstance = modelInstances[i];
                ImGui::PushID(modelInstance->instanceIndex);
                ImGui::DragFloat3("Position", &modelInstance->data()
                        .modelMatrix[3][0], 0.1f);
                ImGui::ColorEdit3("Color", &modelInstance->data().color[0]);
                ImGui::PopID();
            }
        }
        ImGui::End();

    }

    void Renderer::RenderPipelineLayer::Update(Core::UpdateInfo frameInfo) {
        Layer::Update(frameInfo);
        instanceModelManager->Update();
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