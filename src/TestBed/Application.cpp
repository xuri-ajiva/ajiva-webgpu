//
// Created by XuriAjiva on 13.08.2023.
//

#include "Application.h"
#include "Resource/ResourceManager.h"
#include <glm/ext.hpp>
#include "Core/Layer.h"
#include "Renderer/ImGuiLayer.h"
#include "Resource/FilesNames.hpp"
#include "Renderer/RenderPipelineLayer.h"
#include "GameOfLife.h"

namespace Ajiva
{
    Application::~Application() = default;

    Application::Application(ApplicationConfig config) : config(std::move(config))
    {
    }

    inline void onMouseButtonDown(AJ_EVENT_PARAMETERS)
    {
        PLOG_INFO << "onMouseButtonDown";
    }

    bool Application::Init()
    {
        Core::SetupLogger();
        PLOG_INFO << "Hello, World!";

        threadPool = CreateRef<Core::ThreadPool<>>(false);
        threadPool->Start();

        eventSystem = CreateRef<Core::EventSystem>();
        //events.push_back(eventSystem->AddEventListener<Core::FramebufferResize>(AJ_EVENT_CALLBACK_VOID(OnResize)));
        events.push_back(eventSystem->Add(Core::FramebufferResize, this, &Application::OnResize));

        context = CreateRef<Renderer::GpuContext>();
        loader = CreateRef<Resource::Loader>(config.ResourceDirectory, threadPool);
        graphicsResourceManager = CreateRef<Renderer::GraphicsResourceManager>(context, loader);
        window = CreateRef<Platform::Window>(config.WindowConfig, eventSystem);

        clock.Start();
        window->Create();
        if (!context->Init(window->CreateSurfaceFunk())) return false;
        window->Run();

        //ImGui first to block camera input
        camera = CreateRef<Renderer::FreeCamera>(eventSystem);
        Renderer::RenderPipelineLayer pipeline(context, loader, graphicsResourceManager,
                                               [this]() -> glm::mat4x4
                                               {
                                                   //todo make more compact
                                                   return camera->viewMatrix;
                                               }, [this]() -> glm::mat4x4
                                               {
                                                   return projection.Build();
                                               }, [this]() -> glm::vec3
                                               {
                                                   return camera->position;
                                               });
        auto pipelineRef = CreateRef<Renderer::RenderPipelineLayer>(pipeline);
        auto golRef = CreateRef<GameOfLife>(context, eventSystem, window, loader, pipelineRef);
        Renderer::ImGuiLayer imGuiLayer(window, context, eventSystem, pipelineRef, camera);
        camera->Init();

        //layers
        layers.push_back(pipelineRef);
        layers.push_back(golRef);
        layers.push_back(CreateRef<Renderer::ImGuiLayer>(imGuiLayer));

        for (const auto& layer : layers)
        {
            if (!layer->IsEnabled()) continue;
            layer->Attached();
        }
        projection.aspect = static_cast<float>(window->GetWidth()) / static_cast<float>(window->GetHeight());
        projection.far = 1000.0f;
        projection.near = 0.01f;
        BuildSwapChain();

        //AJ_INFO("Startup Time: %s", clock.Total());
        PLOG_DEBUG << clock.Total().count() / 1000.0f << "s";
        clock.Reset();
        return true;
    }

    bool Application::IsRunning()
    {
        return !window->IsClosed();
    }

    void Application::Frame()
    {
        using glm::mat4x4;
        using glm::vec4;
        using glm::vec3;

        clock.Update();
        Core::UpdateInfo frameInfo = {
            clock.Ticks(),
            std::chrono::duration_cast<std::chrono::duration<float>>(clock.Delta()).count(),
            std::chrono::duration_cast<std::chrono::duration<float>>(clock.Total()).count()
        };

        //update "camera"
        camera->Update();

        static std::string lastStats;
        auto newStats = graphicsResourceManager->Statistics();
        if (newStats != lastStats)
        {
            PLOG_INFO << newStats;
            lastStats = newStats;
        }

        //todo seperate render and update thread?
        for (const auto& layer : layers)
        {
            if (!layer->IsEnabled()) continue;
            layer->Update(frameInfo);
        }

        wgpu::TextureView nextTexture = swapChain->getCurrentTextureView();
        //std::cout << "nextTexture: " << nextTexture << std::endl;

        if (!nextTexture)
        {
            std::cerr << "Cannot acquire next swap chain texture" << std::endl;
            window->RequestClose();
        }

        //todo render to texture and blend into nextTexture
        auto renderTarget = Core::RenderTarget{
            .texture = nextTexture,
            .extent = {
                static_cast<uint32_t>(window->GetWidth()), static_cast<uint32_t>(window->GetHeight()),
                1
            }, //todo need accurate size for multithreading
        };

        for (const auto& layer : layers)
        {
            if (!layer->IsEnabled()) continue;
            layer->BeforeRender(frameInfo, renderTarget);
        }

        for (const auto& layer : layers)
        {
            if (!layer->IsEnabled()) continue;
            layer->Render(frameInfo, renderTarget);
        }

        for (const auto& layer : layers)
        {
            if (!layer->IsEnabled()) continue;
            layer->AfterRender(frameInfo, renderTarget);
        }

        nextTexture.release();
        swapChain->present();
    }

    void Application::Finish()
    {
        for (const auto& layer : layers)
        {
            if (!layer->IsEnabled()) continue;
            layer->Detached();
        }
        layers.clear();
        swapChain->release();
    }

    bool Application::OnResize(Ajiva::Core::EventCode code, void* sender, const Ajiva::Core::EventContext& event)
    {
        if (event.framebufferSize.width == 0 || event.framebufferSize.height == 0)
        {
            PLOG_INFO << "Framebuffer size is 0!";
            return false;
        }

        BuildSwapChain();

        BuildDepthTexture();

        //update "camera"
        /*        uniforms.projectionMatrix = glm::perspective(glm::radians(45.0f), static_cast<float>(event.windowRect.width) /
                                                                                  static_cast<float>(event.windowRect.height),
                                                             0.1f, 100.0f);*/
        projection.aspect = static_cast<float>(event.windowRect.width) / static_cast<float>(event.windowRect.height);
        //TODO??        uniformBuffer->UpdateBufferData(&uniforms, sizeof(Ajiva::Renderer::UniformData));
        return false;
    }

    void Application::BuildSwapChain()
    {
        if (swapChain)
        {
            swapChain->release();
        }

        swapChain = context->CreateSwapChain(window->GetWidth(), window->GetHeight());
    }

    void Application::BuildDepthTexture()
    {
        //the ref should auto release
    }
}
