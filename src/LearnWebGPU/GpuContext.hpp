//
// Created by XuriAjiva on 27.07.2023.
//

#pragma once

#include "webgpu/webgpu.hpp"

using namespace wgpu;
namespace Ajiva {
    class GpuContext {
    public:
        std::unique_ptr<Instance> instance;
        std::unique_ptr<Adapter> adapter;
        std::unique_ptr<Device> device;
        std::unique_ptr<Queue> queue;
        Surface surface = nullptr;

        GpuContext(std::function<wgpu::Surface(wgpu::Instance)> createSurface) {
            instance = std::make_unique<Instance>(createInstance(InstanceDescriptor{}));
            if (!instance) {
                std::cerr << "Could not initialize WebGPU!" << std::endl;
                return;
            }
            std::cout << "WebGPU instance: " << instance << std::endl;
            surface = createSurface(*instance);

            std::cout << "Requesting adapter..." << std::endl;
            RequestAdapterOptions adapterOpts;
            adapterOpts.compatibleSurface = surface;
            adapter = std::make_unique<Adapter>(instance->requestAdapter(adapterOpts));
            std::cout << "Got adapter: " << adapter << std::endl;
            SupportedLimits supportedLimits;
            adapter->getLimits(&supportedLimits);


            std::cout << "Requesting device..." << std::endl;
            // Don't forget to = Default
            RequiredLimits requiredLimits = Default;
            // We use at most 1 vertex attribute for now
            requiredLimits.limits.maxVertexAttributes = 1;
            // We should also tell that we use 1 vertex buffers
            requiredLimits.limits.maxVertexBuffers = 1;
            // Maximum size of a buffer is 6 vertices of 2 float each
            requiredLimits.limits.maxBufferSize = 6 * 2 * sizeof(float);
            // Maximum stride between 2 consecutive vertices in the vertex buffer
            requiredLimits.limits.maxVertexBufferArrayStride = 2 * sizeof(float);
            // This must be set even if we do not use storage buffers for now
            requiredLimits.limits.minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment;
            // This must be set even if we do not use uniform buffers for now
            requiredLimits.limits.minUniformBufferOffsetAlignment = supportedLimits.limits.minUniformBufferOffsetAlignment;

            DeviceDescriptor deviceDesc;
            deviceDesc.label = "My Device"; // anything works here, that's your call
            deviceDesc.requiredFeaturesCount = 0; // we do not require any specific feature
            deviceDesc.requiredLimits = &requiredLimits;
            deviceDesc.defaultQueue.label = "The default queue";
            device = std::make_unique<Device>(adapter->requestDevice(deviceDesc));
            std::cout << "Got device: " << device << std::endl;

            device->setUncapturedErrorCallback([](ErrorType type, char const *message) {
                std::cout << "::error:: type " << type;
                if (message) std::cout << " (" << message << ")";
                std::cout << std::endl;
            });

            adapter->getLimits(&supportedLimits);
            std::cout << "adapter.maxVertexAttributes: " << supportedLimits.limits.maxVertexAttributes << std::endl;
            device->getLimits(&supportedLimits);
            std::cout << "device.maxVertexAttributes: " << supportedLimits.limits.maxVertexAttributes << std::endl;

            queue = std::make_unique<Queue>(device->getQueue());
            queue->onSubmittedWorkDone([](QueueWorkDoneStatus status) {
                std::cout << "Queued work finished with status: " << status << std::endl;
            });
        }

        ~GpuContext() {
            queue.reset();
            device.reset();
            adapter.reset();
            instance.reset();
        }

        [[nodiscard]] std::unique_ptr<SwapChain>
        CreateSwapChain(int width, int height, TextureFormat swapChainFormat = TextureFormat::Undefined) {
            if (swapChainFormat == TextureFormat::Undefined)
                swapChainFormat = surface.getPreferredFormat(*adapter);
            SwapChainDescriptor swapChainDesc;
            swapChainDesc.width = width;
            swapChainDesc.height = height;
            swapChainDesc.format = swapChainFormat;
            swapChainDesc.presentMode = PresentMode::Fifo;
            swapChainDesc.usage = TextureUsage::RenderAttachment;

            SwapChain swapChain = device->createSwapChain(surface, swapChainDesc);
            std::cout << "Swapchain: " << swapChain << std::endl;
            return std::make_unique<SwapChain>(swapChain);
        }

        [[nodiscard]] CommandEncoder CreateCommandEncoder(char const *label = "Command Encoder") {
            CommandEncoderDescriptor commandEncoderDesc;
            commandEncoderDesc.label = label;
            return device->createCommandEncoder(commandEncoderDesc);
        }

        [[nodiscard]] RenderPassEncoder CreateRenderPassEncoder(CommandEncoder &encoder, TextureView &textureView,
                                                                Color clearColor = {0.1, 0.3, 0.7, 1.0}) {
            RenderPassColorAttachment renderPassColorAttachment;
            renderPassColorAttachment.view = textureView;
            renderPassColorAttachment.loadOp = LoadOp::Clear;
            renderPassColorAttachment.storeOp = StoreOp::Store;
            renderPassColorAttachment.clearValue = clearColor;

            RenderPassDescriptor renderPassDesc;
            renderPassDesc.colorAttachmentCount = 1;
            renderPassDesc.colorAttachments = &renderPassColorAttachment;
            renderPassDesc.timestampWriteCount = 0;

            return encoder.beginRenderPass(renderPassDesc);
        }

        void SubmitCommandBuffer(CommandBuffer &commandBuffer) {
            queue->submit(1, &commandBuffer);
        }

        void SubmitCommandBuffers(std::vector<CommandBuffer> &commandBuffers) {
            queue->submit(commandBuffers.size(), commandBuffers.data());
        }

        void SubmitEncoder(CommandEncoder &encoder, char const *label = "Command buffer") {
            CommandBufferDescriptor cmdBufferDescriptor;
            cmdBufferDescriptor.label = label;
            CommandBuffer command = encoder.finish(cmdBufferDescriptor);
            SubmitCommandBuffer(command);
        }


        [[nodiscard]] std::unique_ptr<ShaderModule> CreateShaderModule(const char *code) {
            ShaderModuleWGSLDescriptor shaderCodeDesc;
            shaderCodeDesc.chain.next = nullptr;
            shaderCodeDesc.chain.sType = SType::ShaderModuleWGSLDescriptor;
            shaderCodeDesc.code = code;

            ShaderModuleDescriptor shaderDesc;
            shaderDesc.hintCount = 0;
            shaderDesc.hints = nullptr;
            shaderDesc.nextInChain = &shaderCodeDesc.chain;

            ShaderModule shaderModule = device->createShaderModule(shaderDesc);
            std::cout << "Shader module: " << shaderModule << std::endl;
            return std::make_unique<ShaderModule>(shaderModule);
        }

        [[nodiscard]] std::unique_ptr<RenderPipeline>
        CreateRenderPipeline(const std::unique_ptr<ShaderModule> &shaderModule,
                             TextureFormat swapChainFormat = TextureFormat::Undefined) {
            if (swapChainFormat == TextureFormat::Undefined)
                swapChainFormat = surface.getPreferredFormat(*adapter);

            std::cout << "Creating render pipeline..." << std::endl;
            RenderPipelineDescriptor pipelineDesc;

            // Vertex fetch
            // (We don't use any input buffer so far)
            pipelineDesc.vertex.bufferCount = 0;
            pipelineDesc.vertex.buffers = nullptr;

            // Vertex shader
            pipelineDesc.vertex.module = *shaderModule;
            pipelineDesc.vertex.entryPoint = "vs_main";
            pipelineDesc.vertex.constantCount = 0;
            pipelineDesc.vertex.constants = nullptr;

            // Primitive assembly and rasterization
            // Each sequence of 3 vertices is considered as a triangle
            pipelineDesc.primitive.topology = PrimitiveTopology::TriangleList;
            // We'll see later how to specify the order in which vertices should be
            // connected. When not specified, vertices are considered sequentially.
            pipelineDesc.primitive.stripIndexFormat = IndexFormat::Undefined;
            // The face orientation is defined by assuming that when looking
            // from the front of the face, its corner vertices are enumerated
            // in the counter-clockwise (CCW) order.
            pipelineDesc.primitive.frontFace = FrontFace::CCW;
            // But the face orientation does not matter much because we do not
            // cull (i.e. "hide") the faces pointing away from us (which is often
            // used for optimization).
            pipelineDesc.primitive.cullMode = CullMode::None;

            // Fragment shader
            FragmentState fragmentState;
            pipelineDesc.fragment = &fragmentState;
            fragmentState.module = *shaderModule;
            fragmentState.entryPoint = "fs_main";
            fragmentState.constantCount = 0;
            fragmentState.constants = nullptr;

            // Configure blend state
            BlendState blendState;
            // Usual alpha blending for the color:
            blendState.color.srcFactor = BlendFactor::SrcAlpha;
            blendState.color.dstFactor = BlendFactor::OneMinusSrcAlpha;
            blendState.color.operation = BlendOperation::Add;
            // We leave the target alpha untouched:
            blendState.alpha.srcFactor = BlendFactor::Zero;
            blendState.alpha.dstFactor = BlendFactor::One;
            blendState.alpha.operation = BlendOperation::Add;

            ColorTargetState colorTarget;
            colorTarget.format = swapChainFormat;
            colorTarget.blend = &blendState;
            colorTarget.writeMask = ColorWriteMask::All; // We could write to only some of the color channels.

            // We have only one target because our render pass has only one output color
            // attachment.
            fragmentState.targetCount = 1;
            fragmentState.targets = &colorTarget;

            // Depth and stencil tests are not used here
            pipelineDesc.depthStencil = nullptr;

            // Multi-sampling
            // Samples per pixel
            pipelineDesc.multisample.count = 1;
            // Default value for the mask, meaning "all bits on"
            pipelineDesc.multisample.mask = ~0u;
            // Default value as well (irrelevant for count = 1 anyways)
            pipelineDesc.multisample.alphaToCoverageEnabled = false;

            // Pipeline layout
            pipelineDesc.layout = nullptr;

            RenderPipeline pipeline = device->createRenderPipeline(pipelineDesc);
            std::cout << "Render pipeline: " << pipeline << std::endl;
            return std::make_unique<RenderPipeline>(pipeline);
        }

        [[nodiscard]] std::unique_ptr<Buffer>
        CreateBuffer(uint64_t size, WGPUBufferUsageFlags usage = BufferUsage::CopyDst | BufferUsage::CopySrc,
                     char const *label = "Buffer") {
            BufferDescriptor bufferDesc;
            bufferDesc.label = label;
            bufferDesc.usage = usage;
            bufferDesc.size = size;
            bufferDesc.mappedAtCreation = false;
            Buffer buffer = device->createBuffer(bufferDesc);
            std::cout << "Buffer: " << buffer << std::endl;
            return std::make_unique<Buffer>(buffer);
        }
    };
}
