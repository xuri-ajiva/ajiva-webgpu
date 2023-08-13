


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
#include "Application.h"
#include "Resource/ResourceManager.h"


int main() {
    using namespace Ajiva;
    using namespace Ajiva::Renderer;
    using namespace Ajiva::Resource;


    {
        ApplicationConfig config;
        config.startWidth = 1920;
        config.startHeight = 1080;
        config.multiThread = false;
        config.name = "Ajiva Engine";
        config.resourceDirectory = RESOURCE_DIR;

        Application app(config);
        if (!app.Init()) {
            PLOG_FATAL << "Failed to Init Application";
            return -1;
        }

        while (app.IsRunning()) {
            app.Frame();
        }

        app.Finish();
    }
    AJ_CheckForLeaks();


    return 0;
}
