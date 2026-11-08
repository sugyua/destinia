#import <GameController/GameController.h>
#include <map>
#include <cmath>

// Virtual touch mapping for Destinia (native 400x240 screen coordinates):
// D-Pad / Left Thumbstick:
// Center: (52, 185) with radius ~28
// Action Buttons (100% matched to PC DualSense layout):
// - Ataque: X (Cruz / buttonA no ExtendedGamepad): (360, 200)
// - Voltar / Cancelar: Circulo (buttonB no ExtendedGamepad): (384, 16) ou botão voltar/escape
// - Pocao HP: L1 (leftShoulder): (384, 52)
// - Pocao Mana (MP): R1 (rightShoulder): (350, 52)
// - Menu Principal: Start (buttonMenu): (384, 16)
// - Salvar Jogo / Opcoes: Select / Options: (16, 16)
// - Habilidades: Analogico Direito (Right Thumbstick):
//     RS <- (Esquerda): (316, 172) [Slot 0]
//     RS v  (Baixo):    (316, 216) [Slot 1]
//     RS -> (Direita):  (356, 140) [Slot 2]
//     RS ^  (Cima):     (388, 140) [Slot 3]

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
    CGPoint currentLeftStickPoint_ = CGPointZero;
    bool leftStickActive_ = false;

    int currentRightSkillSlot_ = -1;
    CGPoint currentRightSkillPoint_ = CGPointZero;

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

        // 1. Analógico Esquerdo (Movimento 360 graus):
        gamepad.leftThumbstick.valueChangedHandler = ^(GCControllerDirectionPad *dpad, float xValue, float yValue) {
            float dist = std::sqrt(xValue * xValue + yValue * yValue);
            constexpr float deadzone = 0.25f;
            if (dist < deadzone) {
                if (leftStickActive_) {
                    sendTouch_(currentLeftStickPoint_, 1); // Release touch
                    leftStickActive_ = false;
                }
                return;
            }

            float normX = xValue / dist;
            float normY = yValue / dist; // in iOS GC, +y is UP
            float targetX = 52.0f + normX * 28.0f;
            float targetY = 185.0f - normY * 28.0f; // screen +y is DOWN

            CGPoint newPt = CGPointMake(targetX, targetY);
            if (!leftStickActive_) {
                sendTouch_(newPt, 0); // Touch Down
                leftStickActive_ = true;
            } else {
                sendTouch_(newPt, 2); // Touch Move
            }
            currentLeftStickPoint_ = newPt;
        };

        // 2. D-Pad Digital (Movimento 4 direções):
        gamepad.dpad.up.pressedChangedHandler = ^(GCControllerButtonInput *btn, float val, BOOL pressed) {
            if (leftStickActive_) return;
            triggerButton(101, CGPointMake(52, 158), pressed);
        };
        gamepad.dpad.down.pressedChangedHandler = ^(GCControllerButtonInput *btn, float val, BOOL pressed) {
            if (leftStickActive_) return;
            triggerButton(102, CGPointMake(52, 212), pressed);
        };
        gamepad.dpad.left.pressedChangedHandler = ^(GCControllerButtonInput *btn, float val, BOOL pressed) {
            if (leftStickActive_) return;
            triggerButton(103, CGPointMake(25, 185), pressed);
        };
        gamepad.dpad.right.pressedChangedHandler = ^(GCControllerButtonInput *btn, float val, BOOL pressed) {
            if (leftStickActive_) return;
            triggerButton(104, CGPointMake(79, 185), pressed);
        };

        // 3. Analógico Direito -> HABILIDADES (Exatamente igual ao PC!):
        // RS <- (Esquerda): Slot 0 (316, 172)
        // RS v  (Baixo):    Slot 1 (316, 216)
        // RS -> (Direita):  Slot 2 (356, 140)
        // RS ^  (Cima):     Slot 3 (388, 140)
        gamepad.rightThumbstick.valueChangedHandler = ^(GCControllerDirectionPad *dpad, float xValue, float yValue) {
            float dist = std::sqrt(xValue * xValue + yValue * yValue);
            constexpr float deadzone = 0.40f;
            if (dist < deadzone) {
                if (currentRightSkillSlot_ != -1) {
                    sendTouch_(currentRightSkillPoint_, 1); // Solta toque da habilidade
                    currentRightSkillSlot_ = -1;
                }
                return;
            }

            int targetSlot = -1;
            CGPoint targetPt = CGPointZero;
            if (std::abs(xValue) > std::abs(yValue)) {
                if (xValue < 0) {
                    targetSlot = 0; // RS < (Esquerda)
                    targetPt = CGPointMake(316, 172);
                } else {
                    targetSlot = 2; // RS > (Direita)
                    targetPt = CGPointMake(356, 140);
                }
            } else {
                if (yValue > 0) {
                    targetSlot = 3; // RS ^ (Cima)
                    targetPt = CGPointMake(388, 140);
                } else {
                    targetSlot = 1; // RS v (Baixo)
                    targetPt = CGPointMake(316, 216);
                }
            }

            if (targetSlot != currentRightSkillSlot_) {
                if (currentRightSkillSlot_ != -1) {
                    sendTouch_(currentRightSkillPoint_, 1);
                }
                currentRightSkillSlot_ = targetSlot;
                currentRightSkillPoint_ = targetPt;
                sendTouch_(targetPt, 0); // Pressiona o slot de skill correspondente
            }
        };

        // 4. Ataque: X (Cruz / buttonA):
        gamepad.buttonA.pressedChangedHandler = ^(GCControllerButtonInput *btn, float val, BOOL pressed) {
            triggerButton(201, CGPointMake(360, 200), pressed);
        };

        // 5. Voltar / Cancelar: Círculo (buttonB):
        // Aciona o botão de voltar/fechar menus ou cancelamento
        gamepad.buttonB.pressedChangedHandler = ^(GCControllerButtonInput *btn, float val, BOOL pressed) {
            triggerButton(202, CGPointMake(384, 16), pressed);
        };

        // 6. Poções:
        // L1 (leftShoulder): Poção de Vida (HP)
        gamepad.leftShoulder.pressedChangedHandler = ^(GCControllerButtonInput *btn, float val, BOOL pressed) {
            triggerButton(206, CGPointMake(384, 52), pressed);
        };

        // R1 (rightShoulder): Poção de Mana (MP)
        gamepad.rightShoulder.pressedChangedHandler = ^(GCControllerButtonInput *btn, float val, BOOL pressed) {
            triggerButton(207, CGPointMake(350, 52), pressed);
        };

        // 7. Menu: Start (buttonMenu):
        gamepad.buttonMenu.pressedChangedHandler = ^(GCControllerButtonInput *btn, float val, BOOL pressed) {
            triggerButton(301, CGPointMake(384, 16), pressed);
        };

        // 8. Salvar Jogo / Menu de Save: Select / Options / Back:
        // Na tela principal do jogo, o botão "SAVE" fica no topo em (270, 16)
        if (@available(iOS 13.0, *)) {
            if (gamepad.buttonOptions) {
                gamepad.buttonOptions.pressedChangedHandler = ^(GCControllerButtonInput *btn, float val, BOOL pressed) {
                    triggerButton(302, CGPointMake(270, 16), pressed);
                };
            }
        }
    }
};
