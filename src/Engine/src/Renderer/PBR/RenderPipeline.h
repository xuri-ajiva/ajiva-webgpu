//
// Created by XuriAjiva on 22.08.2023.
//

#pragma once

#include "webgpu/webgpu.hpp"
#include "Core/Layer.h"
#include "Renderer/GpuContext.h"
#include "Resource/Loader.h"
#include "Resource/FilesNames.hpp"
#include "Renderer/BindGroupBuilder.h"

namespace Ajiva::Renderer::PBR {
    using namespace glm;

    struct Vertex {
        vec3 position;
        vec3 color;
        vec3 normal;
        vec3 tangent;
        vec3 bitangent;
        vec2 uv;
        f32 padding[3];

        std::vector<wgpu::VertexAttribute> GetVertexAttributes() {
            return {
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
        }
    };

    static_assert(sizeof(Vertex) % 16 == 0);

    struct MaterialProperties {
        vec3 baseColor;
        f32 roughness;
        f32 metallic;
        f32 reflectance;
        f32 normalScale;
        f32 ao;
        vec3 emissive;
        u32 highQuality;
    };
    static_assert(sizeof(MaterialProperties) % 16 == 0);


    class RenderPipeline {
    public:
        RenderPipeline() = default;

        RenderPipeline(Ref<Renderer::GpuContext> context, Ref<Resource::Loader> loader)
                : context(context), loader(loader),bindGroupBuilder(context, loader) {
        }

        ~RenderPipeline() = default;


        void Render(wgpu::RenderPassEncoder renderPass, Core::UpdateInfo frameInfo) {

        }

        void Build() {
            shaderModule = context->CreateShaderModuleFromCode(
                    loader->LoadFile(Ajiva::Resource::Files::pbr_shader_wgsl));

            uniformBuffer = context->CreateFilledBuffer(&uniformData, sizeof(UniformData),
                                                        wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst,
                                                        "PBR::RenderPipeline::uniformBuffer");

            auto sampler = context->CreateSampler(wgpu::AddressMode::Repeat, wgpu::FilterMode::Linear,
                                                  wgpu::CompareFunction::Undefined, 0.0f, 32.0f,
                                                  "Sampler");


            bindGroupBuilder.PushBuffer(uniformBuffer);
            bindGroupBuilder.PushSampler(sampler);
            bindGroupBuilder.BuildBindGroupLayout();
        }


    private:
        Ref<Renderer::GpuContext> context;
        Ref<Resource::Loader> loader;

        Ref<wgpu::ShaderModule> shaderModule;

        UniformData uniformData;
        Ref<Buffer> uniformBuffer;
        MaterialProperties materials[1024];
        Ref<Buffer> materialBuffer;
        Renderer::BindGroupBuilder bindGroupBuilder;
    };
} // PBR
