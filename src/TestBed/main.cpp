


#include "library.h"
#include <iostream>

#define WEBGPU_CPP_IMPLEMENTATION

#include "webgpu/webgpu.hpp"

#include "Platform/Window.h"
#include "Renderer/GpuContext.hpp"

#include "defines.h"
#include "Core/Logger.h"
#include "Core/Clock.h"

int main() {
    using namespace Ajiva;
    using namespace Ajiva::Renderer;

    Core::SetupLogger();
    PLOG_INFO << "Hello, World!";

    Core::Clock clock;
    clock.Start();
    Window window(1280, 720, false);
    window.Create();
    GpuContext context(window.CreateSurfaceFunk());
    window.Run();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

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

    std::unique_ptr<ShaderModule> shaderModule = context.CreateShaderModuleFromFile(
            std::filesystem::path(RESOURCE_DIR "/shader.wgsl"));


    auto bindGroupLayout = context.CreateBindGroupLayout();
    auto renderPipeline = context.CreateRenderPipeline(shaderModule, std::vector{*bindGroupLayout});

    std::vector<VertexData> vertexData;
    std::vector<uint16_t> indexData;

    bool success = GpuContext::LoadGeometry(RESOURCE_DIR "/webgpu.txt", vertexData, indexData);
    if (!success) {
        std::cerr << "Could not load geometry!" << std::endl;
        return 1;
    }
    UniformData uniformData = {
            .color = {1.0f, 0.5f, 0.0f, 1.0f},
            .time = 1.0f,
            .aspectRatio = static_cast<float>(window.GetWidth()) / static_cast<float>(window.GetHeight()),
    };


    auto vertexBuffer = context.CreateFilledBuffer(vertexData.data(), vertexData.size() * sizeof(VertexData),
                                                   BufferUsage::CopyDst | BufferUsage::Vertex);
    auto indexBuffer = context.CreateFilledBuffer(indexData.data(), indexData.size() * sizeof(uint16_t),
                                                  BufferUsage::CopyDst | BufferUsage::Index);
    auto uniformBuffer = context.CreateFilledBuffer(&uniformData, sizeof(UniformData),
                                                    BufferUsage::CopyDst | BufferUsage::Uniform);

    auto bindGroup = context.CreateBindGroup(bindGroupLayout, uniformBuffer);

    std::cout << clock.Total() << std::endl;
    std::cout << "Seconds: " << std::chrono::duration_cast<std::chrono::seconds>(clock.Total()) << std::endl;
    std::cout << "Seconds: " << std::chrono::duration_cast<std::chrono::microseconds>(clock.Total()) << std::endl;
    //AJ_INFO("Startup Time: %s", clock.Total());
    clock.Update();
    static bool show_demo_window = true;
    static bool app_log_open = true;

    while (!window.IsClosed()) {
        clock.Update();
        //BeforeRender

        //io.DisplaySize = ImVec2((float) window.GetWidth(), (float) window.GetHeight());
        ImGui_ImplWGPU_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();



        ImGui::ShowDemoWindow(&show_demo_window);
        Core::ShowAppLog(&app_log_open);

        ImGui::Render();

        TextureView nextTexture = swapChain->getCurrentTextureView();
        //std::cout << "nextTexture: " << nextTexture << std::endl;
        // glfwGetTime returns a double
        uniformData.time = static_cast<float>(glfwGetTime());
        context.UpdateBufferData(uniformBuffer, &uniformData, sizeof(UniformData));

        if (!nextTexture) {
            std::cerr << "Cannot acquire next swap chain texture" << std::endl;
            break;
        }

        CommandEncoder encoder = context.CreateCommandEncoder();

        RenderPassEncoder renderPass = context.CreateRenderPassEncoder(encoder, nextTexture, {0.1, 0.1, 0.1, 0.0});
        renderPass.setPipeline(*renderPipeline);

        // Set both vertex and index buffers
        renderPass.setVertexBuffer(0, *vertexBuffer, 0, vertexData.size() * sizeof(VertexData));

        // The second argument must correspond to the choice of uint16_t or uint32_t
        // we've done when creating the index buffer.
        renderPass.setIndexBuffer(*indexBuffer, IndexFormat::Uint16, 0, indexData.size() * sizeof(uint16_t));

        // Set binding group
        renderPass.setBindGroup(0, *bindGroup, 0, nullptr);

        // Replace `draw()` with `drawIndexed()` and `vertexCount` with `indexCount`
        // The extra argument is an offset within the index buffer.
        renderPass.drawIndexed(indexData.size(), 1, 0, 0, 0);

        ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPass);

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
