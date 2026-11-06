#include "frame_pacing.h"
#include <cassert>
#include <chrono>
#include <iostream>
int main() {
    using namespace std::chrono;
    const auto start=steady_clock::time_point{};
    const auto wait=[&](nanoseconds elapsed,uint32_t guestMs) {
        return destinia::nextFrameWait(start,start+elapsed,guestMs);
    };
    assert(wait(microseconds(56100),60)==microseconds(6400));
    assert(wait(milliseconds(56),50)==microseconds(6500));
    assert(wait(milliseconds(56),65)==milliseconds(9));
    assert(wait(milliseconds(20),1000)==milliseconds(980));
    assert(wait(microseconds(63000),60)==milliseconds(1));
    assert(wait(milliseconds(80),60)==milliseconds(1));
    assert(wait(nanoseconds(56000001),60)==nanoseconds(6499999));
    assert(wait(nanoseconds(0),1)==microseconds(62500));
    assert(wait(nanoseconds(0),10000)==milliseconds(1000));
    for(uint32_t guestMs: {1u,60u,62u,63u,65u,250u,1000u}) {
        for(int64_t us:{0,1000,10000,56000,62500,80000}) {
            auto pending=wait(microseconds(us),guestMs);
            assert(pending>=milliseconds(1));
            if(us<62500)
                assert(microseconds(us)+pending>=microseconds(62500));
        }
    }
    std::cout<<"frame_pacing: PASS: original 16Hz cap and longer pauses\n";
}
