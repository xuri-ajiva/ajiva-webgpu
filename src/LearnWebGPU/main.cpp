#define WEBGPU_CPP_IMPLEMENTATION

#include "webgpu/webgpu.hpp"

#include <glfw3webgpu.h>
#include "Window.hpp"
#include "GpuContext.hpp"
#include <filesystem>

int main() {
    using namespace wgpu;
    std::cout << "Hello, World!" << std::endl;

    Ajiva::Window window;
    window.Run();
    Ajiva::GpuContext gpuContext(window.CreateSurfaceFunk());

    auto swapChain = gpuContext.CreateSwapChain(window.GetWidth(), window.GetHeight());

    std::unique_ptr<ShaderModule> shaderModule = gpuContext.CreateShaderModuleFromFile(std::filesystem::path(RESOURCE_DIR "/shader.wgsl"));

    std::vector<Ajiva::Vertex> vertexData;
    std::vector<uint16_t> indexData;

    bool success = gpuContext.LoadGeometry(RESOURCE_DIR "/webgpu.txt", vertexData, indexData);
    if (!success) {
        std::cerr << "Could not load geometry!" << std::endl;
        return 1;
    }

    auto vertexBuffer = gpuContext.CreateFilledBuffer(vertexData.data(), vertexData.size() * sizeof(Ajiva::Vertex),
                                                      BufferUsage::CopyDst | BufferUsage::Vertex);
    auto indexBuffer = gpuContext.CreateFilledBuffer(indexData.data(), indexData.size() * sizeof(uint16_t),
                                                     BufferUsage::CopyDst | BufferUsage::Index);
    auto renderPipeline = gpuContext.CreateRenderPipeline(shaderModule);

    while (!window.IsClosed()) {
        TextureView nextTexture = swapChain->getCurrentTextureView();
        //std::cout << "nextTexture: " << nextTexture << std::endl;

        if (!nextTexture) {
            std::cerr << "Cannot acquire next swap chain texture" << std::endl;
            break;
        }

        CommandEncoder encoder = gpuContext.CreateCommandEncoder();

        RenderPassEncoder renderPass = gpuContext.CreateRenderPassEncoder(encoder, nextTexture);
        renderPass.setPipeline(*renderPipeline);

        // Set both vertex and index buffers
        renderPass.setVertexBuffer(0, *vertexBuffer, 0, vertexData.size() * sizeof(Ajiva::Vertex));
        // The second argument must correspond to the choice of uint16_t or uint32_t
        // we've done when creating the index buffer.
        renderPass.setIndexBuffer(*indexBuffer, IndexFormat::Uint16, 0, indexData.size() * sizeof(uint16_t));

        // Replace `draw()` with `drawIndexed()` and `vertexCount` with `indexCount`
        // The extra argument is an offset within the index buffer.
        renderPass.drawIndexed(indexData.size(), 1, 0, 0, 0);

        renderPass.end();

        nextTexture.release();

        gpuContext.SubmitEncoder(encoder);

        swapChain->present();
    }

    swapChain->release();
}
