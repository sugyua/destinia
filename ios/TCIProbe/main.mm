#import <UIKit/UIKit.h>
#import <CoreGraphics/CoreGraphics.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <vector>

extern "C" int destinia_game_open(
    const char *path, char *out, size_t capacity,
    uint8_t *frameOut, size_t frameCapacity);
extern "C" int destinia_game_step(
    uint8_t *frameOut, size_t frameCapacity, uint32_t *nextMs,
    char *message, size_t capacity);
extern "C" int destinia_game_touch(
    uint32_t x, uint32_t y, uint32_t type,
    char *message, size_t capacity);
extern "C" void destinia_game_close(void);

static constexpr size_t kWidth=400;
static constexpr size_t kHeight=240;
static constexpr size_t kFrameBytes=kWidth*kHeight*2;

static UIImage *DestiniaImageFromRGB565(const uint8_t *pixels) {
    if (!pixels) return nil;
    std::vector<uint8_t> rgba(kWidth*kHeight*4);
    for (size_t i=0;i<kWidth*kHeight;++i) {
        const uint16_t p=uint16_t(pixels[i*2]) |
            (uint16_t(pixels[i*2+1])<<8);
        rgba[4*i+0]=static_cast<uint8_t>(((p>>11)&31)*255/31);
        rgba[4*i+1]=static_cast<uint8_t>(((p>>5)&63)*255/63);
        rgba[4*i+2]=static_cast<uint8_t>((p&31)*255/31);
        rgba[4*i+3]=255;
    }
    NSData *bytes=[NSData dataWithBytes:rgba.data() length:rgba.size()];
    CGDataProviderRef provider=CGDataProviderCreateWithCFData(
        (__bridge CFDataRef)bytes);
    if (!provider) return nil;
    CGColorSpaceRef colors=CGColorSpaceCreateDeviceRGB();
    if (!colors) {
        CGDataProviderRelease(provider);
        return nil;
    }
    CGImageRef bitmap=CGImageCreate(
        kWidth,kHeight,8,32,kWidth*4,
        colors,kCGBitmapByteOrder32Big|kCGImageAlphaPremultipliedLast,
        provider,nullptr,false,kCGRenderingIntentDefault);
    CGColorSpaceRelease(colors);
    CGDataProviderRelease(provider);
    if (!bitmap) return nil;
    UIImage *image=[UIImage imageWithCGImage:bitmap];
    CGImageRelease(bitmap);
    return image;
}

@interface DestiniaViewController : UIViewController
@property (nonatomic,strong) UIImageView *gameView;
@property (nonatomic,strong) UILabel *statusLabel;
@property (nonatomic,copy) void (^touchCallback)(CGPoint point, int type);
@property (nonatomic,copy) void (^metricsCallback)(void);
@property (nonatomic,assign) BOOL trackingMetricsGesture;
@end

@implementation DestiniaViewController

- (BOOL)prefersStatusBarHidden { return YES; }
- (BOOL)prefersHomeIndicatorAutoHidden { return YES; }

- (void)loadView {
    self.view=[[UIView alloc] initWithFrame:[UIScreen mainScreen].bounds];
    self.view.backgroundColor=[UIColor blackColor];

    self.gameView=[[UIImageView alloc] initWithFrame:self.view.bounds];
    self.gameView.autoresizingMask=
        UIViewAutoresizingFlexibleWidth|UIViewAutoresizingFlexibleHeight;
    self.gameView.contentMode=UIViewContentModeScaleAspectFit;
    self.gameView.userInteractionEnabled=NO;
    [self.view addSubview:self.gameView];

    self.statusLabel=[[UILabel alloc] initWithFrame:CGRectZero];
    self.statusLabel.font=[UIFont monospacedSystemFontOfSize:12
                                                  weight:UIFontWeightMedium];
    self.statusLabel.textColor=[UIColor whiteColor];
    self.statusLabel.backgroundColor=
        [[UIColor blackColor] colorWithAlphaComponent:0.65];
    self.statusLabel.textAlignment=NSTextAlignmentCenter;
    self.statusLabel.numberOfLines=2;
    self.statusLabel.text=@"Inicializando Destinia ARM32...";
    self.statusLabel.userInteractionEnabled=NO;
    self.statusLabel.layer.cornerRadius=7;
    self.statusLabel.layer.masksToBounds=YES;
    [self.view addSubview:self.statusLabel];
}

- (void)viewDidLayoutSubviews {
    [super viewDidLayoutSubviews];
    CGRect safe=self.view.safeAreaLayoutGuide.layoutFrame;
    self.statusLabel.frame=CGRectMake(
        safe.origin.x+10,safe.origin.y+6,
        MAX(100,safe.size.width-20),38);
}

- (void)forwardTouches:(NSSet<UITouch *> *)touches type:(int)type {
    UITouch *touch=[touches anyObject];
    if (!touch || !self.touchCallback) return;
    const CGPoint p=[touch locationInView:self.view];
    const CGSize size=self.view.bounds.size;
    const CGFloat scale=MIN(size.width/kWidth,size.height/kHeight);
    if (scale<=0) return;
    const CGFloat left=(size.width-kWidth*scale)/2;
    const CGFloat top=(size.height-kHeight*scale)/2;
    const CGFloat x=(p.x-left)/scale;
    const CGFloat y=(p.y-top)/scale;
    if (type==0 &&
        (x<0 || y<0 || x>=kWidth || y>=kHeight)) return;
    const CGPoint converted=CGPointMake(
        MAX(0,MIN(kWidth-1,x)),
        MAX(0,MIN(kHeight-1,y)));
    self.touchCallback(converted,type);
}
- (void)touchesBegan:(NSSet<UITouch *> *)touches
          withEvent:(UIEvent *)event {
    if ([[event allTouches] count]>=3) {
        self.trackingMetricsGesture=YES;
        if (self.metricsCallback) self.metricsCallback();
        return;
    }
    if (!self.trackingMetricsGesture)
        [self forwardTouches:touches type:0];
}
- (void)touchesMoved:(NSSet<UITouch *> *)touches
          withEvent:(UIEvent *)event {
    if (!self.trackingMetricsGesture)
        [self forwardTouches:touches type:2];
}
- (void)touchesEnded:(NSSet<UITouch *> *)touches
          withEvent:(UIEvent *)event {
    if (self.trackingMetricsGesture) {
        if ([[event allTouches] count] <= [touches count])
            self.trackingMetricsGesture=NO;
        return;
    }
    [self forwardTouches:touches type:1];
}
- (void)touchesCancelled:(NSSet<UITouch *> *)touches
              withEvent:(UIEvent *)event {
    if (self.trackingMetricsGesture) {
        self.trackingMetricsGesture=NO;
        return;
    }
    [self forwardTouches:touches type:1];
}
@end

@interface AppDelegate : UIResponder <UIApplicationDelegate>
@property (strong,nonatomic) UIWindow *window;
@end

@implementation AppDelegate {
    dispatch_queue_t _gameQueue;
    dispatch_source_t _timer;
    DestiniaViewController *_controller;
    std::atomic<bool> _paused;
    std::atomic<bool> _showMetrics;
    bool _ready;
    uint32_t _frameIndex;
    std::chrono::steady_clock::time_point _measureStarted;
    uint32_t _measureFrames;
    double _totalArmMs;
    double _totalPreparationMs;
}

- (void)showStatus:(NSString *)status {
    dispatch_async(dispatch_get_main_queue(), ^{
        self->_controller.statusLabel.hidden=NO;
        self->_controller.statusLabel.text=status;
    });
}

- (void)showPixels:(const std::vector<uint8_t> &)pixels
          frameNum:(uint32_t)frameNumber {
    UIImage *image=DestiniaImageFromRGB565(pixels.data());
    if (!image) {
        [self showStatus:@"Erro ao converter RGB565"];
        return;
    }
    dispatch_async(dispatch_get_main_queue(), ^{
        self->_controller.gameView.image=image;
        self->_controller.statusLabel.hidden=!self->_showMetrics.load();
    });
}

- (void)armTimer:(uint32_t)delay {
    if (!_timer) {
        _timer=dispatch_source_create(
            DISPATCH_SOURCE_TYPE_TIMER,0,0,_gameQueue);
        __weak AppDelegate *weakSelf=self;
        dispatch_source_set_event_handler(_timer,^{
            AppDelegate *owner=weakSelf;
            if (owner) [owner gameTick];
        });
        dispatch_resume(_timer);
    }
    const uint64_t ms=std::max(1u,std::min(delay,1000u));
    dispatch_source_set_timer(
        _timer,dispatch_time(DISPATCH_TIME_NOW,
                              static_cast<int64_t>(ms)*NSEC_PER_MSEC),
        DISPATCH_TIME_FOREVER,5*NSEC_PER_MSEC);
}

- (void)gameTick {
    if (!_ready) return;
    if (_paused.load()) {
        [self armTimer:250];
        return;
    }

    const auto tickStart=std::chrono::steady_clock::now();
    std::vector<uint8_t> pixels(kFrameBytes);
    uint32_t nextMs=0;
    char error[512]={};
    if (!destinia_game_step(
            pixels.data(),pixels.size(),&nextMs,error,sizeof(error))) {
        _ready=false;
        NSString *message=[NSString stringWithFormat:
            @"Erro no quadro ARM32:\n%s",error];
        [self showStatus:message];
        return;
    }
    const auto emulationFinished=std::chrono::steady_clock::now();
    ++_frameIndex;
    [self showPixels:pixels frameNum:_frameIndex];
    const auto prepared=std::chrono::steady_clock::now();
    const double armMs=std::chrono::duration<double,std::milli>(
        emulationFinished-tickStart).count();
    const double preparationMs=std::chrono::duration<double,std::milli>(
        prepared-emulationFinished).count();
    ++_measureFrames;
    _totalArmMs+=armMs;
    _totalPreparationMs+=preparationMs;
    const double sampleMs=std::chrono::duration<double,std::milli>(
        prepared-_measureStarted).count();
    if (sampleMs>=2000.0 && _measureFrames>0) {
        const double fps=1000.0*_measureFrames/sampleMs;
        const double avgArm=_totalArmMs/_measureFrames;
        const double avgPrep=_totalPreparationMs/_measureFrames;
        NSLog(@"Destinia perf FPS=%.1f ARM32=%.1fms RGB565=%.1fms frames=%u",
              fps,avgArm,avgPrep,_measureFrames);
        if (_showMetrics.load()) {
            NSString *summary=[NSString stringWithFormat:
                @"FPS %.1f | ARM32 %.1fms | RGB %.1fms",
                fps,avgArm,avgPrep];
            dispatch_async(dispatch_get_main_queue(), ^{
                self->_controller.statusLabel.hidden=NO;
                self->_controller.statusLabel.text=summary;
            });
        }
        _measureStarted=prepared;
        _measureFrames=0;
        _totalArmMs=0;
        _totalPreparationMs=0;
    }
    if (nextMs==0) {
        _ready=false;
        [self showStatus:@"Destinia encerrou a execucao."];
        return;
    }
    const auto tickElapsed=std::chrono::duration_cast<
        std::chrono::milliseconds>(
            std::chrono::steady_clock::now()-tickStart).count();
    const uint32_t remaining=tickElapsed>=nextMs
        ? 1u : nextMs-static_cast<uint32_t>(tickElapsed);
    [self armTimer:remaining];
}

- (BOOL)application:(UIApplication *)application
    didFinishLaunchingWithOptions:(NSDictionary *)options {
    _paused.store(false);
    _showMetrics.store(false);
    _ready=false;
    _frameIndex=0;
    _measureStarted=std::chrono::steady_clock::now();
    _measureFrames=0;
    _totalArmMs=0;
    _totalPreparationMs=0;
    _gameQueue=dispatch_queue_create(
        "com.destinia.arm32.interpreter",DISPATCH_QUEUE_SERIAL);
    _controller=[[DestiniaViewController alloc] init];
    self.window=[[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
    self.window.rootViewController=_controller;
    [self.window makeKeyAndVisible];

    __weak AppDelegate *weakSelf=self;
    _controller.metricsCallback=^{
        AppDelegate *owner=weakSelf;
        if (!owner) return;
        const bool enable=!owner->_showMetrics.load();
        owner->_showMetrics.store(enable);
        owner->_controller.statusLabel.hidden=!enable;
        if (enable)
            owner->_controller.statusLabel.text=
                @"Medindo FPS e custo ARM32...";
    };
    _controller.touchCallback=^(CGPoint point,int type) {
        AppDelegate *owner=weakSelf;
        if (!owner) return;
        dispatch_async(owner->_gameQueue,^{
            if (!owner->_ready) return;
            char error[512]={};
            if (!destinia_game_touch(
                    static_cast<uint32_t>(point.x),
                    static_cast<uint32_t>(point.y),
                    static_cast<uint32_t>(type),
                    error,sizeof(error))) {
                NSString *message=[NSString stringWithFormat:
                    @"Falha no toque ARM32: %s",error];
                [owner showStatus:message];
            }
        });
    };

    dispatch_async(_gameQueue,^{
        @autoreleasepool {
            NSString *path=[[NSBundle mainBundle]
                pathForResource:@"libdestinia_jni" ofType:@"so"];
            if (!path) {
                [self showStatus:@"Biblioteca Destinia ausente."];
                return;
            }
            std::vector<uint8_t> pixels(kFrameBytes);
            char output[1024]={};
            if (!destinia_game_open(
                    [path fileSystemRepresentation],
                    output,sizeof(output),
                    pixels.data(),pixels.size())) {
                NSString *error=[NSString stringWithFormat:@"%s",output];
                [self showStatus:error];
                return;
            }
            NSLog(@"Destinia iOS: %s",output);
            _ready=true;
            _measureStarted=std::chrono::steady_clock::now();
            _measureFrames=0;
            _totalArmMs=0;
            _totalPreparationMs=0;
            _frameIndex=4;
            [self showPixels:pixels frameNum:_frameIndex];
            [self armTimer:62];
        }
    });
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application {
    _paused.store(true);
}
- (void)applicationDidBecomeActive:(UIApplication *)application {
    _paused.store(false);
}
- (void)applicationWillTerminate:(UIApplication *)application {
    _paused.store(true);
    dispatch_async(_gameQueue,^{ destinia_game_close(); });
}
@end

int main(int argc,char *argv[]) {
    @autoreleasepool {
        return UIApplicationMain(
            argc,argv,nil,NSStringFromClass([AppDelegate class]));
    }
}
