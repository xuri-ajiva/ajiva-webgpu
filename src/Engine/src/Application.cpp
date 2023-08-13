//
// Created by XuriAjiva on 13.08.2023.
//

#include "Application.h"
#include "Resource/ResourceManager.h"
#include <glm/ext.hpp>

namespace Ajiva {

    Application::~Application() {

    }

    Application::Application(ApplicationConfig config) : config(config), loader(config.resourceDirectory),
                                                         window(config.startWidth, config.startHeight,
                                                                config.multiThread) {
    }

    bool Application::Init() {
        Core::SetupLogger();
        PLOG_INFO << "Hello, World!";
        clock.Start();;
        window.Create();
        if (!context.Init(window.CreateSurfaceFunk())) return false;
        window.Run();

        ///Foreach layer init
        SetupImGui();
        SetupPipeline();
        return true;
    }

    void Application::SetupImGui() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void) io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsLight();

        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        ImGuiStyle &style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOther(window.GetWindow(), true);
        ImGui_ImplWGPU_Init(*context.device, 2, context.swapChainFormat, WGPUTextureFormat_Undefined);
    }

    bool Application::SetupPipeline() {
        swapChain = context.CreateSwapChain(window.GetWidth(), window.GetHeight());

        //todo move to member??? or not
        Ref<wgpu::ShaderModule> shaderModule = context.CreateShaderModuleFromCode(loader.LoadFile("shader.wgsl"));

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

/*    auto depthTexture = context.CreateTexture(TextureFormat::Depth24Plus, {static_cast<uint32_t>(window.GetWidth()),
                                                                           static_cast<uint32_t>(window.GetHeight()),
                                                                           1});*/

        // Create the depth texture
        depthTexture = context.CreateDepthTexture(
                {static_cast<uint32_t>(window.GetWidth()), static_cast<uint32_t>(window.GetHeight()), 1});
        PLOG_INFO << "Depth texture format: " << depthTexture->textureFormat;

        std::vector<wgpu::BindGroupLayoutEntry> bindingLayoutEntries(3, wgpu::Default);
        bindingLayoutEntries[0] = WGPUBindGroupLayoutEntry{
                .binding = 0,
                .visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment,
                .buffer = WGPUBufferBindingLayout{
                        .type = wgpu::BufferBindingType::Uniform,
                        .minBindingSize = sizeof(Ajiva::Renderer::UniformData),
                },
        };
        bindingLayoutEntries[1] = WGPUBindGroupLayoutEntry{
                .binding = 1,
                .visibility = wgpu::ShaderStage::Fragment,
                .texture = WGPUTextureBindingLayout{
                        .sampleType = wgpu::TextureSampleType::Float,
                        .viewDimension = wgpu::TextureViewDimension::_2D,
                },
        };
        bindingLayoutEntries[2] = WGPUBindGroupLayoutEntry{
                .binding = 2,
                .visibility = wgpu::ShaderStage::Fragment,
                .sampler = WGPUSamplerBindingLayout{
                        .type = wgpu::SamplerBindingType::Filtering,
                },
        };

        auto bindGroupLayout = context.CreateBindGroupLayout(bindingLayoutEntries);
        renderPipeline = context.CreateRenderPipeline(shaderModule, std::vector{*bindGroupLayout}, vertexAttribs,
                                                      depthTexture->textureFormat);

        /*bool success = loader.LoadGeometryFromSimpleTxt( "pyramid.txt", vertexData, indexData);
        if (!success) {
            PLOG_ERROR << "Could not load geometry!";
            return 1;
        }*/
        bool success = loader.LoadGeometryFromObj("fourareen.obj", vertexData, indexData);
        if (!success) {
            std::cerr << "Could not load geometry!" << std::endl;
            return false;
        }
        uniforms = {
                .color = {1.0f, 0.5f, 0.0f, 1.0f},
                .time = 1.0f,
        };

        const int mipLevelCount = 8;
/*    auto textureTest = context.CreateTexture(wgpu::TextureFormat::RGBA8Unorm, {256, 256, 1},
                                             static_cast<const WGPUTextureUsage>(wgpu::TextureUsage::TextureBinding |
                                                                                 wgpu::TextureUsage::CopyDst),
                                             wgpu::TextureAspect::All, mipLevelCount, "Texture Test");

    std::vector<uint8_t> pixels(4 * textureTest->size.width * textureTest->size.height);
    for (uint32_t i = 0; i < textureTest->size.width; ++i) {
        for (uint32_t j = 0; j < textureTest->size.height; ++j) {
            uint8_t *p = &pixels[4 * (j * textureTest->size.width + i)];
            p[0] = (i / 16) % 2 == (j / 16) % 2 ? 255 : 0; // r
            p[1] = ((i - j) / 16) % 2 == 0 ? 255 : 0; // g
            p[2] = ((i + j) / 16) % 2 == 0 ? 255 : 0; // b
            p[3] = 255; // a
        }
    }
    textureTest->WriteTextureMips(pixels.data(), pixels.size(), mipLevelCount);*/

        texture = loader.LoadTexture("fourareen2K_albedo.jpg", context);
        if (!texture) {
            AJ_FAIL("Could not load texture!");
            return false;
        }

        constexpr float PI = 3.14159265358979323846f;
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

        float ratio = static_cast<float>(window.GetWidth()) / static_cast<float>(window.GetHeight());
        float fov = 45.0f * PI / 180.0f;
        float near = 0.01f;
        float far = 100.0f;
        uniforms.projectionMatrix = glm::perspective(fov, ratio, near, far);

        vertexBuffer = context.CreateFilledBuffer(vertexData.data(),
                                                  vertexData.size() * sizeof(Ajiva::Renderer::VertexData),
                                                  wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex,
                                                  "Vertex Buffer");
        /*auto indexBuffer = context.CreateFilledBuffer(indexData.data(), indexData.size() * sizeof(uint16_t),
                                                      wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index,
                                                      "Index Buffer");*/
        uniformBuffer = context.CreateFilledBuffer(&uniforms, sizeof(Ajiva::Renderer::UniformData),
                                                   wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform,
                                                   "Uniform Buffer");

        auto sampler = context.CreateSampler(wgpu::AddressMode::Repeat, wgpu::FilterMode::Linear,
                                             wgpu::CompareFunction::Undefined, 0.0f, 1.0f * mipLevelCount, "Sampler");

        std::vector<wgpu::BindGroupEntry> bindings(3);
        bindings[0] = WGPUBindGroupEntry{
                .binding = 0,
                .buffer = uniformBuffer->buffer,
                .offset = 0,
                .size = sizeof(Ajiva::Renderer::UniformData),
        };
        bindings[1] = WGPUBindGroupEntry{
                .binding = 1,
                .textureView = texture->view
        };
        bindings[2] = WGPUBindGroupEntry{
                .binding = 2,
                .sampler = *sampler
        };


        bindGroup = context.CreateBindGroup(bindGroupLayout, uniformBuffer, bindings);

        PLOG_DEBUG << std::chrono::duration_cast<std::chrono::milliseconds>(clock.Total());
        //AJ_INFO("Startup Time: %s", clock.Total());
        clock.Update();
        return true;
    }

    bool Application::IsRunning() {
        return !window.IsClosed();
    }

    void Application::Frame() {
        using glm::mat4x4;
        using glm::vec4;
        using glm::vec3;

        clock.Update();

        ///Before Render
        wgpu::TextureView nextTexture = swapChain->getCurrentTextureView();
        //std::cout << "nextTexture: " << nextTexture << std::endl;

        if (!nextTexture) {
            std::cerr << "Cannot acquire next swap chain texture" << std::endl;
            window.RequestClose();
        }

        //update "camera"
        std::chrono::high_resolution_clock::duration delta = clock.Total();
        uniforms.time = std::chrono::duration_cast<std::chrono::duration<float>>(delta).count();
        uniforms.modelMatrix = glm::rotate(mat4x4(1.0), uniforms.time, vec3(0.0, 0.0, 1.0)) *
                               glm::translate(mat4x4(1.0), vec3(0.5, 0.0, 0.0)) *
                               glm::scale(mat4x4(1.0), vec3(0.8f));
        uniformBuffer->UpdateBufferData(&uniforms, sizeof(Ajiva::Renderer::UniformData));

        BeforeRenderFrameImGui();

        ///Render


        wgpu::CommandEncoder encoder = context.CreateCommandEncoder();

        wgpu::RenderPassEncoder renderPass = context.CreateRenderPassEncoder(encoder, nextTexture,
                                                                             depthTexture->view,
                                                                             {0.4, 0.4, 0.4, 1.0});
        renderPass.setPipeline(*renderPipeline);

        renderPass.setVertexBuffer(0, vertexBuffer->buffer, 0, vertexData.size() * sizeof(Ajiva::Renderer::VertexData));
        /* renderPass.setIndexBuffer(indexBuffer->buffer, wgpu::IndexFormat::Uint16, 0,
                                   indexData.size() * sizeof(uint16_t));*/
        renderPass.setBindGroup(0, *bindGroup, 0, nullptr);
        //renderPass.drawIndexed(indexData.size(), 1, 0, 0, 0);
        renderPass.draw(vertexData.size(), 1, 0, 0);



        //ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPass);


        renderPass.end();

        nextTexture.release();

        context.SubmitEncoder(encoder);

        swapChain->present();

        ///After Render
        AfterRenderFrameImGui();
    }

    inline static bool show_demo_window = true;
    inline static bool app_log_open = true;

    void Application::BeforeRenderFrameImGui() {
        ImGuiIO &io = ImGui::GetIO();
        //io.DisplaySize = ImVec2((float) window.GetWidth(), (float) window.GetHeight());
        ImGui_ImplWGPU_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        ImGui::ShowDemoWindow(&show_demo_window);
        Core::ShowAppLog(&app_log_open);

        ImGui::Render();
    }

    void Application::AfterRenderFrameImGui() {
        ImGuiIO &io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow *backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }

    void Application::Finish() {
        swapChain->release();

        ///Foreach layer Shutdown

        ImGui_ImplGlfw_Shutdown();
        ImGui_ImplWGPU_Shutdown();
    }


}