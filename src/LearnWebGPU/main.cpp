#define WEBGPU_CPP_IMPLEMENTATION

#include "webgpu/webgpu.hpp"

#include <glfw3webgpu.h>
#include "Window.hpp"
#include "GpuContext.hpp"

int main() {
    using namespace wgpu;
    std::cout << "Hello, World!" << std::endl;

    Ajiva::Window window;
    window.Run();
    Ajiva::GpuContext gpuContext(window.CreateSurfaceFunk());

    auto swapChain = gpuContext.CreateSwapChain(window.GetWidth(), window.GetHeight());

    std::unique_ptr<ShaderModule> shaderModule = gpuContext.CreateShaderModule(R"(
@vertex
fn vs_main(
    @builtin(vertex_index) in_vertex_index: u32
) -> @builtin(position) vec4<f32> {
	var p = vec2f(0.0, 0.0);
	if (in_vertex_index == 0u) {
		p = vec2f(-0.5, -0.5);
	} else if (in_vertex_index == 1u) {
		p = vec2f(0.5, -0.5);
	} else {
		p = vec2f(0.0, 0.5);
	}
	return vec4f(p, 0.0, 1.0);
}

@fragment
fn fs_main() -> @location(0) vec4f {
    return vec4f(0.0, 0.4, 1.0, 1.0);
}
)");

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
        renderPass.draw(3, 1, 0, 0);
        renderPass.end();

        nextTexture.release();

        gpuContext.SubmitEncoder(encoder);

        swapChain->present();
    }

    swapChain->release();
}
