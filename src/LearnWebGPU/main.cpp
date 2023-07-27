

#define WEBGPU_CPP_IMPLEMENTATION
#include "webgpu/webgpu.hpp"

#include <glfw3webgpu.h>

int main() {
    using namespace wgpu;
    std::cout << "Hello, World!" << std::endl;


    // 2. We create the instance using this descriptor
    Instance instance = createInstance(InstanceDescriptor{});
    // 3. We can check whether there is actually an instance created
    if (!instance) {
        std::cerr << "Could not initialize WebGPU!" << std::endl;
        return 1;
    }

    // 4. Display the object (Instance is a simple pointer, it may be
    // copied around without worrying about its size).
    std::cout << "WebGPU instance: " << instance << std::endl;

    if (!glfwInit()) {
        std::cerr << "Could not initialize GLFW!" << std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(640, 480, "Learn WebGPU", NULL, NULL);
    if (!window) {
        std::cerr << "Could not open window!" << std::endl;
        glfwTerminate();
        return 1;
    }

    Surface surface = glfwGetWGPUSurface(instance, window);

    std::cout << "Requesting adapter..." << std::endl;

    RequestAdapterOptions adapterOpts;
    adapterOpts.compatibleSurface = surface;

    Adapter adapter = instance.requestAdapter(adapterOpts);

    std::cout << "Got adapter: " << adapter << std::endl;

    //inspectAdapter(adapter);

    std::cout << "Requesting device..." << std::endl;

    DeviceDescriptor deviceDesc;
    deviceDesc.label = "My Device"; // anything works here, that's your call
    deviceDesc.requiredFeaturesCount = 0; // we do not require any specific feature
    deviceDesc.defaultQueue.label = "The default queue";
    Device device = adapter.requestDevice(deviceDesc);

    std::cout << "Got device: " << device << std::endl;

    device.setUncapturedErrorCallback([](ErrorType type, char const *message) {
        std::cout << "::error:: type " << type;
        if (message) std::cout << " (" << message << ")";
        std::cout << std::endl;
    });

    //inspectDevice(device);

    Queue queue = device.getQueue();

    queue.onSubmittedWorkDone([](QueueWorkDoneStatus status) {
        std::cout << "Queued work finished with status: " << status << std::endl;
    });


    TextureFormat swapChainFormat = surface.getPreferredFormat(adapter);

    SwapChainDescriptor swapChainDesc;
    swapChainDesc.width = 640;
    swapChainDesc.height = 480;
    swapChainDesc.format = swapChainFormat;
    swapChainDesc.usage = TextureUsage::RenderAttachment;
    swapChainDesc.presentMode = PresentMode::Fifo;

    SwapChain swapChain = device.createSwapChain(surface, swapChainDesc);
    std::cout << "Swapchain: " << swapChain << std::endl;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        TextureView nextTexture = swapChain.getCurrentTextureView();
        //std::cout << "nextTexture: " << nextTexture << std::endl;

        if (!nextTexture) {
            std::cerr << "Cannot acquire next swap chain texture" << std::endl;
            break;
        }

        CommandEncoderDescriptor commandEncoderDesc;
        commandEncoderDesc.label = "Command Encoder";
        CommandEncoder encoder = device.createCommandEncoder(commandEncoderDesc);

        RenderPassColorAttachment renderPassColorAttachment;
        renderPassColorAttachment.view = nextTexture;
        renderPassColorAttachment.loadOp = LoadOp::Clear;
        renderPassColorAttachment.storeOp = StoreOp::Store;
        renderPassColorAttachment.clearValue = Color{0.1, 0.3, 0.7, 1.0};

        RenderPassDescriptor renderPassDesc;
        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &renderPassColorAttachment;
        renderPassDesc.timestampWriteCount = 0;
        RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);
        renderPass.end();

        nextTexture.release();

        CommandBufferDescriptor cmdBufferDescriptor;
        cmdBufferDescriptor.label = "Command buffer";
        CommandBuffer command = encoder.finish(cmdBufferDescriptor);
        queue.submit(1, &command);

        swapChain.present();
    }

    swapChain.release();

    device.release();
    adapter.release();
    surface.release();
    instance.release();

    glfwDestroyWindow(window);
    glfwTerminate();

}
