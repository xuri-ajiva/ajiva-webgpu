


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


int main() {
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
        if (!app.Init()) {
            PLOG_FATAL << "Failed to Init Application";
            return -1;
        }

        while (app.IsRunning()) {
            app.Frame();
        }

        app.Finish();
    }

    Ajiva::Platform::PlatformSystem::Shutdown();

    AJ_CheckForLeaks();


    return 0;
}
