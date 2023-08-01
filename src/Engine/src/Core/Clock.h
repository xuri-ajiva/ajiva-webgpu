#pragma once

#include "defines.h"
#include <chrono>

namespace Ajiva::Core {
    struct Clock {
        using ClockType = std::chrono::high_resolution_clock;
        using TimePoint = ClockType::time_point;
        using Duration = ClockType::duration;

    private:
        TimePoint m_start{};
        TimePoint m_last{};
        Duration m_delta{};

    public:
        AJ_API void Start();

        AJ_API void Update();

        [[nodiscard]] AJ_API Duration Delta() const;

        [[nodiscard]] AJ_API Duration Total() const;
    };
}