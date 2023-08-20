//
// Created by XuriAjiva on 20.08.2023.
//

#include "BindGroupBuilder.h"


namespace Ajiva::Renderer {
    void BindGroupBuilder::PushTexture(const Ref<Texture> &texture) {
        textures.push_back(texture);

        auto bindingIndex = (uint32_t) bindingLayoutEntries.size();
        wgpu::BindGroupLayoutEntry bindingLayout = wgpu::Default;
        bindingLayout.binding = bindingIndex;
        bindingLayout.visibility = ShaderStage::Fragment;
        bindingLayout.texture.sampleType = TextureSampleType::Float;
        bindingLayout.texture.viewDimension = TextureViewDimension::_2D;
        bindingLayoutEntries.push_back(bindingLayout);

        BindGroupEntry binding = Default;
        binding.binding = bindingIndex;
        binding.textureView = texture->view;
        bindings.push_back(binding);
    }

    void BindGroupBuilder::PushSampler(const Ref<Sampler> &sampler) {
        samplers.push_back(sampler);

        auto bindingIndex = (uint32_t) bindingLayoutEntries.size();
        wgpu::BindGroupLayoutEntry bindingLayout = wgpu::Default;
        bindingLayout.binding = bindingIndex;
        bindingLayout.visibility = ShaderStage::Fragment;
        bindingLayout.sampler.type = SamplerBindingType::Filtering;
        bindingLayoutEntries.push_back(bindingLayout);

        BindGroupEntry binding = Default;
        binding.binding = bindingIndex;
        binding.sampler = *sampler;
        bindings.push_back(binding);
    }

    void BindGroupBuilder::PushBuffer(const Ref<Buffer> &buffer, ShaderStage visibility, BufferBindingType type) {
        uniformBuffers.push_back(buffer);

        auto bindingIndex = (uint32_t) bindingLayoutEntries.size();
        wgpu::BindGroupLayoutEntry bindingLayout = wgpu::Default;
        bindingLayout.binding = bindingIndex;
        bindingLayout.visibility = visibility;
        bindingLayout.buffer.type = type;
        bindingLayout.buffer.hasDynamicOffset = false;
        bindingLayout.buffer.minBindingSize = 0;
        bindingLayoutEntries.push_back(bindingLayout);

        BindGroupEntry binding = Default;
        binding.binding = bindingIndex;
        binding.buffer = buffer->buffer;
        binding.size = buffer->alignedSize;
        bindings.push_back(binding);
    }

    std::shared_ptr<BindGroupLayout> BindGroupBuilder::BuildBindGroupLayout() {
        if (bindGroup) {
            PLOG_WARNING << "BindGroup already build for: " << this;
        }

        PLOG_INFO << "Build BindGroupLayout for: " << this;
        for (int i = 0; i < bindings.size(); ++i) {
            auto &entry = bindings[i];
            if (entry.binding != i) {
                PLOG_ERROR << "Binding: " << i << " is not equal to " << entry.binding;
            }
            if (entry.buffer)
                PLOG_INFO << "\tBinding: " << i << " is Buffer " << entry.buffer;
            else if (entry.sampler)
                PLOG_INFO << "\tBinding: " << i << " is Sampler " << entry.sampler;
            else if (entry.textureView)
                PLOG_INFO << "\tBinding: " << i << " is TextureView " << entry.textureView;
            else PLOG_INFO << "\tBinding: " << i << " is Empty";
        }

        auto bindGroupLayout = context->CreateBindGroupLayout(bindingLayoutEntries);

        bindGroup = context->CreateBindGroup(bindGroupLayout, bindings);

        return bindGroupLayout;
    }

    BindGroupBuilder::BindGroupBuilder(Ref<Renderer::GpuContext> context, Ref<Resource::Loader> loader) : context(
            std::move(context)), loader(std::move(loader)) {
    }
} // Ajiva