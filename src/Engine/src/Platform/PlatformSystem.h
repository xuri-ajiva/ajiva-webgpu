//
// Created by XuriAjiva on 13.08.2023.
//

#pragma once

#include "Platform/Window.h"

namespace Ajiva::Platform {
    class PlatformSystem {
    public:
        static bool Init();
        static void Shutdown();
    };
} // Ajiva::Platform
