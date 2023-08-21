//
// Created by XuriAjiva on 13.08.2023.
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

namespace Ajiva {
    struct ApplicationConfig {
        Ajiva::Platform::WindowConfig WindowConfig;
        std::string ResourceDirectory;
    };

    class AJ_API Application {

    public:
        explicit Application(ApplicationConfig config);

        bool Init();

        bool IsRunning();

        void Frame();

        void Finish();

        ~Application();

        bool OnResize(Ajiva::Core::EventCode code, void *sender, const Ajiva::Core::EventContext &event);

    private:
        ApplicationConfig config = {};
        Ajiva::Core::Clock clock = {};
        Ref<Ajiva::Platform::Window> window;
        Ref<Ajiva::Renderer::GpuContext> context;
        Ref<Ajiva::Resource::Loader> loader;
        Ref<Core::EventSystem> eventSystem = nullptr;


        Ref<wgpu::SwapChain> swapChain = nullptr;
        Ref<Ajiva::Renderer::Texture> depthTexture = nullptr;
        Ajiva::Renderer::UniformData uniforms = {};
        Ajiva::Renderer::LightningUniform lightningUniform = {};
        std::vector<Ajiva::Renderer::InstanceData> instanceData;
        Ref<Ajiva::Renderer::Buffer> instanceBuffer = nullptr;
        Ref<Ajiva::Renderer::Buffer> uniformBuffer = nullptr;
        Ref<Ajiva::Renderer::Buffer> lightningUniformBuffer = nullptr;
        Ref<Ajiva::Renderer::Buffer> vertexBuffer = nullptr;
        //Ref<Ajiva::Renderer::Buffer> indexBuffer;
        Ref<wgpu::RenderPipeline> renderPipeline = nullptr;
        Renderer::BindGroupBuilder bindGroupBuilder;
        std::vector<Ajiva::Renderer::VertexData> vertexData;
        std::vector<u16> indexData;
        Ref<Renderer::EventCamera> camera;
        Renderer::Projection projection = {};

        std::vector<Ref<Ajiva::Core::Layer>> layers;
        std::vector<Ref<Ajiva::Core::IListener>> events;

        Ref<Core::ThreadPool<>> threadPool;
    private:
        bool SetupPipeline();

        void BuildSwapChain();

        void BuildDepthTexture();
    };
}
