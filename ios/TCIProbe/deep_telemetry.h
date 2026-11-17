#pragma once
// Bounded, low-overhead, in-memory frame telemetry; no gameplay state or input data.
// Recording uses a single mutex per completed frame; exporting snapshots off-lock.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <mutex>
#include <string>
#include <vector>

namespace destinia {
struct DeepFrame {
    uint64_t uptimeMs=0;
    uint64_t presentationDrops=0;
    uint32_t frame=0;
    uint32_t lastGuestEntryPc=0;
    uint32_t requestedNextMs=0;
    uint32_t previousRequestMs=0;
    float armMs=0;
    float preparationMs=0;
    float gapMs=0;
    float timerLateMs=0;
    float uiQueueDelayMs=0;
    bool profiled=false;
};

class DeepTelemetry {
    static constexpr size_t kCapacity=4096; // ~4 min at 16Hz; longer if slow
    mutable std::mutex lock_;
    std::array<DeepFrame,kCapacity> history_{};
    size_t next_=0, size_=0;
    uint64_t total_=0, slow70_=0, stalls250_=0, stalls500_=0,
             stalls2000_=0, guestPauses_=0, profileFrames_=0;
    double maxArmMs_=0, maxGapMs_=0, maxTimerMs_=0;
    std::array<uint64_t,9> armHistogram_{};
    static size_t histogramBucket(double value) {
        const double limits[]={32,62.5,70,100,250,500,1000,2000};
        for(size_t i=0;i<8;++i) if(value<=limits[i]) return i;
        return 8;
    }
    static double percentile(std::vector<double> values, double p) {
        if(values.empty()) return 0;
        std::sort(values.begin(),values.end());
        size_t index=static_cast<size_t>(std::ceil(p*values.size()));
        if(index==0) index=1;
        return values[std::min(values.size()-1,index-1)];
    }
public:
    void record(const DeepFrame &item) {
        std::lock_guard<std::mutex> guard(lock_);
        history_[next_]=item;
        next_=(next_+1)%kCapacity;
        if(size_<kCapacity) ++size_;
        ++total_;
        const double a=item.armMs;
        if(item.gapMs>70) ++slow70_;
        if(a>250) ++stalls250_;
        if(a>500) ++stalls500_;
        if(a>2000) ++stalls2000_;
        if(item.requestedNextMs>=500) ++guestPauses_;
        if(item.profiled) ++profileFrames_;
        maxArmMs_=std::max(maxArmMs_,a);
        maxGapMs_=std::max(maxGapMs_,static_cast<double>(item.gapMs));
        maxTimerMs_=std::max(maxTimerMs_,static_cast<double>(item.timerLateMs));
        ++armHistogram_[histogramBucket(a)];
    }
    std::string reportText() const {
        std::vector<DeepFrame> frames;
        uint64_t total,slow,over250,over500,over2000,pauses,profile;
        double maxArm,maxGap,maxTimer;
        std::array<uint64_t,9> hist;
        {
            std::lock_guard<std::mutex> guard(lock_);
            frames.reserve(size_);
            const size_t start=(next_+kCapacity-size_)%kCapacity;
            for(size_t i=0;i<size_;++i) frames.push_back(history_[(start+i)%kCapacity]);
            total=total_; slow=slow70_; over250=stalls250_; over500=stalls500_;
            over2000=stalls2000_; pauses=guestPauses_; profile=profileFrames_;
            maxArm=maxArmMs_; maxGap=maxGapMs_; maxTimer=maxTimerMs_;
            hist=armHistogram_;
        }
        std::vector<double> arm,gap,late,ui;
        arm.reserve(frames.size()); gap.reserve(frames.size());
        late.reserve(frames.size()); ui.reserve(frames.size());
        for(const auto &f:frames) {
            if(!f.profiled) arm.push_back(f.armMs);
            gap.push_back(f.gapMs); late.push_back(f.timerLateMs);
            ui.push_back(f.uiQueueDelayMs);
        }
        std::string out;
        out.reserve(1280+frames.size()*192);
        char line[512];
        std::snprintf(line,sizeof(line),
            "DEEP_SUMMARY frames_total=%llu frames_retained=%zu capacity=%zu slow_gap_70=%llu arm_over250=%llu arm_over500=%llu arm_over2000=%llu next_pause_500=%llu profile_frames=%llu peak_arm_ms=%.2f peak_gap_ms=%.2f peak_timer_late_ms=%.2f\n",
            (unsigned long long)total,frames.size(),kCapacity,
            (unsigned long long)slow,(unsigned long long)over250,(unsigned long long)over500,
            (unsigned long long)over2000,(unsigned long long)pauses,(unsigned long long)profile,
            maxArm,maxGap,maxTimer);
        out+=line;
        std::snprintf(line,sizeof(line),
            "DEEP_PERCENTILES scope=retained arm_unprofiled_p50=%.2f arm_unprofiled_p95=%.2f arm_unprofiled_p99=%.2f gap_p50=%.2f gap_p95=%.2f gap_p99=%.2f timer_p95=%.2f ui_queue_p95=%.2f\n",
            percentile(arm,.50),percentile(arm,.95),percentile(arm,.99),
            percentile(gap,.50),percentile(gap,.95),percentile(gap,.99),
            percentile(late,.95),percentile(ui,.95));
        out+=line;
        static const char *names[]={"<=32","32-62.5","62.5-70","70-100","100-250","250-500","500-1000","1000-2000",">2000"};
        for(size_t i=0;i<hist.size();++i) {
            std::snprintf(line,sizeof(line),"DEEP_HISTOGRAM arm_ms=%s frames=%llu\n",
                          names[i],(unsigned long long)hist[i]);
            out+=line;
        }
        out+="DEEP_FIELDS uptime_ms frame arm_ms prep_ms gap_ms timer_late_ms next_request_ms previous_request_ms ui_queue_last_ms drops_total last_guest_entry_pc profile\n";
        for(const auto &f:frames) {
            std::snprintf(line,sizeof(line),
                "DEEP_FRAME uptime_ms=%llu frame=%u arm_ms=%.2f prep_ms=%.3f gap_ms=%.2f timer_late_ms=%.2f next_request_ms=%u previous_request_ms=%u ui_queue_last_ms=%.2f drops_total=%llu checkpoint_pc=0x%08x profiled=%u\n",
                (unsigned long long)f.uptimeMs,f.frame,
                f.armMs,f.preparationMs,f.gapMs,f.timerLateMs,
                f.requestedNextMs,f.previousRequestMs,f.uiQueueDelayMs,
                (unsigned long long)f.presentationDrops,f.lastGuestEntryPc,f.profiled?1u:0u);
            out+=line;
        }
        return out;
    }
};
} // namespace destinia
