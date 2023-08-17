#include "Clock.h"

using ClockType = std::chrono::high_resolution_clock;
using TimePoint = ClockType::time_point;
using Duration = ClockType::duration;

void Ajiva::Core::Clock::Start() {
    m_start = ClockType::now();
    m_last = m_start;
}

void Ajiva::Core::Clock::Update() {
    auto now = ClockType::now();
    m_delta = now - m_last;
    m_last = now;
    m_ticks++;
}

[[nodiscard]] Duration Ajiva::Core::Clock::Delta() const {
    return m_delta;
}

[[nodiscard]] Duration Ajiva::Core::Clock::Total() const {
    return ClockType::now() - m_start;
}

void Ajiva::Core::Clock::Reset() {
    m_start = ClockType::now();
    m_last = m_start;
    m_delta = Duration{};
    m_ticks = 0;
}

u64 Ajiva::Core::Clock::Ticks() const {
    return m_ticks;
}
