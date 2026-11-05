#import <UIKit/UIKit.h>
#import <unicorn/unicorn.h>
#include <stdint.h>

static NSString *runTCI(void) {
    uc_engine *uc = NULL;
    uint32_t r0 = 0;

    const uint8_t code[] = {
        0x2a, 0x00, 0xa0, 0xe3
    };

    uc_err err = uc_open(UC_ARCH_ARM, UC_MODE_ARM, &uc);

    if (err == UC_ERR_OK)
        err = uc_mem_map(uc, 0x1000, 0x1000, UC_PROT_ALL);

    if (err == UC_ERR_OK)
        err = uc_mem_write(uc, 0x1000, code, sizeof(code));

    if (err == UC_ERR_OK)
        err = uc_emu_start(uc, 0x1000, 0x1004, 0, 0);

    if (err == UC_ERR_OK)
        err = uc_reg_read(uc, UC_ARM_REG_R0, &r0);

    NSString *result;

    if (err != UC_ERR_OK) {
        result = [NSString stringWithFormat:
            @"TCI FALHOU\n%s", uc_strerror(err)];
    } else if (r0 == 42) {
        result = @"TCI OK\nARM32 retornou 42";
    } else {
        result = [NSString stringWithFormat:
            @"RESULTADO INCORRETO\nARM32 retornou %u", r0];
    }

    if (uc)
        uc_close(uc);

    return result;
}

@interface AppDelegate : UIResponder <UIApplicationDelegate>
@property (strong, nonatomic) UIWindow *window;
@end

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application
    didFinishLaunchingWithOptions:(NSDictionary *)options {

    self.window = [[UIWindow alloc]
        initWithFrame:[UIScreen mainScreen].bounds];

    UIViewController *controller = [[UIViewController alloc] init];
    controller.view.backgroundColor = [UIColor systemBackgroundColor];

    UILabel *label = [[UILabel alloc] initWithFrame:
        CGRectInset(controller.view.bounds, 24, 80)];

    label.autoresizingMask =
        UIViewAutoresizingFlexibleWidth |
        UIViewAutoresizingFlexibleHeight;

    label.numberOfLines = 0;
    label.textAlignment = NSTextAlignmentCenter;
    label.font = [UIFont boldSystemFontOfSize:24];

    [controller.view addSubview:label];

    self.window.rootViewController = controller;
    [self.window makeKeyAndVisible];

    label.text = runTCI();
    NSLog(@"TCIProbe: %@", label.text);

    return YES;
}

@end

int main(int argc, char *argv[]) {
    @autoreleasepool {
        return UIApplicationMain(
            argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}