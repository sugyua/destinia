#pragma once
// A read-only, thread-safe freeze monitor. Never access Unicorn from the monitor thread.
// Times are monotonic; function-entry PCs are checkpoints, not sampled live PCs.
#include <atomic>
#include <chrono>
#include <cstdint>

namespace destinia {
inline std::atomic<uint32_t> g_lastGuestFunctionEntryPc{0};
enum class StallPhase : uint32_t { Idle=0, Arm32=1, Pixels=2, Diagnostics=3 };
struct StallSnapshot {
    bool active=false;
    uint32_t frame=0;
    uint32_t previousGuestDelayMs=0;
    uint32_t lastGuestFunctionEntryPc=0;
    StallPhase phase=StallPhase::Idle;
    double elapsedMs=0.0;
    unsigned severity=0;
};
class StallWatchdog {
    std::atomic<uint64_t> startedNs_{0};
    std::atomic<uint32_t> frame_{0};
    std::atomic<uint32_t> previousGuestDelayMs_{0};
    std::atomic<uint32_t> phase_{0};
    std::atomic<unsigned> reportedSeverity_{0};
public:
    static uint64_t nowNs() {
        using namespace std::chrono;
        return static_cast<uint64_t>(duration_cast<nanoseconds>(
            steady_clock::now().time_since_epoch()).count());
    }
    void beginAt(uint32_t frame,uint32_t guestDelay,uint64_t startedNs) {
        startedNs_.store(0,std::memory_order_release);
        frame_.store(frame,std::memory_order_relaxed);
        previousGuestDelayMs_.store(guestDelay,std::memory_order_relaxed);
        phase_.store(static_cast<uint32_t>(StallPhase::Arm32),std::memory_order_relaxed);
        g_lastGuestFunctionEntryPc.store(0,std::memory_order_relaxed);
        reportedSeverity_.store(0,std::memory_order_relaxed);
        startedNs_.store(startedNs,std::memory_order_release);
    }
    void begin(uint32_t frame,uint32_t guestDelay) {
        beginAt(frame,guestDelay,nowNs());
    }
    void phase(StallPhase value) {
        phase_.store(static_cast<uint32_t>(value),std::memory_order_relaxed);
    }
    void complete() {
        startedNs_.store(0,std::memory_order_release);
        phase_.store(static_cast<uint32_t>(StallPhase::Idle),std::memory_order_relaxed);
    }
    StallSnapshot snapshotAt(uint64_t now) const {
        StallSnapshot s;
        const uint64_t started=startedNs_.load(std::memory_order_acquire);
        if (!started || now<started) return s;
        s.active=true;
        s.frame=frame_.load(std::memory_order_relaxed);
        s.previousGuestDelayMs=previousGuestDelayMs_.load(std::memory_order_relaxed);
        s.phase=static_cast<StallPhase>(phase_.load(std::memory_order_relaxed));
        s.lastGuestFunctionEntryPc=g_lastGuestFunctionEntryPc.load(std::memory_order_relaxed);
        s.elapsedMs=static_cast<double>(now-started)/1000000.0;
        s.severity=s.elapsedMs>=5000.0?3u:s.elapsedMs>=2000.0?2u:s.elapsedMs>=500.0?1u:0u;
        // If the emulation ended while we were copying fields, reject the stale sample.
        if (startedNs_.load(std::memory_order_acquire)!=started) return StallSnapshot{};
        return s;
    }
    StallSnapshot snapshot() const { return snapshotAt(nowNs()); }
    bool pollAt(uint64_t now,StallSnapshot &sample) {
        sample=snapshotAt(now);
        if (!sample.active || sample.severity==0) return false;
        unsigned old=reportedSeverity_.load(std::memory_order_relaxed);
        while (old<sample.severity) {
            if (reportedSeverity_.compare_exchange_weak(
                    old,sample.severity,std::memory_order_relaxed))
                return true;
        }
        return false;
    }
    bool poll(StallSnapshot &sample) { return pollAt(nowNs(),sample); }
};
} // namespace destinia
