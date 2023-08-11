


#include <iostream>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED

#include <glm/ext.hpp>

#define WEBGPU_CPP_IMPLEMENTATION

#include "webgpu/webgpu.hpp"

#include "Platform/Window.h"
#include "Renderer/GpuContext.cpp"

#include "defines.h"
#include "Core/Logger.h"
#include "Core/Clock.h"

#include "Resource/Loader.h"


int main() {
    using namespace Ajiva;
    using namespace Ajiva::Renderer;
    using namespace Ajiva::Resource;

    Core::SetupLogger();
    PLOG_INFO << "Hello, World!";

    Core::Clock clock;
    clock.Start();
    Window window(1920, 1080, "Ajiva Engine");
    window.Create();
    GpuContext context(window.CreateSurfaceFunk());
    auto contextRef = CreateRef<GpuContext>(context);
    window.Run();


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

    auto swapChain = context.CreateSwapChain(window.GetWidth(), window.GetHeight());

    Ref<wgpu::ShaderModule> shaderModule = context.CreateShaderModuleFromCode(
            Loader::LoadFile(std::filesystem::path(RESOURCE_DIR "/shader.wgsl")));

    // Vertex fetch
    // We now have 2 attributes
    std::vector<wgpu::VertexAttribute> vertexAttribs = {
            WGPUVertexAttribute{ //todo macro this
                    .format = wgpu::VertexFormat::Float32x3,
                    .offset = offsetof(VertexData, position),
                    .shaderLocation = 0,
            },
            WGPUVertexAttribute{
                    .format = wgpu::VertexFormat::Float32x3,
                    .offset = offsetof(VertexData, normal),
                    .shaderLocation = 1,
            },
            WGPUVertexAttribute{
                    .format = wgpu::VertexFormat::Float32x3,
                    .offset = offsetof(VertexData, color),
                    .shaderLocation = 2,
            },
            WGPUVertexAttribute{
                    .format = wgpu::VertexFormat::Float32x2,
                    .offset = offsetof(VertexData, uv),
                    .shaderLocation = 3,
            },
    };

/*    auto depthTexture = context.CreateTexture(TextureFormat::Depth24Plus, {static_cast<uint32_t>(window.GetWidth()),
                                                                           static_cast<uint32_t>(window.GetHeight()),
                                                                           1});*/

    // Create the depth texture
    Ref<Ajiva::Renderer::Texture> depthTexture = context.CreateDepthTexture(
            {static_cast<uint32_t>(window.GetWidth()), static_cast<uint32_t>(window.GetHeight()), 1});
    PLOG_INFO << "Depth texture format: " << depthTexture->textureFormat;

    std::vector<wgpu::BindGroupLayoutEntry> bindingLayoutEntries(2, wgpu::Default);
    bindingLayoutEntries[0] = WGPUBindGroupLayoutEntry{
            .binding = 0,
            .visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment,
            .buffer = WGPUBufferBindingLayout{
                    .type = wgpu::BufferBindingType::Uniform,
                    .minBindingSize = sizeof(UniformData),
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

    auto bindGroupLayout = context.CreateBindGroupLayout(bindingLayoutEntries);
    auto renderPipeline = context.CreateRenderPipeline(shaderModule, std::vector{*bindGroupLayout}, vertexAttribs,
                                                       depthTexture->textureFormat);

    std::vector<VertexData> vertexData;
    std::vector<uint16_t> indexData;

    /*bool success = Loader::LoadGeometryFromSimpleTxt(RESOURCE_DIR "/pyramid.txt", vertexData, indexData);
    if (!success) {
        PLOG_ERROR << "Could not load geometry!";
        return 1;
    }*/
    bool success = Loader::LoadGeometryFromObj(RESOURCE_DIR "/cube.obj", vertexData, indexData);
    if (!success) {
        std::cerr << "Could not load geometry!" << std::endl;
        return 1;
    }
    UniformData uniforms = {
            .color = {1.0f, 0.5f, 0.0f, 1.0f},
            .time = 1.0f,
    };

    auto textureTest = context.CreateTexture(wgpu::TextureFormat::RGBA8Unorm, {128, 128, 1},
                                             static_cast<const WGPUTextureUsage>(wgpu::TextureUsage::TextureBinding |
                                                                                 wgpu::TextureUsage::CopyDst),
                                             wgpu::TextureAspect::All, "Texture Test");
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
    textureTest->WriteTexture(pixels.data(), pixels.size());

    constexpr float PI = 3.14159265358979323846f;
    using glm::mat4x4;
    using glm::vec4;
    using glm::vec3;
    using glm::cos;
    using glm::sin;


    vec3 eye = {0, 0, 0};
    vec3 pos = {1, 1, 2};
    mat4x4 V = glm::lookAt(pos, eye, vec3{0, 0, 1}); //z-up, (left handed ?? )
    uniforms.viewMatrix = V;

    float ratio = static_cast<float>(window.GetWidth()) / static_cast<float>(window.GetHeight());
    float fov = 45.0f * PI / 180.0f;
    float near = 0.01f;
    float far = 100.0f;
    uniforms.projectionMatrix = glm::perspective(fov, ratio, near, far);

    auto vertexBuffer = context.CreateFilledBuffer(vertexData.data(),
                                                   vertexData.size() * sizeof(VertexData),
                                                   wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex,
                                                   "Vertex Buffer");
    /*auto indexBuffer = context.CreateFilledBuffer(indexData.data(), indexData.size() * sizeof(uint16_t),
                                                  wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index,
                                                  "Index Buffer");*/
    auto uniformBuffer = context.CreateFilledBuffer(&uniforms, sizeof(UniformData),
                                                    wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform,
                                                    "Uniform Buffer");


    std::vector<wgpu::BindGroupEntry> bindings(2);
    bindings[0] = WGPUBindGroupEntry{
            .binding = 0,
            .buffer = uniformBuffer->buffer,
            .offset = 0,
            .size = sizeof(UniformData),
    };
    bindings[1] = WGPUBindGroupEntry{
            .binding = 1,
            .textureView = textureTest->view
    };


    auto bindGroup = context.CreateBindGroup(bindGroupLayout, uniformBuffer, bindings);

    PLOG_DEBUG << std::chrono::duration_cast<std::chrono::milliseconds>(clock.Total());
    //AJ_INFO("Startup Time: %s", clock.Total());
    clock.Update();
    static bool show_demo_window = true;
    static bool app_log_open = true;

    while (!window.IsClosed()) {
        clock.Update();
        //BeforeRender

        // Update view matrix

        std::chrono::high_resolution_clock::duration delta = clock.Total();
        uniforms.time = std::chrono::duration_cast<std::chrono::duration<float>>(delta).count();
        /*uniformData.modelMatrix = glm::rotate(mat4x4(1.0), uniformData.time, vec3(0.0, 0.0, 1.0)) *
                                  glm::translate(mat4x4(1.0), vec3(0.5, 0.0, 0.0)) *
                                  glm::scale(mat4x4(1.0), vec3(0.8f));*/
        uniforms.modelMatrix = mat4x4(1.0);
        uniforms.viewMatrix = glm::lookAt(vec3(-0.5f, -2.5f, 2.0f), vec3(0.0f),
                                          vec3(0, 0, 1)); // the last argument indicates our Up direction convention
        uniforms.projectionMatrix = glm::perspective(45 * PI / 180, 640.0f / 480.0f, 0.01f, 100.0f);
        uniformBuffer->UpdateBufferData(&uniforms, sizeof(UniformData));

        //io.DisplaySize = ImVec2((float) window.GetWidth(), (float) window.GetHeight());
        ImGui_ImplWGPU_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        ImGui::ShowDemoWindow(&show_demo_window);
        Core::ShowAppLog(&app_log_open);

        ImGui::Render();

        wgpu::TextureView nextTexture = swapChain->getCurrentTextureView();
        //std::cout << "nextTexture: " << nextTexture << std::endl;

        if (!nextTexture) {
            std::cerr << "Cannot acquire next swap chain texture" << std::endl;
            break;
        }

        wgpu::CommandEncoder encoder = context.CreateCommandEncoder();

        wgpu::RenderPassEncoder renderPass = context.CreateRenderPassEncoder(encoder, nextTexture,
                                                                             depthTexture->view,
                                                                             {0.4, 0.4, 0.4, 1.0});
        renderPass.setPipeline(*renderPipeline);

        renderPass.setVertexBuffer(0, vertexBuffer->buffer, 0, vertexData.size() * sizeof(VertexData));
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

        // AfterRender
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow *backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }

    swapChain->release();

    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplWGPU_Shutdown();
    return 0;
}
