//
// Created by XuriAjiva on 13.08.2023.
//

#include "Application.h"
#include "Resource/ResourceManager.h"
#include <glm/ext.hpp>
#include "Core/Layer.h"
#include "Renderer/ImGuiLayer.h"

namespace Ajiva {

    Application::~Application() = default;

    Application::Application(ApplicationConfig config) : config(std::move(config)) {}

    inline void onMouseButtonDown(AJ_EVENT_PARAMETERS) {
        PLOG_INFO << "onMouseButtonDown";
    }

    bool Application::Init() {
        Core::SetupLogger();
        PLOG_INFO << "Hello, World!";
        eventSystem = CreateRef<Core::EventSystem>();
        //events.push_back(eventSystem->AddEventListener<Core::FramebufferResize>(AJ_EVENT_CALLBACK_VOID(OnResize)));
        events.push_back(eventSystem->Add(Core::FramebufferResize, this, &Application::OnResize));

        context = CreateRef<Renderer::GpuContext>();
        loader = CreateRef<Resource::Loader>(config.ResourceDirectory);
        window = CreateRef<Platform::Window>(config.WindowConfig, eventSystem);

        clock.Start();
        window->Create();
        if (!context->Init(window->CreateSurfaceFunk())) return false;
        window->Run();

        //ImGui first to block camera input
        Renderer::ImGuiLayer imGuiLayer(window, context, eventSystem, &lightningUniform);
        camera = CreateRef<Renderer::Camera>(eventSystem);
        camera->Init();

        bindGroupBuilder = Renderer::BindGroupBuilder(context, loader);

        //layers
        layers.push_back(CreateRef<Renderer::ImGuiLayer>(imGuiLayer));

        for (const auto &layer: layers) {
            if (!layer->IsEnabled()) continue;
            layer->Attached();
        }
        SetupPipeline();
        return true;
    }

    bool Application::SetupPipeline() {
        uniforms = {
                .color = {1.0f, 0.5f, 0.0f, 1.0f},
                .time = 1.0f,
        };

        lightningUniform = {
                .lights = {
                        {.position = {0.5f, 0.5f, 1.0f, 0}, .color = {1.0f, 1.0f, 1.0f, 1.0f}},
                        {.position = {1.0f, 1.0f, 1.0f, 0}, .color = {1.0f, 0.0f, 0.0f, 1.0f}},
                        {.position = {1.0f, 0.0f, 1.0f, 0}, .color = {0.0f, 1.0f, 0.0f, 1.0f}},
                        {.position = {0.0f, 1.0f, 1.0f, 0}, .color = {0.0f, 0.0f, 1.0f, 1.0f}},
                },
                .ambient = {0.5f, 0.5f, 0.5f, 1.0f},
                .hardness = 32.0f,
                .kd = 0.8f,
                .ks = 0.2f,
        };


        BuildSwapChain();
        // Create the depth texture
        BuildDepthTexture();

        //todo move to member??? or not
        Ref<wgpu::ShaderModule> shaderModule = context->CreateShaderModuleFromCode(loader->LoadFile("shader.wgsl"));

        // Vertex fetch
        // We now have 2 attributes
        std::vector<wgpu::VertexAttribute> vertexAttribs = {
                WGPUVertexAttribute{ //todo macro this
                        .format = wgpu::VertexFormat::Float32x3,
                        .offset = offsetof(Ajiva::Renderer::VertexData, position),
                        .shaderLocation = 0,
                },
                WGPUVertexAttribute{
                        .format = wgpu::VertexFormat::Float32x3,
                        .offset = offsetof(Ajiva::Renderer::VertexData, normal),
                        .shaderLocation = 1,
                },
                WGPUVertexAttribute{
                        .format = wgpu::VertexFormat::Float32x3,
                        .offset = offsetof(Ajiva::Renderer::VertexData, color),
                        .shaderLocation = 2,
                },
                WGPUVertexAttribute{
                        .format = wgpu::VertexFormat::Float32x2,
                        .offset = offsetof(Ajiva::Renderer::VertexData, uv),
                        .shaderLocation = 3,
                },
        };

/*    auto depthTexture = context->CreateTexture(TextureFormat::Depth24Plus, {static_cast<uint32_t>(window->GetWidth()),
                                                                           static_cast<uint32_t>(window->GetHeight()),
                                                                           1});*/

        {
            uniformBuffer = context->CreateFilledBuffer(&uniforms, sizeof(Ajiva::Renderer::UniformData),
                                                        wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform,
                                                        "Uniform Buffer");
            bindGroupBuilder.PushBuffer(uniformBuffer);

            const int mipLevelCount = 8;
            auto sampler = context->CreateSampler(wgpu::AddressMode::Repeat, wgpu::FilterMode::Linear,
                                                  wgpu::CompareFunction::Undefined, 0.0f, 1.0f * mipLevelCount,
                                                  "Sampler");
            bindGroupBuilder.PushSampler(sampler);

            auto texture = loader->LoadTexture("cobblestone_floor_08_diff_2k.jpg", *context, mipLevelCount);
            if (!texture) {
                AJ_FAIL("Could not load texture!");
                return false;
            }
            bindGroupBuilder.PushTexture(texture);

            texture = loader->LoadTexture("cobblestone_floor_08_nor_gl_2k.png", *context, mipLevelCount);
            if (!texture) {
                AJ_FAIL("Could not load texture!");
                return false;
            }
            bindGroupBuilder.PushTexture(texture);

            lightningUniformBuffer = context->CreateFilledBuffer(&uniforms, sizeof(Ajiva::Renderer::LightningUniform),
                                                                 wgpu::BufferUsage::CopyDst |
                                                                 wgpu::BufferUsage::Uniform,
                                                                 "Lightning Uniform Buffer");
            bindGroupBuilder.PushBuffer(lightningUniformBuffer);


            auto bindGroupLayout = bindGroupBuilder.BuildBindGroupLayout();
            renderPipeline = context->CreateRenderPipeline(shaderModule, std::vector{*bindGroupLayout}, vertexAttribs,
                                                           depthTexture->textureFormat);

        }

        bool success = loader->LoadGeometryFromObj("plane.obj", vertexData, indexData);
        if (!success) {
            std::cerr << "Could not load geometry!" << std::endl;
            return false;
        }


        {
            //camera stuff
            using glm::mat4x4;
            using glm::vec4;
            using glm::vec3;
            using glm::cos;
            using glm::sin;


            vec3 eye = {0, 0, 0};
            vec3 pos = {-2.0f, -3.0f, 2.0f};
            mat4x4 V = glm::lookAt(pos, eye, vec3{0, 0, 1}); //z-up, (left handed ?? )
            uniforms.viewMatrix = V;
            uniforms.modelMatrix = glm::mat4(1.0f);

            float ratio = static_cast<float>(window->GetWidth()) / static_cast<float>(window->GetHeight());
            float fov = 45.0f * PI / 180.0f;
            float near = 0.01f;
            float far = 100.0f;
            uniforms.projectionMatrix = glm::perspective(fov, ratio, near, far);
        }

        vertexBuffer = context->CreateFilledBuffer(vertexData.data(),
                                                   vertexData.size() * sizeof(Ajiva::Renderer::VertexData),
                                                   wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex,
                                                   "Vertex Buffer");
        /*auto indexBuffer = context->CreateFilledBuffer(indexData.data(), indexData.size() * sizeof(uint16_t),
                                                      wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index,
                                                      "Index Buffer");*/



        //AJ_INFO("Startup Time: %s", clock.Total());
        PLOG_DEBUG << std::chrono::duration_cast<std::chrono::milliseconds>(clock.Total());
        clock.Reset();
        return true;
    }

    bool Application::IsRunning() {
        return !window->IsClosed();
    }

    void Application::Frame() {
        using glm::mat4x4;
        using glm::vec4;
        using glm::vec3;

        clock.Update();
        Core::FrameInfo frameInfo = {clock.Ticks(),
                                     std::chrono::duration_cast<std::chrono::duration<float>>(clock.Delta()).count(),
                                     std::chrono::duration_cast<std::chrono::duration<float>>(clock.Total()).count()};

        ///Before Render

        for (const auto &layer: layers) {
            if (!layer->IsEnabled()) continue;
            layer->BeforeRender();
        }

        wgpu::TextureView nextTexture = swapChain->getCurrentTextureView();
        //std::cout << "nextTexture: " << nextTexture << std::endl;

        if (!nextTexture) {
            std::cerr << "Cannot acquire next swap chain texture" << std::endl;
            window->RequestClose();
        }

        //update "camera"
        uniforms.time = frameInfo.TotalTime;
/*        uniforms.modelMatrix = glm::rotate(mat4x4(1.0), uniforms.time, vec3(0.0, 0.0, 1.0)) *
                               glm::translate(mat4x4(1.0), vec3(0.5, 0.0, 0.0)) *
                               glm::scale(mat4x4(1.0), vec3(0.8f));*/
        uniforms.viewMatrix = camera->viewMatrix;
        uniforms.worldPos = camera->position;
        uniformBuffer->UpdateBufferData(&uniforms, sizeof(Ajiva::Renderer::UniformData));
        lightningUniformBuffer->UpdateBufferData(&lightningUniform, sizeof(Ajiva::Renderer::LightningUniform));

        for (const auto &layer: layers) {
            if (!layer->IsEnabled()) continue;
            layer->Render(frameInfo);
        }


        wgpu::CommandEncoder encoder = context->CreateCommandEncoder();

        wgpu::RenderPassEncoder renderPass = context->CreateRenderPassEncoder(encoder, nextTexture, depthTexture->view,
                                                                              {0.4, 0.4, 0.4, 1.0});
        renderPass.setPipeline(*renderPipeline);
        renderPass.setVertexBuffer(0, vertexBuffer->buffer, 0, vertexData.size() * sizeof(Ajiva::Renderer::VertexData));
        /* renderPass.setIndexBuffer(indexBuffer->buffer, wgpu::IndexFormat::Uint16, 0,
                                   indexData.size() * sizeof(uint16_t));*/
        renderPass.setBindGroup(0, *bindGroupBuilder.bindGroup, 0, nullptr);
        //renderPass.drawIndexed(indexData.size(), 1, 0, 0, 0);
        renderPass.draw(vertexData.size(), 1, 0, 0);

        for (const auto &layer: layers) {
            if (!layer->IsEnabled()) continue;
            layer->AfterRender(renderPass);
        }

        renderPass.end();

        nextTexture.release();

        context->SubmitEncoder(encoder);

        swapChain->present();
    }

    void Application::Finish() {
        for (const auto &layer: layers) {
            if (!layer->IsEnabled()) continue;
            layer->Detached();
        }
        layers.clear();
        swapChain->release();
    }

    bool Application::OnResize(Ajiva::Core::EventCode code, void *sender, const Ajiva::Core::EventContext &event) {
        if (event.framebufferSize.width == 0 || event.framebufferSize.height == 0) {
            PLOG_INFO << "Framebuffer size is 0!";
            return false;
        }

        BuildSwapChain();

        BuildDepthTexture();

        //update "camera"
        uniforms.projectionMatrix = glm::perspective(glm::radians(45.0f), static_cast<float>(event.windowRect.width) /
                                                                          static_cast<float>(event.windowRect.height),
                                                     0.1f, 100.0f);
        uniformBuffer->UpdateBufferData(&uniforms, sizeof(Ajiva::Renderer::UniformData));
        return false;
    }

    void Application::BuildSwapChain() {
        if (swapChain) {
            swapChain->release();
        }

        swapChain = context->CreateSwapChain(window->GetWidth(), window->GetHeight());
    }

    void Application::BuildDepthTexture() {
        //the ref should auto release
        depthTexture = context->CreateDepthTexture(
                {static_cast<uint32_t>(window->GetWidth()), static_cast<uint32_t>(window->GetHeight()), 1});
        PLOG_INFO << "Depth Texture " << depthTexture->texture << " " << depthTexture->view;
    }
}