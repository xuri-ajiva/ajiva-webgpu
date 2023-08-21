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
        glm::vec3 worldPos;
        float time;
    };
    static_assert(sizeof(UniformData) % 16 == 0);

    struct InstanceData {
        glm::mat4x4 modelMatrix;
        glm::vec4 color;
    };
    static_assert(sizeof(InstanceData) % 16 == 0);

    struct Light {
        glm::vec4 position;
        glm::vec4 color;
    };
    static_assert(sizeof(Light) == 32);

    struct LightningUniform {
        Light lights[4];
        glm::vec4 ambient;
        float hardness;
        float kd;
        float ks;
        float padding;
    };
    static_assert(sizeof(LightningUniform) % 16 == 0);

    class GpuContext {
        Ref<wgpu::ErrorCallback> callback;
        Ref<wgpu::Surface> surface = nullptr;
    public:
        Ref<wgpu::Instance> instance;
        Ref<wgpu::Adapter> adapter;
        Ref<wgpu::Device> device;
        Ref<wgpu::Queue> queue;
        wgpu::TextureFormat swapChainFormat = wgpu::TextureFormat::Undefined;
        wgpu::TextureFormat depthTextureFormat = wgpu::TextureFormat::Depth24Plus;

        GpuContext();

        ~GpuContext();

        bool Init(const std::function<wgpu::Surface(wgpu::Instance)> &createSurface);

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
                      wgpu::TextureUsage usage, wgpu::TextureAspect textureAspect, uint32_t mipLevelCount,
                      const char *label) const;

        [[nodiscard]] Ref<Ajiva::Renderer::Texture>
        CreateDepthTexture(const WGPUExtent3D &textureSize);

        [[nodiscard]] Ref<wgpu::Sampler>
        CreateSampler(wgpu::AddressMode addressMode, wgpu::FilterMode filterMode,
                      wgpu::CompareFunction compareFunction, float lodMinClamp,
                      float lodMaxClamp, const char *label = "Sampler") const;


        [[nodiscard]] Ref<wgpu::BindGroupLayout>
        CreateBindGroupLayout(std::vector<wgpu::BindGroupLayoutEntry> entries);

        [[nodiscard]] Ref<wgpu::BindGroup>
        CreateBindGroup(const Ref<wgpu::BindGroupLayout> &bindGroupLayout,
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
