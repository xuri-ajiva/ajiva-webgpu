//
// Created by XuriAjiva on 31.07.2023.
//
#pragma once

#include "defines.h"
#define PLOG_CAPTURE_FILE
#include <plog/Log.h>

namespace Ajiva::Core {
    AJ_API void ShowAppLog(bool *p_open);

    AJ_API void SetupLogger();
}

