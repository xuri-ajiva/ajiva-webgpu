#pragma once

#include "defines.h"
#include <chrono>

namespace Ajiva::Core
{
    struct AJ_API Clock
    {
        using ClockType = std::chrono::high_resolution_clock;
        using TimePoint = ClockType::time_point;
        using Duration = ClockType::duration;

    private:
        TimePoint m_start{};
        TimePoint m_last{};
        Duration m_delta{};
        u64 m_ticks{};

    public:
        void Start();

        void Reset();

        void Update();

        [[nodiscard]] Duration Delta() const;

        [[nodiscard]] Duration Total() const;

        [[nodiscard]] u64 Ticks() const;
    };
}
