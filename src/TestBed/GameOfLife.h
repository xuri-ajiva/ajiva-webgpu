//
// Created by XuriAjiva on 28.04.2024.
//
#pragma once

#include "defines.h"
#include "Platform/Window.h"
#include "Renderer/GpuContext.h"
#include "Resource/Loader.h"
#include "Core/Clock.h"
#include "Core/EventSystem.h"
#include "Renderer/Camera.h"
#include "Core/Layer.h"
#include "Renderer/BindGroupBuilder.h"
#include "Core/ThreadPool.h"
#include "Renderer/GraphicsResourceManager.h"
#include "Resource/FilesNames.hpp"
#include "imgui.h"
#include "Renderer/RenderPipelineLayer.h"
#include <array>

//include random for random distribution
#include <random>

namespace Ajiva {

    class GameOfLife : public Core::Layer {
        Ref<Renderer::GpuContext> context;
        Ref<Core::EventSystem> eventSystem;
        Ref<Platform::Window> window;
        Ref<Resource::Loader> loader;
        Ref<Renderer::RenderPipelineLayer> pipelineLayer;
        constexpr static u32 dimension = 1024;
        std::array<f32, dimension * dimension> data{};
        std::vector<u8> data2;
        //Renderer::BindGroupBuilder bindGroupBuilder;
        WGPUBindGroup bindGroup{};
        Scope<wgpu::BufferMapCallback> callback;
        bool running = false;
        bool dirty = true;
        Scope<wgpu::Buffer> resultBuffer;
    public:
        GameOfLife(const Ref<Renderer::GpuContext> &context,
                   const Ref<Core::EventSystem> &eventSystem,
                   const Ref<Platform::Window> &window,
                   const Ref<Resource::Loader> &loader,
                   const Ref<Renderer::RenderPipelineLayer> &pipelineLayer
        ) : Layer("Game Of Life"),
            context(context),
            eventSystem(eventSystem),
            window(window), loader(loader),
            pipelineLayer(pipelineLayer) {}

        Scope<wgpu::Buffer> outputBuffer;
        Scope<wgpu::Buffer> inputBuffer;
        Ref<wgpu::ShaderModule> shaderModule;
        Scope<wgpu::ComputePipeline> pipeline;
        //Ref<Renderer::Texture> texture;


        std::uniform_real_distribution<f32> dist;
        std::mt19937 engine;
        bool Attached() override {
            using namespace wgpu;
            Layer::Attached();

            //create random distribution
            dist = std::uniform_real_distribution<f32>(0.0, 1.0);
            engine = std::mt19937(std::random_device{}());

            for (int i = 0; i < dimension; ++i) {
                for (int j = 0; j < dimension; ++j) {
                    data[i * dimension + j] = dist(engine);
                }
            }

            {
                texture = context->CreateTexture(
                        TextureFormat::RGBA8Unorm, wgpu::Extent3D{dimension, dimension, 1},
                        static_cast<const WGPUTextureUsage>(TextureUsage::TextureBinding |
                                                            TextureUsage::StorageBinding |
                                                            TextureUsage::RenderAttachment |
                                                            TextureUsage::CopyDst | TextureUsage::CopySrc),
                        TextureAspect::All, 1, "GOL Texture");

                shaderModule = context->CreateShaderModuleFromCode(loader->LoadFile(Ajiva::Resource::Files::gol_wgsl));

                pipeline = CreateScope<wgpu::ComputePipeline>(context->device->createComputePipeline(
                        WGPUComputePipelineDescriptor{
                                .label = "Compute Pipeline",
                                .compute = WGPUProgrammableStageDescriptor{
                                        .module = *shaderModule,
                                        .entryPoint = "compute"
                                },
                        }
                ));
                inputBuffer = CreateScope<wgpu::Buffer>(context->device->createBuffer(
                        WGPUBufferDescriptor{
                                .label = "GOL InputBuffer",
                                .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst,
                                .size = data.size(),
                        }));

                context->queue->writeBuffer(*inputBuffer, 0, data.data(), data.size());

                outputBuffer = CreateScope<wgpu::Buffer>(context->device->createBuffer(
                        WGPUBufferDescriptor{.label =  "GOL outputBuffer",
                                .usage = BufferUsage::Storage | wgpu::BufferUsage::CopySrc,
                                .size = data.size(),
                        }));

                resultBuffer = CreateScope<wgpu::Buffer>(context->device->createBuffer(
                        WGPUBufferDescriptor{.label =  "result inputBuffer",
                                .usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst,
                                .size = data.size(),
                        }));

                std::vector<BindGroupEntry> entries(3, Default);

                // Input buffer
                entries[0].binding = 0;
                entries[0].buffer = *inputBuffer;
                entries[0].offset = 0;
                entries[0].size = data.size();

                // Output buffer
                entries[1].binding = 1;
                entries[1].buffer = *outputBuffer;
                entries[1].offset = 0;
                entries[1].size = data.size();

                entries[2].binding = 2;
                entries[2].textureView = texture->view;


                //bindGroupBuilder.PushBuffer(inputBuffer, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage);
                bindGroup = context->device->createBindGroup(WGPUBindGroupDescriptor{
                        .label = "GOL Bind group",
                        .layout = pipeline->getBindGroupLayout(0),
                        .entryCount = (uint32_t) entries.size(),
                        .entries = entries.data(),
                });

            }


            return true;
        }

        void Detached() override {
            inputBuffer.reset();
            Layer::Detached();
        }

        void Render(Core::UpdateInfo frameInfo, Core::RenderTarget target) override {
            Layer::Render(frameInfo, target);

            static bool pOpen = true;
            if (!pOpen) return;
            if (!ImGui::Begin("Game of Life", &pOpen)) {
                ImGui::End();
                return;
            }


            if (ImGui::Button("Step")) {
                running = !running;
            }

            if (ImGui::Button("New Data")) {
                for (int i = 0; i < dimension; ++i) {
                    for (int j = 0; j < dimension; ++j) {
                        data[i * dimension + j] = dist(engine);
                    }
                }

                context->queue->writeBuffer(*inputBuffer, 0, data.data(), data.size());
            }

            if (running) {
                auto encoder = context->CreateCommandEncoder("GOL Command Encoder");

                auto pass = encoder.beginComputePass(WGPUComputePassDescriptor{
                        .label = "GOL Compute Pass",
                });
                pass.setPipeline(*pipeline);
                pass.setBindGroup(0, bindGroup, 0, nullptr);
                pass.dispatchWorkgroups(dimension, 1, 1);
                pass.end();

                //encoder.copyBufferToBuffer(*outputBuffer, 0, *resultBuffer, 0, data.size());
                encoder.copyBufferToBuffer(*outputBuffer, 0, *inputBuffer, 0, data.size());
                context->SubmitEncoder(encoder);
                //callback = inputBuffer->CopyTo(data.data(), 1024 * 1024 * sizeof(u8));
                /*callback = resultBuffer->mapAsync(wgpu::MapMode::Read, 0, data.size(),
                                                  [&](wgpu::BufferMapAsyncStatus status) {
                                                      if (status == wgpu::BufferMapAsyncStatus::Success) {
                                                          auto from = resultBuffer->getConstMappedRange(0, data.size());
                                                          std::memcpy(data.data(), from, data.size());
                                                          resultBuffer->unmap();
                                                          for (int i = 0; i < 10; ++i) {
                                                              for (int j = 0; j < 10; ++j) {
                                                                  std::cout << (data[i * 1024 + j] > 0.1 ? "X" : " ");
                                                              }
                                                              printf("\n");
                                                          }
                                                      }
                                                      running = false;
                                                      dirty = true;
                                                  });*/

                if (dirty) {
                    pipelineLayer->textureDiff->SwapBackingTexture(texture);
                    dirty = false;
                }
            }
/*            while (running) {
#ifdef WEBGPU_BACKEND_WGPU
                context->queue->submit(0, nullptr);
#else
                m_instance.processEvents();
#endif
            }*/

/*            if (dirty) {
                // update pipelineLayer->texture
*//*                auto &t = pipelineLayer->textureDiff;
                data2.clear();
                data2.reserve(t->size.width * t->size.height * 4);
                for (int x = 0; x < t->size.width; ++x) {
                    for (int y = 0; y < t->size.height; ++y) {
                        auto index =
                                i32((f32(x) / f32(t->size.width)) * 1024) * 1024 +
                                i32((f32(y) / f32(t->size.height)) * 1024);
                        auto val = u8(data[index] > 0.1 ? 255 : 0);
                        data2.push_back(val);
                        data2.push_back(val);
                        data2.push_back(val);
                        data2.push_back(255);
                    }
                }
                t->WriteTextureMips(data2.data(), data2.size(), 8);*//*
                //pipelineLayer->textureDiff->SwapBackingTexture(texture);
                dirty = false;
            }*/

            //ImGui::Image(pipelineLayer->textureDiff->texture, ImVec2(pipelineLayer->textureDiff->size.width, pipelineLayer->textureDiff->size.height));
            //ImGui::Image(texture->texture, ImVec2(dimension, dimension));

            ImGui::End();
        }

        void Update(Core::UpdateInfo frameInfo) override {
            Layer::Update(frameInfo);
        }


        Ref<Renderer::Texture> texture;
    };


}
