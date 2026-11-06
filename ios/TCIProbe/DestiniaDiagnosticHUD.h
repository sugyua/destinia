#pragma once
#import <UIKit/UIKit.h>
#import <mach/mach.h>

// Temporary iOS diagnostic overlay. Remove the import and its call sites
// from main.mm to remove this feature from production builds.
#ifndef DESTINIA_DIAGNOSTIC_HUD
#define DESTINIA_DIAGNOSTIC_HUD 1
#endif

static inline double DestiniaResidentMemoryMiB(void) {
    mach_task_basic_info_data_t info = {};
    mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
    kern_return_t result = task_info(mach_task_self(),
        MACH_TASK_BASIC_INFO, reinterpret_cast<task_info_t>(&info), &count);
    return result == KERN_SUCCESS ? (double)info.resident_size / (1024.0 * 1024.0) : -1.0;
}

static inline NSString *DestiniaThermalName(NSProcessInfoThermalState state) {
    switch (state) {
        case NSProcessInfoThermalStateNominal: return @"NORMAL";
        case NSProcessInfoThermalStateFair: return @"ELEVADO";
        case NSProcessInfoThermalStateSerious: return @"ALTO";
        case NSProcessInfoThermalStateCritical: return @"CRITICO";
        default: return @"DESCONHECIDO";
    }
}

@interface DestiniaDiagnosticHUD : UILabel
@property (nonatomic,assign) NSTimeInterval startTime;
@property (nonatomic,assign) NSTimeInterval firstBatteryTime;
@property (nonatomic,assign) float firstBatteryLevel;
- (void)refreshFPS:(double)fps
             armMs:(double)armMs
         cpuPrepMs:(double)prepMs
       avgFrameMs:(double)frameMs
       maxFrameMs:(double)maxFrameMs
       slowFrames:(uint64_t)slowFrames;
@end

@implementation DestiniaDiagnosticHUD
- (instancetype)initWithFrame:(CGRect)frame {
    if ((self = [super initWithFrame:frame])) {
        self.font = [UIFont monospacedSystemFontOfSize:10 weight:UIFontWeightSemibold];
        self.textColor = [UIColor colorWithRed:0.70 green:1.0 blue:0.78 alpha:1.0];
        self.backgroundColor = [[UIColor blackColor] colorWithAlphaComponent:0.78];
        self.numberOfLines = 0;
        self.textAlignment = NSTextAlignmentLeft;
        self.layer.cornerRadius = 6.0;
        self.layer.masksToBounds = YES;
        self.userInteractionEnabled = NO;
        self.adjustsFontSizeToFitWidth = YES;
        self.minimumScaleFactor = 0.8;
        self.startTime = NSProcessInfo.processInfo.systemUptime;
        self.firstBatteryTime = 0;
        self.firstBatteryLevel = -1;
        self.text = @"DESTINIA | DEBUG\nColetando dados...";
    }
    return self;
}
- (void)refreshFPS:(double)fps
             armMs:(double)armMs
         cpuPrepMs:(double)prepMs
       avgFrameMs:(double)frameMs
       maxFrameMs:(double)maxFrameMs
       slowFrames:(uint64_t)slowFrames {
    NSProcessInfo *process = NSProcessInfo.processInfo;
    const NSTimeInterval now = process.systemUptime;
    UIDevice *device = UIDevice.currentDevice;
    const float battery = device.batteryLevel;
    NSString *batteryText = @"N/D";
    NSString *drainText = @"N/D";
    if (battery >= 0.0f) {
        batteryText = [NSString stringWithFormat:@"%.0f%%", battery * 100.0f];
        if (self.firstBatteryTime == 0) {
            self.firstBatteryTime = now;
            self.firstBatteryLevel = battery;
        }
        const double hours = (now - self.firstBatteryTime) / 3600.0;
        // Battery level updates in coarse 1% steps; short samples are misleading.
        if (hours >= 0.05 && self.firstBatteryLevel >= 0 &&
            device.batteryState == UIDeviceBatteryStateUnplugged) {
            const double drain = (self.firstBatteryLevel - battery) * 100.0 / hours;
            drainText = [NSString stringWithFormat:@"%.1f%%/h", drain];
        }
    }
    NSString *memoryText = @"N/D";
    const double memory = DestiniaResidentMemoryMiB();
    if (memory >= 0) memoryText = [NSString stringWithFormat:@"%.0f MiB", memory];
    const unsigned long session = (unsigned long)MAX(0.0, now-self.startTime);
    self.text = [NSString stringWithFormat:
        @" DESTINIA 0.3.7 | DEBUG HUD\n"
         @" FPS       %5.1f / 16\n"
         @" FRAME     %5.1f ms (pico %.0f)\n"
         @" ARM32     %5.1f ms\n"
         @" PREP CPU  %5.1f ms\n"
         @" LENTOS    %llu (>62.5ms)\n"
         @" RAM       %@\n"
         @" BATERIA   %@  %@\n"
         @" TERMICO   %@\n"
         @" SESSAO    %02lu:%02lu\n"
         @" 3 dedos: HUD | 4 dedos: perfil",
        fps, frameMs, maxFrameMs, armMs, prepMs,
        (unsigned long long)slowFrames, memoryText,
        batteryText, drainText,
        DestiniaThermalName(process.thermalState),
        session/60, session%60];
}
@end
