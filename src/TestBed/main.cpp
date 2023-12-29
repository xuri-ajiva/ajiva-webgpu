#include <iostream>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED

#include <glm/ext.hpp>
#include "webgpu/webgpu.hpp"

#include "Platform/PlatformSystem.h"
#include "Renderer/GpuContext.cpp"

#include "defines.h"
#include "Core/Logger.h"
#include "Core/Clock.h"

#include "Resource/Loader.h"
#include "Application.h"
#include "Resource/ResourceManager.h"


int main()
{
    using namespace Ajiva;
    using namespace Ajiva::Renderer;
    using namespace Ajiva::Resource;

    Ajiva::Platform::PlatformSystem::Init();
    {
        ApplicationConfig config = {
            .WindowConfig = {
                .Width = 1920,
                .Height = 1080,
                .DedicatedThread = false,
                .Name = "Ajiva Engine"
            },
            .ResourceDirectory = RESOURCE_DIR
        };
        Application app(config);
        if (!app.Init())
        {
            PLOG_FATAL << "Failed to Init Application";
            return -1;
        }

        while (app.IsRunning())
        {
            app.Frame();
        }

        app.Finish();
    }

    Ajiva::Platform::PlatformSystem::Shutdown();

    AJ_CheckForLeaks();


    return 0;
}



/*
#include "glfw3webgpu.h"

#define GLFW_INCLUDE_NONE
#include <driverspecs.h>
#include <GLFW/glfw3.h>
#include <webgpu/webgpu.h>
#include <webgpu/webgpu.hpp>
#include <stdio.h>

int main(int argc, char* argv[])
{
    // Init WebGPU
    wgpu::InstanceDescriptor desc;
    desc.nextInChain = NULL;
    wgpu::Instance instance = wgpu::createInstance(desc);

    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(640, 480, "Learn WebGPU", NULL, NULL);

    // Here we create our WebGPU surface from the window!
    wgpu::Surface surface = glfwGetWGPUSurface(instance, window);
    printf("surface = %p", surface);

    wgpu::RequestAdapterOptions options;
    options.nextInChain = NULL;
    options.compatibleSurface = surface;
    wgpu::Adapter adapter = instance.requestAdapter(options);
    wgpu::DeviceDescriptor deviceDesc;
    deviceDesc.nextInChain = NULL;
    auto device = adapter.requestDevice(deviceDesc);

    auto callback = device.setUncapturedErrorCallback([](WGPUErrorType type, const char* message)
    {
        printf("Error %d: %s\n", type, message);
    });


    wgpu::SurfaceConfiguration config;
    config.device = device;
    config.format = surface.getPreferredFormat(adapter);
    config.usage = wgpu::TextureUsage::RenderAttachment;
    config.width = 640;
    config.height = 480;
    config.presentMode = wgpu::PresentMode::Fifo;
    config.nextInChain = NULL;
    config.viewFormatCount = 1;
    config.viewFormats = &config.format;

    wgpuSurfaceConfigure(surface, &config);

    // Terminate GLFW
    while (!glfwWindowShouldClose(window))
    {
        wgpu::SurfaceTexture surfaceTexture;
        surface.getCurrentTexture(&surfaceTexture);
        if(surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::Success)
        {
            printf("SurfaceTexture status: %d\n", surfaceTexture.status);
            break;
        }
        printf("Surface.Suboptimal: %d\n", surfaceTexture.suboptimal);

        WGPUTextureViewDescriptor viewDesc;
        viewDesc.nextInChain = NULL;
        viewDesc.dimension = WGPUTextureViewDimension::WGPUTextureViewDimension_2D;
        viewDesc.format = config.format;
        viewDesc.baseMipLevel = 0;
        viewDesc.mipLevelCount = 1;

        wgpu::TextureView view = wgpuTextureCreateView(surfaceTexture.texture, &viewDesc);


        //instance.processEvents();
        glfwPollEvents();

        view.release();
    }
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
*/
