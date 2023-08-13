


#include "PlatformSystem.h"

#define AJ_INIT_CHECK(name) if(!Ajiva::Platform::name::Init()) {\
 PLOG_FATAL << "Failed to Init " << #name << "!" << std::endl;  \
 return false;                                                  \
 }
#define AJ_SHUTDOWN(name) Ajiva::Platform::name::Shutdown();

namespace Ajiva::Platform {
    bool PlatformSystem::Init() {
        AJ_INIT_CHECK(Window)
        return true;
    }

    void PlatformSystem::Shutdown() {
        AJ_SHUTDOWN(Window)
    }
}
