//
// Created by XuriAjiva on 31.07.2023.
//
#pragma once

#include "defines.h"
#include <plog/Log.h>

using namespace plog;

namespace Ajiva::Core {
    AJ_API void ShowAppLog(bool *p_open);

    AJ_API void SetupLogger();
}
