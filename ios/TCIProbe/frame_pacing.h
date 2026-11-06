#pragma once
#include <algorithm>
#include <chrono>
#include <cstdint>

namespace destinia {
// Preserve original 16Hz gameplay pace, honor longer ARM32 pauses,
// and never attempt catch-up updates that could accelerate game logic.
inline std::chrono::nanoseconds nextFrameWait(
    std::chrono::steady_clock::time_point started,
    std::chrono::steady_clock::time_point now,
    uint32_t requestedMs) {
    using namespace std::chrono;
    constexpr nanoseconds kLogicPeriod(62500000);
    constexpr nanoseconds kMinimumWakeWait(1000000);
    const auto guestWait=duration_cast<nanoseconds>(
        milliseconds(std::min(requestedMs,1000u)));
    const auto period=std::max(kLogicPeriod,guestWait);
    const auto elapsed=duration_cast<nanoseconds>(now-started);
    return elapsed>=period ? kMinimumWakeWait
        : std::max(kMinimumWakeWait,period-elapsed);
}
} // namespace destinia
