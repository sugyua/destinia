#include "deep_telemetry.h"
#include <cassert>
#include <string>
int main() {
    destinia::DeepTelemetry hist;
    destinia::DeepFrame f;
    f.frame=1; f.armMs=50; f.gapMs=63; f.requestedNextMs=60;
    hist.record(f);
    f.frame=2; f.armMs=280; f.gapMs=282; f.requestedNextMs=1000;
    f.lastGuestEntryPc=0x1000bb90u; f.uiQueueDelayMs=3.5f;
    hist.record(f);
    f.frame=3; f.armMs=2300; f.gapMs=2310; f.profiled=true;
    f.requestedNextMs=60; // This frame did not request another one-second pause.
    hist.record(f);
    std::string out=hist.reportText();
    assert(out.find("frames_total=3")!=std::string::npos);
    assert(out.find("slow_gap_70=2")!=std::string::npos);
    assert(out.find("arm_over2000=1")!=std::string::npos);
    assert(out.find("next_pause_500=1")!=std::string::npos);
    assert(out.find("profile_frames=1")!=std::string::npos);
    assert(out.find("checkpoint_pc=0x1000bb90")!=std::string::npos);
    assert(out.find("arm_unprofiled_p50=50.00")!=std::string::npos);
    assert(out.find("arm_unprofiled_p99=280.00")!=std::string::npos);
    // Export is bounded to the most recent 4096 frames but retains session counts.
    for(unsigned i=4;i<=4210;++i) {
        f=destinia::DeepFrame{};
        f.frame=i;
        f.armMs=45;
        hist.record(f);
    }
    out=hist.reportText();
    assert(out.find("frames_total=4210")!=std::string::npos);
    assert(out.find("frames_retained=4096")!=std::string::npos);
    assert(out.find("DEEP_FRAME uptime_ms=0 frame=1 ")==std::string::npos);
    assert(out.find("DEEP_FRAME uptime_ms=0 frame=4210 ")!=std::string::npos);
    assert(out.size()<1100000);
}
