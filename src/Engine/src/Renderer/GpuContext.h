//
// Created by XuriAjiva on 27.07.2023.
//

#pragma once

#include "defines.h"

#include <filesystem>
#include "webgpu/webgpu.hpp"
#include "Renderer/Buffer.h"
#include "Renderer/Texture.h"
#include "glm/glm.hpp"

namespace Ajiva::Renderer {
    struct VertexData { //move to resource
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 color;
        glm::vec2 uv;

        VertexData(std::istringstream &s) {
            s >> position.x;
            s >> position.y;
            s >> position.z;
            s >> normal.x;
            s >> normal.y;
            s >> normal.z;
            s >> color.r;
            s >> color.g;
            s >> color.b;
            s >> uv.x;
            s >> uv.y;
        }

        VertexData(glm::vec3 position, glm::vec3 normal, glm::vec3 color) : position(position), normal(normal),
                                                                            color(color) {}

        VertexData() = default;
    };

    struct UniformData {
        glm::mat4x4 projectionMatrix;
        glm::mat4x4 viewMatrix;
        glm::mat4x4 modelMatrix;
        glm::vec4 color;
        float time;
        float _pad[3];
    };
    static_assert(sizeof(UniformData) % 16 == 0);

    class GpuContext {
        Ref<wgpu::ErrorCallback> callback;
        Ref<wgpu::Surface> surface = nullptr;
    public:
        Ref<wgpu::Instance> instance;
        Ref<wgpu::Adapter> adapter;
        Ref<wgpu::Device> device;
        Ref<wgpu::Queue> queue;
        wgpu::TextureFormat swapChainFormat = wgpu::TextureFormat::Undefined;

        explicit GpuContext(const std::function<wgpu::Surface(wgpu::Instance)> &createSurface);


        ~GpuContext();

        [[nodiscard]] Ref<wgpu::SwapChain>
        CreateSwapChain(int width, int height) const;

        [[nodiscard]] wgpu::CommandEncoder CreateCommandEncoder(char const *label = "Command Encoder") const;

        [[nodiscard]] wgpu::RenderPassEncoder
        CreateRenderPassEncoder(wgpu::CommandEncoder &encoder, wgpu::TextureView &textureView,
                                wgpu::TextureView depthTextureView, wgpu::Color clearColor = {0.1, 0.1, 0.1, 1.0});

        void SubmitCommandBuffer(wgpu::CommandBuffer &commandBuffer) const;

        void SubmitCommandBuffers(std::vector<wgpu::CommandBuffer> &commandBuffers) const;

        void SubmitEncoder(wgpu::CommandEncoder &encoder, char const *label = "Command buffer") const;


        [[nodiscard]] Ref<wgpu::ShaderModule>
        CreateShaderModuleFromCode(const std::string &code) const;


        [[nodiscard]] Ref<wgpu::RenderPipeline>
        CreateRenderPipeline(const Ref<wgpu::ShaderModule> &shaderModule,
                             const std::vector<wgpu::BindGroupLayout> &bindGroupLayouts,
                             const std::vector<wgpu::VertexAttribute> &vertexAttribs,
                             wgpu::TextureFormat depthTextureFormat) const;

        [[nodiscard]] Ref<Ajiva::Renderer::Texture>
        CreateTexture(const WGPUTextureFormat &textureFormat, const WGPUExtent3D &textureSize,
                      wgpu::TextureUsage usage = wgpu::TextureUsage::RenderAttachment,
                      wgpu::TextureAspect textureAspect = wgpu::TextureAspect::All,
                      const char *label = "Texture") const;

        [[nodiscard]] inline Ref<Ajiva::Renderer::Texture>
        CreateDepthTexture(const WGPUExtent3D &textureSize);

        [[nodiscard]] Ref<wgpu::BindGroupLayout>
        CreateBindGroupLayout(std::vector<wgpu::BindGroupLayoutEntry> entries);

        [[nodiscard]] Ref<wgpu::BindGroup>
        CreateBindGroup(const Ref<wgpu::BindGroupLayout> &bindGroupLayout, const Ref<Buffer> &uniformBuffer,
                        std::vector<wgpu::BindGroupEntry> bindings) const;

        [[nodiscard]] Ref<Ajiva::Renderer::Buffer>
        CreateBuffer(uint64_t size, WGPUBufferUsageFlags usage =
        wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc, char const *label = "Buffer") const;

        [[nodiscard]] Ref<Ajiva::Renderer::Buffer>
        CreateFilledBuffer(void const *data, uint64_t size,
                           WGPUBufferUsageFlags usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc,
                           char const *label = "Buffer") const;
    };
}
