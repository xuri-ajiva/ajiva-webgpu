//
// Created by XuriAjiva on 31.07.2023.
//
#pragma once

#include "defines.h"

namespace Ajiva::Core
{
    AJ_API void ShowAppLog(bool* p_open);

    AJ_API void SetupLogger();
}

#define AJ_FAIL(...) PLOG_FATAL << __VA_ARGS__; throw std::runtime_error(__VA_ARGS__)
