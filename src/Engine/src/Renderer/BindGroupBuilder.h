//
// Created by XuriAjiva on 20.08.2023.
//

#pragma once


#include <utility>
#include <vector>
#include "webgpu/webgpu.hpp"
#include "defines.h"
#include "Texture.h"
#include "Buffer.h"
#include "GpuContext.h"
#include "Resource/Loader.h"

namespace Ajiva::Renderer
{
    using namespace wgpu;

    struct BindGroupBuilder
    {
    public:
        BindGroupBuilder() = default;

        BindGroupBuilder(Ref<Renderer::GpuContext> context, Ref<Resource::Loader> loader);

        void PushTexture(const Ref<Renderer::Texture>& texture);

        void PushSampler(const Ref<Renderer::Sampler>& sampler);

        void PushBuffer(const Ref<Renderer::Buffer>& buffer,
                        ShaderStage visibility = static_cast<WGPUShaderStage>(WGPUShaderStage_Vertex |
                            WGPUShaderStage_Fragment),
                        BufferBindingType type = BufferBindingType::Uniform);

        void BuildBindGroupLayout();

        void UpdateBindings();

        Ref<wgpu::BindGroup> bindGroup = nullptr;
        Ref<wgpu::BindGroupLayout> bindGroupLayout = nullptr;

    private:
        Ref<Renderer::GpuContext> context;
        Ref<Resource::Loader> loader;

        std::vector<wgpu::BindGroupLayoutEntry> bindingLayoutEntries;
        std::vector<wgpu::BindGroupEntry> bindings;
        std::vector<Ref<Renderer::Buffer>> uniformBuffers;
        std::vector<Ref<Renderer::Texture>> textures;
        std::vector<u64> textureVersions;
        std::vector<Ref<Renderer::Sampler>> samplers;
    };
} // Ajiva Renderer
