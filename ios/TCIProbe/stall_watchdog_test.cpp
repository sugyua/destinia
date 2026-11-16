#include "stall_watchdog.h"
#include <cassert>
#include <cstring>
int main() {
    using destinia::StallPhase;
    destinia::StallWatchdog w;
    destinia::StallSnapshot s;
    constexpr uint64_t start=1000000000ULL;
    assert(std::strcmp(destinia::stallCheckpointName(0x10007c5c), "jniRun")==0);
    assert(std::strcmp(destinia::stallCheckpointName(0x1000bb91), "Grp_drawClipWImageP")==0);
    assert(std::strcmp(destinia::stallCheckpointName(0x1000ab40), "Grp_fillRect")==0);
    assert(std::strcmp(destinia::stallCheckpointName(0x10034454), "_drawMiniMap")==0);
    assert(std::strcmp(destinia::stallCheckpointName(0x1000b8d4), "Grp_drawWPNImage2")==0);
    assert(std::strcmp(destinia::stallCheckpointName(0x1000b4bc), "Grp_drawSimpleClipWImageP")==0);
    assert(std::strcmp(destinia::stallCheckpointName(0x10009868), "_Grp_drawScreen16")==0);
    assert(std::strcmp(destinia::stallCheckpointName(0x10009a00), "_Grp_drawAlpScreen16")==0);
    assert(std::strcmp(destinia::stallCheckpointName(0x1000bd90), "unknown")==0);
    assert(!w.pollAt(start,s));
    w.beginAt(42,60,start);
    assert(!w.pollAt(start+499000000ULL,s));
    assert(w.pollAt(start+500000000ULL,s) && s.severity==1);
    assert(s.frame==42 && s.previousGuestDelayMs==60 && s.phase==StallPhase::Arm32);
    assert(!w.pollAt(start+600000000ULL,s));
    destinia::g_lastGuestFunctionEntryPc.store(0x1000bd90u);
    assert(w.pollAt(start+2000000000ULL,s) && s.severity==2);
    assert(s.lastGuestFunctionEntryPc==0x1000bd90u);
    w.phase(StallPhase::Pixels);
    assert(w.pollAt(start+5000000000ULL,s) && s.severity==3);
    assert(s.phase==StallPhase::Pixels);
    assert(!w.pollAt(start+7000000000ULL,s));
    w.complete();
    assert(!w.pollAt(start+8000000000ULL,s) && !s.active);
    w.beginAt(43,1000,start+9000000000ULL);
    assert(w.pollAt(start+9600000000ULL,s) && s.frame==43 && s.severity==1);
}
