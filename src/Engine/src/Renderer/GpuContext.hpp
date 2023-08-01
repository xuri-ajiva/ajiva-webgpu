//
// Created by XuriAjiva on 27.07.2023.
//

#pragma once

#include <filesystem>
#include <fstream>
#include <vector>
#include "webgpu/webgpu.hpp"
#include "magic_enum.hpp"
#include "Core/Logger.h"

#define ALIGN_AT(value, X) (value + X - 1) & ~(X - 1)

using namespace wgpu;
namespace Ajiva::Renderer {
    struct VertexData {
        float x, y;
        float r, g, b;
    };
    struct UniformData {
        float color[4];
        float time;
        float aspectRatio;
        float _pad[2];
    };
    static_assert(sizeof(UniformData) % 16 == 0);

    class GpuContext {
    public:
        std::unique_ptr<Instance> instance;
        std::unique_ptr<Adapter> adapter;
        std::unique_ptr<Device> device;
        std::unique_ptr<Queue> queue;
        std::unique_ptr<Surface> surface = nullptr;
        TextureFormat swapChainFormat = TextureFormat::Undefined;

        explicit GpuContext(const std::function<wgpu::Surface(wgpu::Instance)> &createSurface) {
            instance = std::make_unique<Instance>(createInstance(InstanceDescriptor{}));
            if (!instance) {
                PLOG_FATAL << "Could not initialize WebGPU!";
                return;
            }
            PLOG_INFO << "WebGPU instance: " << instance.get();
            surface = std::make_unique<Surface>(createSurface(*instance));

            PLOG_INFO << "Requesting adapter...";
            RequestAdapterOptions adapterOpts;
            adapterOpts.compatibleSurface = *surface;
            adapter = std::make_unique<Adapter>(instance->requestAdapter(adapterOpts));
            PLOG_INFO << "Got adapter: " << adapter.get();
            SupportedLimits supportedLimits;
            adapter->getLimits(&supportedLimits);

            PLOG_INFO << "Requesting device...";
            // Don't forget to = Default
            RequiredLimits requiredLimits = Default;
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

            DeviceDescriptor deviceDesc;
            deviceDesc.label = "My Device"; // anything works here, that's your call
            deviceDesc.requiredFeaturesCount = 0; // we do not require any specific feature
            deviceDesc.requiredLimits = &requiredLimits;
            deviceDesc.defaultQueue.label = "The default queue";
            device = std::make_unique<Device>(adapter->requestDevice(deviceDesc));
            PLOG_INFO << "Got device: " << device.get();

            callback = device->setUncapturedErrorCallback([](ErrorType type, char const *message) {
                if (type != WGPUErrorType_NoError)
                    PLOG_FATAL << magic_enum::enum_name<WGPUErrorType>(type) << ": " << message;
                else PLOG_INFO << magic_enum::enum_name<WGPUErrorType>(type) << ": " << message;
            });

            adapter->getLimits(&supportedLimits);
            PLOG_INFO << "adapter.maxVertexAttributes: " << supportedLimits.limits.maxVertexAttributes;
            device->getLimits(&supportedLimits);
            PLOG_INFO << "device.maxVertexAttributes: " << supportedLimits.limits.maxVertexAttributes;

            queue = std::make_unique<Queue>(device->getQueue());
            queue->onSubmittedWorkDone([](QueueWorkDoneStatus status) {
                PLOG_VERBOSE
                            << "QueueWorkDoneStatus: " << magic_enum::enum_name<WGPUQueueWorkDoneStatus>(status).data();
            });

            //swapChainFormat = surface->getPreferredFormat(*adapter);
            if (swapChainFormat == TextureFormat::Undefined)
                swapChainFormat = TextureFormat::BGRA8Unorm;
            PLOG_INFO << "SwapChainFormat: " << magic_enum::enum_name<WGPUTextureFormat>(swapChainFormat).data();
        }

        std::unique_ptr<ErrorCallback> callback;

        ~GpuContext() {
            queue.reset();
            device.reset();
            adapter.reset();
            instance.reset();
        }

        [[nodiscard]] std::unique_ptr<SwapChain>
        CreateSwapChain(int width, int height) const {
            SwapChainDescriptor swapChainDesc;
            swapChainDesc.width = width;
            swapChainDesc.height = height;
            swapChainDesc.format = swapChainFormat;
            swapChainDesc.presentMode = PresentMode::Fifo;
            swapChainDesc.usage = TextureUsage::RenderAttachment;

            SwapChain swapChain = device->createSwapChain(*surface, swapChainDesc);
            PLOG_INFO << "Created swap chain: " << &swapChain;
            return std::make_unique<SwapChain>(swapChain);
        }

        [[nodiscard]] CommandEncoder CreateCommandEncoder(char const *label = "Command Encoder") const {
            CommandEncoderDescriptor commandEncoderDesc;
            commandEncoderDesc.label = label;
            return device->createCommandEncoder(commandEncoderDesc);
        }

        [[nodiscard]] RenderPassEncoder CreateRenderPassEncoder(CommandEncoder &encoder, TextureView &textureView,
                                                                Color clearColor = {0.1, 0.1, 0.1, 1.0}) {
            RenderPassColorAttachment renderPassColorAttachment;
            renderPassColorAttachment.view = textureView;
            renderPassColorAttachment.loadOp = LoadOp::Load;
            renderPassColorAttachment.storeOp = StoreOp::Store;
            renderPassColorAttachment.clearValue = clearColor;

            RenderPassDescriptor renderPassDesc;
            renderPassDesc.colorAttachmentCount = 1;
            renderPassDesc.colorAttachments = &renderPassColorAttachment;
            renderPassDesc.timestampWriteCount = 0;

            return encoder.beginRenderPass(renderPassDesc);
        }

        void SubmitCommandBuffer(CommandBuffer &commandBuffer) const {
            queue->submit(1, &commandBuffer);
        }

        void SubmitCommandBuffers(std::vector<CommandBuffer> &commandBuffers) const {
            queue->submit(commandBuffers.size(), commandBuffers.data());
        }

        void SubmitEncoder(CommandEncoder &encoder, char const *label = "Command buffer") const {
            CommandBufferDescriptor cmdBufferDescriptor;
            cmdBufferDescriptor.label = label;
            CommandBuffer command = encoder.finish(cmdBufferDescriptor);
            SubmitCommandBuffer(command);
        }


        [[nodiscard]] std::unique_ptr<ShaderModule>
        CreateShaderModuleFromCode(const char *code) const {
            ShaderModuleWGSLDescriptor shaderCodeDesc;
            shaderCodeDesc.chain.next = nullptr;
            shaderCodeDesc.chain.sType = SType::ShaderModuleWGSLDescriptor;
            shaderCodeDesc.code = code;

            ShaderModuleDescriptor shaderDesc;
            shaderDesc.hintCount = 0;
            shaderDesc.hints = nullptr;
            shaderDesc.nextInChain = &shaderCodeDesc.chain;

            ShaderModule shaderModule = device->createShaderModule(shaderDesc);
            PLOG_INFO << "Created shader module: " << &shaderModule;
            return std::make_unique<ShaderModule>(shaderModule);
        }

        [[nodiscard]] std::unique_ptr<ShaderModule>
        CreateShaderModuleFromFile(std::filesystem::path const &path) {
            std::ifstream file(path);
            std::string code((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());
            if (code.empty()) {
                PLOG_FATAL << "Failed to load shader from file: " << path;
                return nullptr;
            }
            return CreateShaderModuleFromCode(code.c_str());
        }

        static bool LoadGeometry(const std::filesystem::path &path, std::vector<VertexData> &pointData,
                                 std::vector<uint16_t> &indexData) {
            std::ifstream file(path);
            if (!file.is_open()) {
                return false;
            }

            pointData.clear();
            indexData.clear();

            enum class Section {
                None,
                Points,
                Indices,
            };
            Section currentSection = Section::None;

            float value;
            uint16_t index;
            std::string line;
            while (!file.eof()) {
                getline(file, line);

                // overcome the `CRLF` problem
                if (!line.empty() && line.back() == '\r') {
                    line.pop_back();
                }

                if (line == "[points]") {
                    currentSection = Section::Points;
                } else if (line == "[indices]") {
                    currentSection = Section::Indices;
                } else if (line[0] == '#' || line.empty()) {
                    // Do nothing, this is a comment
                } else if (currentSection == Section::Points) {
                    std::istringstream iss(line);
                    // Get x, y, r, g, b
                    VertexData vertex{};
                    iss >> vertex.x;
                    iss >> vertex.y;
                    iss >> vertex.r;
                    iss >> vertex.g;
                    iss >> vertex.b;

                    pointData.push_back(vertex);
                } else if (currentSection == Section::Indices) {
                    std::istringstream iss(line);
                    // Get corners #0 #1 and #2
                    for (int i = 0; i < 3; ++i) {
                        iss >> index;
                        indexData.push_back(index);
                    }
                }
            }
            return true;
        }

        [[nodiscard]] std::unique_ptr<RenderPipeline>
        CreateRenderPipeline(const std::unique_ptr<ShaderModule> &shaderModule,
                             const std::vector<BindGroupLayout> &bindGroupLayouts
        ) const {
            PLOG_INFO << "Creating render pipeline";
            RenderPipelineDescriptor pipelineDesc;

            // Vertex fetch
            // We now have 2 attributes
            std::vector<VertexAttribute> vertexAttribs(2);

            // Position attribute
            vertexAttribs[0].shaderLocation = 0;
            vertexAttribs[0].format = VertexFormat::Float32x2;
            vertexAttribs[0].offset = offsetof(VertexData, x);

            // Color attribute
            vertexAttribs[1].shaderLocation = 1;
            vertexAttribs[1].format = VertexFormat::Float32x3; // different type!
            vertexAttribs[1].offset = offsetof(VertexData, r); // non null offset!

            VertexBufferLayout vertexBufferLayout;
            // [...] Build vertex buffer layout
            vertexBufferLayout.attributeCount = vertexAttribs.size();
            vertexBufferLayout.attributes = vertexAttribs.data();
            // == Common to attributes from the same buffer ==
            vertexBufferLayout.arrayStride = sizeof(VertexData);
            vertexBufferLayout.stepMode = VertexStepMode::Vertex;

            pipelineDesc.vertex.bufferCount = 1;
            pipelineDesc.vertex.buffers = &vertexBufferLayout;

            // Vertex shader
            pipelineDesc.vertex.module = *shaderModule;
            pipelineDesc.vertex.entryPoint = "vs_main";
            pipelineDesc.vertex.constantCount = 0;
            pipelineDesc.vertex.constants = nullptr;

            pipelineDesc.primitive.topology = PrimitiveTopology::TriangleList;
            pipelineDesc.primitive.stripIndexFormat = IndexFormat::Undefined;
            pipelineDesc.primitive.frontFace = FrontFace::CCW;
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

            // Create the pipeline layout
            PipelineLayoutDescriptor layoutDesc;
            layoutDesc.bindGroupLayoutCount = bindGroupLayouts.size();
            layoutDesc.bindGroupLayouts = (WGPUBindGroupLayout *) bindGroupLayouts.data();
            PipelineLayout layout = device->createPipelineLayout(layoutDesc);
            pipelineDesc.layout = layout;

            RenderPipeline pipeline = device->createRenderPipeline(pipelineDesc);
            PLOG_INFO << "Render pipeline: " << pipeline;
            return std::make_unique<RenderPipeline>(pipeline);
        }

        [[nodiscard]] std::unique_ptr<BindGroupLayout>
        CreateBindGroupLayout() {
            // Create binding layout (don't forget to = Default)
            BindGroupLayoutEntry bindingLayout = Default;
            // The binding index as used in the @binding attribute in the shader
            bindingLayout.binding = 0;
            // The stage that needs to access this resource
            bindingLayout.visibility = ShaderStage::Vertex | ShaderStage::Fragment;
            bindingLayout.buffer.type = BufferBindingType::Uniform;
            bindingLayout.buffer.minBindingSize = sizeof(UniformData);

            // Create a bind group layout
            BindGroupLayoutDescriptor bindGroupLayoutDesc;
            bindGroupLayoutDesc.entryCount = 1;
            bindGroupLayoutDesc.entries = &bindingLayout;
            BindGroupLayout bindGroupLayout = device->createBindGroupLayout(bindGroupLayoutDesc);
            PLOG_INFO << "Bind group layout: " << bindGroupLayout;
            return std::make_unique<BindGroupLayout>(bindGroupLayout);
        }

        [[nodiscard]] std::unique_ptr<BindGroup>
        CreateBindGroup(const std::unique_ptr<BindGroupLayout> &bindGroupLayout,
                        const std::unique_ptr<Buffer> &uniformBuffer) const {
            PLOG_INFO << "Creating bind group";
            BindGroupEntry binding;
            // The index of the binding (the entries in bindGroupDesc can be in any order)
            binding.binding = 0;
            // The buffer it is actually bound to
            binding.buffer = *uniformBuffer;
            // We can specify an offset within the buffer, so that a single buffer can hold
            // multiple uniform blocks.
            binding.offset = 0;
            // And we specify again the size of the buffer.
            binding.size = sizeof(UniformData);

            // A bind group contains one or multiple bindings
            BindGroupDescriptor bindGroupDesc;
            bindGroupDesc.layout = *bindGroupLayout;
            // There must be as many bindings as declared in the layout!
            bindGroupDesc.entryCount = 1; //(bindGroupLayoutDesc.entryCount)
            bindGroupDesc.entries = &binding;
            BindGroup bindGroup = device->createBindGroup(bindGroupDesc);
            PLOG_INFO << "Bind group: " << bindGroup;
            return std::make_unique<BindGroup>(bindGroup);
        }


        [[nodiscard]] std::unique_ptr<Buffer>
        CreateBuffer(uint64_t size, WGPUBufferUsageFlags usage = BufferUsage::CopyDst | BufferUsage::CopySrc,
                     char const *label = "Buffer") const {
            BufferDescriptor bufferDesc;
            bufferDesc.label = label;
            bufferDesc.usage = usage;
            bufferDesc.size = size;
            bufferDesc.mappedAtCreation = false;
            Buffer buffer = device->createBuffer(bufferDesc);
            PLOG_VERBOSE << "Buffer: " << buffer;
            return std::make_unique<Buffer>(buffer);
        }

        [[nodiscard]] std::unique_ptr<Buffer>
        CreateFilledBuffer(void const *data, uint64_t size,
                           WGPUBufferUsageFlags usage = BufferUsage::CopyDst | BufferUsage::CopySrc,
                           char const *label = "Buffer") const {
            //enshure size is multiple of 4
            size = ALIGN_AT(size, 4);
            auto buffer = CreateBuffer(size, usage, label);
            UpdateBufferData(buffer, data, size);
            return buffer;
        }

        void UpdateBufferData(const std::unique_ptr<Buffer> &buffer, void const *data, uint64_t size) const {
            queue->writeBuffer(*buffer, 0, data, ALIGN_AT(size, 4));
            //std::cout << "Buffer updated" << std::endl;
        }
    };
}
