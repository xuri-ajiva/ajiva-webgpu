//
// Created by XuriAjiva on 13.08.2023.
//
#pragma once

#include "defines.h"
#include "Platform/Window.h"
#include "Renderer/GpuContext.h"
#include "Resource/Loader.h"
#include "Core/Clock.h"

namespace Ajiva {

    struct ApplicationConfig {
        i16 startX = 200;
        i16 startY = 200;
        i16 startWidth = 800;
        i16 startHeight = 600;
        bool multiThread = true;
        std::string name;
        std::string resourceDirectory;
    };

    class Application {

    public:
        explicit Application(ApplicationConfig config);

        bool Init();

        bool IsRunning();

        void Frame();

        void Finish();

        ~Application();

    private:
        ApplicationConfig config = {};
        Ajiva::Core::Clock clock = {};
        Ajiva::Window window;
        Ajiva::Renderer::GpuContext context;
        Ajiva::Resource::Loader loader;

        Ref<wgpu::SwapChain> swapChain = nullptr;
        Ref<Ajiva::Renderer::Texture> depthTexture = nullptr;
        Ajiva::Renderer::UniformData uniforms = {};
        Ref<Ajiva::Renderer::Buffer> uniformBuffer = nullptr;
        Ref<Ajiva::Renderer::Buffer> vertexBuffer = nullptr;
        //Ref<Ajiva::Renderer::Buffer> indexBuffer;
        Ref<wgpu::RenderPipeline> renderPipeline = nullptr;
        Ref<wgpu::BindGroup> bindGroup = nullptr;
        std::vector<Ajiva::Renderer::VertexData> vertexData;
        std::vector<u16> indexData;
        Ref<Renderer::Texture> texture;


    private:
        void SetupImGui();

        bool SetupPipeline();

        void BeforeRenderFrameImGui();

        void AfterRenderFrameImGui();

    };
}
