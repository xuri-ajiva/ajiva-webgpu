//
// Created by XuriAjiva on 27.07.2023.
//

#include "GpuContext.h"

#include "magic_enum.hpp"
#include "Core/Logger.h"

namespace Ajiva::Renderer {
    bool GpuContext::Init(const std::function<wgpu::Surface(wgpu::Instance)> &createSurface) {
        instance = CreateScope<wgpu::Instance>(createInstance(wgpu::InstanceDescriptor{}));
        if (!instance) {
            AJ_FAIL("Could not initialize WebGPU!");
            return false;
        }
        PLOG_INFO << "WebGPU instance: " << instance.get();
        surface = CreateScope<wgpu::Surface>(createSurface(*instance));

        PLOG_INFO << "Requesting adapter...";
        wgpu::RequestAdapterOptions adapterOpts;
        adapterOpts.compatibleSurface = *surface;
        adapter = CreateScope<wgpu::Adapter>(instance->requestAdapter(adapterOpts));
        PLOG_INFO << "Got adapter: " << adapter.get();
        wgpu::SupportedLimits supportedLimits;
        adapter->getLimits(&supportedLimits);

        PLOG_INFO << "Requesting device...";
        // Don't forget to = Default
        wgpu::RequiredLimits requiredLimits = wgpu::Default;
/*
            // We use at most 1 vertex attribute for now
            requiredLimits.limits.maxVertexAttributes = 10;
            // We should also tell that we use 1 vertex buffers
            requiredLimits.limits.maxVertexBuffers = 1;
            // Maximum size of a buffer is 6 vertices of 2 float each
            requiredLimits.limits.maxBufferSize = 60 * 5 * sizeof(float);
            // Maximum stride between 2 consecutive vertices in the vertex buffer
            requiredLimits.limits.maxVertexBufferArrayStride = sizeof(Vertex);
            // This must be set even if we do not use storage buffers for now
            requiredLimits.limits.minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment;
            // This must be set even if we do not use uniform buffers for now
            requiredLimits.limits.minUniformBufferOffsetAlignment = supportedLimits.limits.minUniformBufferOffsetAlignment;
            requiredLimits.limits.maxInterStageShaderComponents = 10;*/
        //DON'T CARE JUST GIVE ALL
        requiredLimits.limits = supportedLimits.limits;

        wgpu::DeviceDescriptor deviceDesc;
        deviceDesc.label = "My Device"; // anything works here, that's your call
        deviceDesc.requiredFeaturesCount = 0; // we do not require any specific feature
        deviceDesc.requiredLimits = &requiredLimits;
        deviceDesc.defaultQueue.label = "The default queue";
        device = CreateScope<wgpu::Device>(adapter->requestDevice(deviceDesc));
        PLOG_INFO << "Got device: " << device.get();

        callback = device->setUncapturedErrorCallback([](wgpu::ErrorType type, char const *message) {
            if (type == WGPUErrorType_NoError)
                PLOG_INFO << magic_enum::enum_name<WGPUErrorType>(type) << ": " << message;
            else {
                PLOG_FATAL << "Uncaptured Error: " << magic_enum::enum_name<WGPUErrorType>(type) << ": ";
                AJ_FAIL(message);
            }
        });

        adapter->getLimits(&supportedLimits);
        PLOG_INFO << "adapter.maxVertexAttributes: " << supportedLimits.limits.maxVertexAttributes;
        device->getLimits(&supportedLimits);
        PLOG_INFO << "device.maxVertexAttributes: " << supportedLimits.limits.maxVertexAttributes;

        queue = CreateScope<wgpu::Queue>(device->getQueue());
        queue->onSubmittedWorkDone([](wgpu::QueueWorkDoneStatus status) {
            PLOG_VERBOSE
                        << "QueueWorkDoneStatus: " << magic_enum::enum_name<WGPUQueueWorkDoneStatus>(status).data();
        });

        //swapChainFormat = surface->getPreferredFormat(*adapter);
        if (swapChainFormat == wgpu::TextureFormat::Undefined)
            swapChainFormat = wgpu::TextureFormat::BGRA8Unorm;
        PLOG_INFO << "SwapChainFormat: " << magic_enum::enum_name<WGPUTextureFormat>(swapChainFormat).data();

        return true;
    }

    GpuContext::~GpuContext() {
        queue.reset();
        device.reset();
        adapter.reset();
        instance.reset();
    }

    Ref<wgpu::SwapChain> GpuContext::CreateSwapChain(int width, int height) const {
        wgpu::SwapChainDescriptor swapChainDesc;
        swapChainDesc.width = width;
        swapChainDesc.height = height;
        swapChainDesc.format = swapChainFormat;
        swapChainDesc.presentMode = wgpu::PresentMode::Fifo;
        swapChainDesc.usage = wgpu::TextureUsage::RenderAttachment;

        wgpu::SwapChain swapChain = device->createSwapChain(*surface, swapChainDesc);
        PLOG_INFO << "Created swap chain: " << &swapChain;
        return CreateScope<wgpu::SwapChain>(swapChain);
    }

    wgpu::CommandEncoder GpuContext::CreateCommandEncoder(const char *label) const {
        wgpu::CommandEncoderDescriptor commandEncoderDesc;
        commandEncoderDesc.label = label;
        return device->createCommandEncoder(commandEncoderDesc);
    }

    wgpu::RenderPassEncoder
    GpuContext::CreateRenderPassEncoder(wgpu::CommandEncoder &encoder, wgpu::TextureView &textureView,
                                        wgpu::TextureView depthTextureView, wgpu::Color clearColor) {
        wgpu::RenderPassDescriptor renderPassDesc{};
        wgpu::RenderPassColorAttachment renderPassColorAttachment;
        renderPassColorAttachment.view = textureView;
        renderPassColorAttachment.resolveTarget = nullptr;
        renderPassColorAttachment.loadOp = wgpu::LoadOp::Clear;
        renderPassColorAttachment.storeOp = wgpu::StoreOp::Store;
        renderPassColorAttachment.clearValue = clearColor;
        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &renderPassColorAttachment;

        // We now add a depth/stencil attachment:
        wgpu::RenderPassDepthStencilAttachment depthStencilAttachment;
        // The view of the depth texture
        depthStencilAttachment.view = depthTextureView;

        // The initial value of the depth buffer, meaning "far"
        depthStencilAttachment.depthClearValue = 1.0f;
        // Operation settings comparable to the color attachment
        depthStencilAttachment.depthLoadOp = wgpu::LoadOp::Clear;
        depthStencilAttachment.depthStoreOp = wgpu::StoreOp::Store;
        // we could turn off writing to the depth buffer globally here
        depthStencilAttachment.depthReadOnly = false;

        // Stencil setup, mandatory but unused
        depthStencilAttachment.stencilClearValue = 0;
#ifdef WEBGPU_BACKEND_WGPU
        depthStencilAttachment.stencilLoadOp = wgpu::LoadOp::Clear;
        depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Store;
#else
        depthStencilAttachment.stencilLoadOp = LoadOp::Undefined;
        depthStencilAttachment.stencilStoreOp = StoreOp::Undefined;
#endif
        depthStencilAttachment.stencilReadOnly = true;

        renderPassDesc.depthStencilAttachment = &depthStencilAttachment;

        renderPassDesc.timestampWriteCount = 0;
        renderPassDesc.timestampWrites = nullptr;
        return encoder.beginRenderPass(renderPassDesc);
    }

    void GpuContext::SubmitCommandBuffer(wgpu::CommandBuffer &commandBuffer) const {
        queue->submit(1, &commandBuffer);
    }

    void GpuContext::SubmitCommandBuffers(std::vector<wgpu::CommandBuffer> &commandBuffers) const {
        queue->submit(commandBuffers.size(), commandBuffers.data());
    }

    void GpuContext::SubmitEncoder(wgpu::CommandEncoder &encoder, const char *label) const {
        wgpu::CommandBufferDescriptor cmdBufferDescriptor;
        cmdBufferDescriptor.label = label;
        wgpu::CommandBuffer command = encoder.finish(cmdBufferDescriptor);
        SubmitCommandBuffer(command);
    }

    Ref<wgpu::ShaderModule> GpuContext::CreateShaderModuleFromCode(const std::string &code) const {
        wgpu::ShaderModuleWGSLDescriptor shaderCodeDesc;
        shaderCodeDesc.chain.next = nullptr;
        shaderCodeDesc.chain.sType = wgpu::SType::ShaderModuleWGSLDescriptor;
        shaderCodeDesc.code = code.c_str();

        wgpu::ShaderModuleDescriptor shaderDesc;
        shaderDesc.hintCount = 0;
        shaderDesc.hints = nullptr;
        shaderDesc.nextInChain = &shaderCodeDesc.chain;

        wgpu::ShaderModule shaderModule = device->createShaderModule(shaderDesc);
        PLOG_INFO << "Created shader module: " << &shaderModule;
        return CreateScope<wgpu::ShaderModule>(shaderModule);
    }

    Ref<wgpu::RenderPipeline> GpuContext::CreateRenderPipeline(const Ref<wgpu::ShaderModule> &shaderModule,
                                                               const std::vector<wgpu::BindGroupLayout> &bindGroupLayouts,
                                                               const std::vector<wgpu::VertexAttribute> &vertexAttribs,
                                                               const wgpu::TextureFormat depthTextureFormat) const {
        PLOG_INFO << "Creating render pipeline";
        wgpu::RenderPipelineDescriptor pipelineDesc;

        wgpu::VertexBufferLayout vertexBufferLayout;
        // [...] Build vertex buffer layout
        vertexBufferLayout.attributeCount = vertexAttribs.size();
        vertexBufferLayout.attributes = vertexAttribs.data();
        // == Common to attributes from the same buffer ==
        vertexBufferLayout.arrayStride = sizeof(VertexData);
        vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;

        pipelineDesc.vertex.bufferCount = 1;
        pipelineDesc.vertex.buffers = &vertexBufferLayout;

        // Vertex shader
        pipelineDesc.vertex.module = *shaderModule;
        pipelineDesc.vertex.entryPoint = "vs_main";
        pipelineDesc.vertex.constantCount = 0;
        pipelineDesc.vertex.constants = nullptr;

        pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        pipelineDesc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;
        pipelineDesc.primitive.frontFace = wgpu::FrontFace::CCW;
        pipelineDesc.primitive.cullMode = wgpu::CullMode::None;

        // Fragment shader
        wgpu::FragmentState fragmentState;
        pipelineDesc.fragment = &fragmentState;
        fragmentState.module = *shaderModule;
        fragmentState.entryPoint = "fs_main";
        fragmentState.constantCount = 0;
        fragmentState.constants = nullptr;

        // Configure blend state
        wgpu::BlendState blendState;
        // Usual alpha blending for the color:
        blendState.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
        blendState.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
        blendState.color.operation = wgpu::BlendOperation::Add;
        // We leave the target alpha untouched:
        blendState.alpha.srcFactor = wgpu::BlendFactor::Zero;
        blendState.alpha.dstFactor = wgpu::BlendFactor::One;
        blendState.alpha.operation = wgpu::BlendOperation::Add;

        wgpu::ColorTargetState colorTarget;
        colorTarget.format = swapChainFormat;
        colorTarget.blend = &blendState;
        colorTarget.writeMask = wgpu::ColorWriteMask::All; // We could write to only some of the color channels.

        // We have only one target because our render pass has only one output color
        // attachment.
        fragmentState.targetCount = 1;
        fragmentState.targets = &colorTarget;

        // Depth and stencil tests are not used here
        wgpu::DepthStencilState depthStencilState = wgpu::Default;
        depthStencilState.depthCompare = wgpu::CompareFunction::Less;
        depthStencilState.depthWriteEnabled = true;
        depthStencilState.format = depthTextureFormat;
        depthStencilState.stencilReadMask = 0;
        depthStencilState.stencilWriteMask = 0;

        pipelineDesc.depthStencil = &depthStencilState;

        // Multi-sampling
        // Samples per pixel
        pipelineDesc.multisample.count = 1;
        // Default value for the mask, meaning "all bits on"
        pipelineDesc.multisample.mask = ~0u;
        // Default value as well (irrelevant for count = 1 anyways)
        pipelineDesc.multisample.alphaToCoverageEnabled = false;

        // Create the pipeline layout
        wgpu::PipelineLayoutDescriptor layoutDesc{};
        layoutDesc.bindGroupLayoutCount = bindGroupLayouts.size();
        layoutDesc.bindGroupLayouts = (WGPUBindGroupLayout *) bindGroupLayouts.data();
        wgpu::PipelineLayout layout = device->createPipelineLayout(layoutDesc);
        pipelineDesc.layout = layout;

        wgpu::RenderPipeline pipeline = device->createRenderPipeline(pipelineDesc);
        PLOG_INFO << "Render pipeline: " << pipeline;
        return CreateScope<wgpu::RenderPipeline>(pipeline);
    }

    Ref<Ajiva::Renderer::Texture>
    GpuContext::CreateTexture(const WGPUTextureFormat &textureFormat, const WGPUExtent3D &textureSize,
                              wgpu::TextureUsage usage, wgpu::TextureAspect textureAspect, uint32_t mipLevelCount,
                              const char *label) const {
        wgpu::TextureDescriptor textureDesc;
        textureDesc.dimension = textureSize.depthOrArrayLayers > 1 ? wgpu::TextureDimension::_3D
                                                                   : wgpu::TextureDimension::_2D; // todo check for array?
        textureDesc.format = textureFormat;
        textureDesc.mipLevelCount = mipLevelCount;
        textureDesc.sampleCount = 1;
        textureDesc.size = textureSize;
        textureDesc.usage = usage;
        textureDesc.viewFormatCount = 1;
        textureDesc.viewFormats = &textureFormat;
        textureDesc.label = label;
        auto texture = device->createTexture(textureDesc);
        wgpu::TextureViewDescriptor textureViewDesc;
        textureViewDesc.aspect = textureAspect;
        textureViewDesc.baseArrayLayer = 0;
        textureViewDesc.arrayLayerCount = 1;
        textureViewDesc.baseMipLevel = 0;
        textureViewDesc.mipLevelCount = mipLevelCount;
        textureViewDesc.dimension = textureSize.depthOrArrayLayers > 1 ? wgpu::TextureViewDimension::_3D
                                                                       : wgpu::TextureViewDimension::_2D; // todo check for array?
        textureViewDesc.format = textureFormat;
        wgpu::TextureView textureView = texture.createView(textureViewDesc);
        PLOG_INFO << "Texture(" << textureFormat << "): " << texture;
        return CreateRef<Ajiva::Renderer::Texture>(texture, textureView, queue, textureFormat, textureAspect,
                                                   textureSize);
    }

    Ref<Ajiva::Renderer::Texture> GpuContext::CreateDepthTexture(const WGPUExtent3D &textureSize) {
        return CreateTexture(depthTextureFormat, textureSize, wgpu::TextureUsage::RenderAttachment,
                             wgpu::TextureAspect::DepthOnly, 1, "DepthTexture");
    }

    Ref<wgpu::BindGroupLayout> GpuContext::CreateBindGroupLayout(std::vector<wgpu::BindGroupLayoutEntry> entries) {
        // Create a bind group layout
        wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc;
        bindGroupLayoutDesc.entryCount = entries.size();
        bindGroupLayoutDesc.entries = entries.data();
        wgpu::BindGroupLayout bindGroupLayout = device->createBindGroupLayout(bindGroupLayoutDesc);
        PLOG_INFO << "Bind group layout: " << bindGroupLayout;
        return CreateRef<wgpu::BindGroupLayout>(bindGroupLayout);
    }

    Ref<wgpu::BindGroup>
    GpuContext::CreateBindGroup(const Ref<wgpu::BindGroupLayout> &bindGroupLayout,
                                std::vector<wgpu::BindGroupEntry> bindings) const {
        PLOG_INFO << "Creating bind group";
        // A bind group contains one or multiple bindings
        wgpu::BindGroupDescriptor bindGroupDesc;
        bindGroupDesc.layout = *bindGroupLayout;
        // There must be as many bindings as declared in the layout!
        bindGroupDesc.entryCount = bindings.size(); //(bindGroupLayoutDesc.entryCount)
        bindGroupDesc.entries = bindings.data();
        wgpu::BindGroup bindGroup = device->createBindGroup(bindGroupDesc);
        PLOG_INFO << "Bind group: " << bindGroup;
        return CreateRef<wgpu::BindGroup>(bindGroup);
    }

    Ref<Ajiva::Renderer::Buffer>
    GpuContext::CreateBuffer(uint64_t size, WGPUBufferUsageFlags usage, const char *label) const {
        wgpu::BufferDescriptor bufferDesc;
        bufferDesc.label = label;
        bufferDesc.usage = usage;
        bufferDesc.size = ALIGN_AT(size, 4);
        bufferDesc.mappedAtCreation = false;
        auto buffer = device->createBuffer(bufferDesc);
        PLOG_VERBOSE << "Buffer: " << buffer;
        return CreateRef<Ajiva::Renderer::Buffer>(buffer, size, bufferDesc.size, queue);
    }

    Ref<Ajiva::Renderer::Buffer>
    GpuContext::CreateFilledBuffer(const void *data, uint64_t size, WGPUBufferUsageFlags usage,
                                   const char *label) const {
        auto buffer = CreateBuffer(size, usage, label);
        buffer->UpdateBufferData(data, size);
        return buffer;
    }

    Ref<wgpu::Sampler> GpuContext::CreateSampler(wgpu::AddressMode addressMode, wgpu::FilterMode filterMode,
                                                 wgpu::CompareFunction compareFunction, float lodMinClamp,
                                                 float lodMaxClamp, const char *label) const {
        wgpu::SamplerDescriptor samplerDesc;
        samplerDesc.addressModeU = addressMode;
        samplerDesc.addressModeV = addressMode;
        samplerDesc.addressModeW = addressMode;
        samplerDesc.magFilter = filterMode;
        samplerDesc.minFilter = filterMode;
        samplerDesc.mipmapFilter = static_cast<WGPUMipmapFilterMode>((WGPUFilterMode) filterMode);
        samplerDesc.lodMinClamp = lodMinClamp;
        samplerDesc.lodMaxClamp = lodMaxClamp;
        samplerDesc.compare = compareFunction;
        samplerDesc.maxAnisotropy = 1;
        samplerDesc.label = label;
        auto sampler = device->createSampler(samplerDesc);
        PLOG_INFO << "Sampler: " << sampler;
        return CreateRef<wgpu::Sampler>(sampler);
    }

    GpuContext::GpuContext() {

    }


}