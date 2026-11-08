#import <GameController/GameController.h>
#include <map>

// Virtual touch mapping for Destinia (native 400x240 screen coordinates):
// D-Pad / Analog Stick:
// Center of Dpad: (52, 185)
// Up: (52, 160)
// Down: (52, 210)
// Left: (27, 185)
// Right: (77, 185)
// Up-Left: (34, 167)
// Up-Right: (70, 167)
// Down-Left: (34, 203)
// Down-Right: (70, 203)

// Action Buttons:
// Attack / Main Action (A): (360, 200)
// Skill 1 (X): (316, 216)
// Skill 2 (Y): (316, 172)
// Skill 3 (B): (356, 140)
// Special / Ultimate (RB): (388, 140)
// Potion HP (LB): (384, 52)
// Potion MP / Menu / Bag (Start/Menu): (384, 16)
// Pause / Character Status (Select/Options): (16, 16)

class DestiniaGameControllerBridge {
public:
    typedef void (^SendTouchFn)(CGPoint pt, int type);

    static DestiniaGameControllerBridge& instance() {
        static DestiniaGameControllerBridge inst;
        return inst;
    }

    void setup(SendTouchFn sendTouch) {
        sendTouch_ = [sendTouch copy];

        [[NSNotificationCenter defaultCenter]
            addObserverForName:GCControllerDidConnectNotification
            object:nil
            queue:[NSOperationQueue mainQueue]
            usingBlock:^(NSNotification *note) {
                GCController *controller = note.object;
                NSLog(@"[Destinia] Controle conectado: %@ (fornecedor: %@)",
                      controller.vendorName, controller.productCategory);
                bindController(controller);
            }];

        [[NSNotificationCenter defaultCenter]
            addObserverForName:GCControllerDidDisconnectNotification
            object:nil
            queue:[NSOperationQueue mainQueue]
            usingBlock:^(NSNotification *note) {
                GCController *controller = note.object;
                NSLog(@"[Destinia] Controle desconectado: %@", controller.vendorName);
            }];

        for (GCController *c in [GCController controllers]) {
            bindController(c);
        }
    }

private:
    SendTouchFn sendTouch_ = nil;
    CGPoint currentStickPoint_ = CGPointZero;
    bool stickActive_ = false;

    // Track active digital buttons to release touches correctly
    std::map<int, CGPoint> activeButtons_;

    void triggerButton(int btnId, CGPoint coord, bool pressed) {
        if (!sendTouch_) return;
        if (pressed) {
            activeButtons_[btnId] = coord;
            sendTouch_(coord, 0); // Touch Down
        } else {
            auto it = activeButtons_.find(btnId);
            if (it != activeButtons_.end()) {
                sendTouch_(it->second, 1); // Touch Up
                activeButtons_.erase(it);
            }
        }
    }

    void bindController(GCController *controller) {
        GCExtendedGamepad *gamepad = controller.extendedGamepad;
        if (!gamepad) return;

        __weak GCController *weakCtrl = controller;

        // Left Thumbstick / Analog Movement:
        gamepad.leftThumbstick.valueChangedHandler = ^(GCControllerDirectionPad *dpad, float xValue, float yValue) {
            float dist = std::sqrt(xValue * xValue + yValue * yValue);
            constexpr float deadzone = 0.25f;
            if (dist < deadzone) {
                if (stickActive_) {
                    sendTouch_(currentStickPoint_, 1); // Release touch
                    stickActive_ = false;
                }
                return;
            }

            // Map angle to offset around D-pad center (52, 185) with radius 28
            float normX = xValue / dist;
            float normY = yValue / dist; // in iOS GC, +y is UP
            float targetX = 52.0f + normX * 28.0f;
            float targetY = 185.0f - normY * 28.0f; // screen +y is DOWN

            CGPoint newPt = CGPointMake(targetX, targetY);
            if (!stickActive_) {
                sendTouch_(newPt, 0); // Touch Down
                stickActive_ = true;
            } else {
                sendTouch_(newPt, 2); // Touch Move
            }
            currentStickPoint_ = newPt;
        };

        // D-Pad Digitals:
        gamepad.dpad.up.pressedChangedHandler = ^(GCControllerButtonInput *btn, float val, BOOL pressed) {
            if (stickActive_) return;
            triggerButton(101, CGPointMake(52, 158), pressed);
        };
        gamepad.dpad.down.pressedChangedHandler = ^(GCControllerButtonInput *btn, float val, BOOL pressed) {
            if (stickActive_) return;
            triggerButton(102, CGPointMake(52, 212), pressed);
        };
        gamepad.dpad.left.pressedChangedHandler = ^(GCControllerButtonInput *btn, float val, BOOL pressed) {
            if (stickActive_) return;
            triggerButton(103, CGPointMake(25, 185), pressed);
        };
        gamepad.dpad.right.pressedChangedHandler = ^(GCControllerButtonInput *btn, float val, BOOL pressed) {
            if (stickActive_) return;
            triggerButton(104, CGPointMake(79, 185), pressed);
        };

        // Action Buttons:
        // Attack: Button A (or Cross on PS / B on Nintendo) -> Main Attack
        gamepad.buttonA.pressedChangedHandler = ^(GCControllerButtonInput *btn, float val, BOOL pressed) {
            triggerButton(201, CGPointMake(360, 200), pressed);
        };

        // Skills:
        // Button X -> Skill 1
        gamepad.buttonX.pressedChangedHandler = ^(GCControllerButtonInput *btn, float val, BOOL pressed) {
            triggerButton(202, CGPointMake(316, 216), pressed);
        };

        // Button Y -> Skill 2
        gamepad.buttonY.pressedChangedHandler = ^(GCControllerButtonInput *btn, float val, BOOL pressed) {
            triggerButton(203, CGPointMake(316, 172), pressed);
        };

        // Button B -> Skill 3
        gamepad.buttonB.pressedChangedHandler = ^(GCControllerButtonInput *btn, float val, BOOL pressed) {
            triggerButton(204, CGPointMake(356, 140), pressed);
        };

        // Right Shoulder (R1 / RB) -> Ultimate / 4th Skill or Dash:
        gamepad.rightShoulder.pressedChangedHandler = ^(GCControllerButtonInput *btn, float val, BOOL pressed) {
            triggerButton(205, CGPointMake(388, 140), pressed);
        };

        // Left Shoulder (L1 / LB) -> Quick HP Potion:
        gamepad.leftShoulder.pressedChangedHandler = ^(GCControllerButtonInput *btn, float val, BOOL pressed) {
            triggerButton(206, CGPointMake(384, 52), pressed);
        };

        // Left Trigger (L2 / LT) -> Quick MP Potion:
        gamepad.leftTrigger.pressedChangedHandler = ^(GCControllerButtonInput *btn, float val, BOOL pressed) {
            triggerButton(207, CGPointMake(350, 52), pressed);
        };

        // Menu / Start:
        gamepad.buttonMenu.pressedChangedHandler = ^(GCControllerButtonInput *btn, float val, BOOL pressed) {
            triggerButton(301, CGPointMake(384, 16), pressed);
        };

        // Options / Select (buttonOptions if available):
        if (@available(iOS 13.0, *)) {
            if (gamepad.buttonOptions) {
                gamepad.buttonOptions.pressedChangedHandler = ^(GCControllerButtonInput *btn, float val, BOOL pressed) {
                    triggerButton(302, CGPointMake(16, 16), pressed);
                };
            }
        }
    }
};
