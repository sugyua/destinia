#import <GameController/GameController.h>
#include <map>
#include <cmath>

// Virtual touch mapping for Destinia (native 400x240 screen coordinates):
// D-Pad / Left Thumbstick:
// Center: (52, 185) with radius ~28
// Action Buttons (GameSir X5 Lite & MFi / ExtendedGamepad):
// - Ataque: Botão A (físico e MFi buttonA) -> (360, 200)
// - Voltar / Cancelar / Ação secundária: Botão B (buttonB) -> (384, 16)
// - Ataque alternativo / Interagir: Botão X (buttonX) -> (360, 200)
// - Menu / Bag: Botão Y (buttonY) -> (384, 16)
// - Pocao HP (Vida): L1 (leftShoulder) -> Botão HP na HUD de jogo (316, 172)
// - Pocao MP (Mana): R1 (rightShoulder) -> Botão MP na HUD de jogo (356, 140)
// - Menu Principal: Start / Menu (buttonMenu) -> (384, 16)
// - Salvar Jogo / Save: Select / Options / Back -> (270, 16)
// - Habilidades: Analógico Direito (Right Thumbstick):
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

        // 1. Analógico Esquerdo (Movimento 360 graus com deadzone de 0.20):
        gamepad.leftThumbstick.valueChangedHandler = ^(GCControllerDirectionPad *dpad, float xValue, float yValue) {
            float dist = std::sqrt(xValue * xValue + yValue * yValue);
            constexpr float deadzone = 0.20f;
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

        // 3. Analógico Direito -> HABILIDADES (Slots 0, 1, 2, 3):
        // RS <- (Esquerda): Slot 0 (316, 172)
        // RS v  (Baixo):    Slot 1 (316, 216)
        // RS -> (Direita):  Slot 2 (356, 140)
        // RS ^  (Cima):     Slot 3 (388, 140)
        gamepad.rightThumbstick.valueChangedHandler = ^(GCControllerDirectionPad *dpad, float xValue, float yValue) {
            float dist = std::sqrt(xValue * xValue + yValue * yValue);
            constexpr float deadzone = 0.25f; // Sensibilidade responsiva
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

        // 4. Ataque Principal: Botão A (Solicitado pelo usuário: ataque no botão A!)
        gamepad.buttonA.pressedChangedHandler = ^(GCControllerButtonInput *btn, float val, BOOL pressed) {
            triggerButton(201, CGPointMake(360, 200), pressed);
        };

        // 5. Voltar / Cancelar / Ação secundária: Botão B
        gamepad.buttonB.pressedChangedHandler = ^(GCControllerButtonInput *btn, float val, BOOL pressed) {
            triggerButton(202, CGPointMake(384, 16), pressed);
        };

        // 6. Botão X: Também configurado para Ataque / Interação (para compatibilidade com GameSir X5 Lite)
        gamepad.buttonX.pressedChangedHandler = ^(GCControllerButtonInput *btn, float val, BOOL pressed) {
            triggerButton(203, CGPointMake(360, 200), pressed);
        };

        // 7. Botão Y: Menu / Inventário
        gamepad.buttonY.pressedChangedHandler = ^(GCControllerButtonInput *btn, float val, BOOL pressed) {
            triggerButton(204, CGPointMake(384, 16), pressed);
        };

        // 8. Gatilhos e Ombros (L1, R1, L2, R2):
        // L1 (leftShoulder): Acesso rápido a Poção de HP (Slot 0: 316, 172)
        gamepad.leftShoulder.pressedChangedHandler = ^(GCControllerButtonInput *btn, float val, BOOL pressed) {
            triggerButton(206, CGPointMake(316, 172), pressed);
        };

        // R1 (rightShoulder): Acesso rápido a Poção de MP (Slot 2: 356, 140)
        gamepad.rightShoulder.pressedChangedHandler = ^(GCControllerButtonInput *btn, float val, BOOL pressed) {
            triggerButton(207, CGPointMake(356, 140), pressed);
        };

        // L2 (leftTrigger): Habilidade Slot 1 (316, 216)
        gamepad.leftTrigger.pressedChangedHandler = ^(GCControllerButtonInput *btn, float val, BOOL pressed) {
            triggerButton(208, CGPointMake(316, 216), pressed);
        };

        // R2 (rightTrigger): Habilidade Slot 3 (388, 140)
        gamepad.rightTrigger.pressedChangedHandler = ^(GCControllerButtonInput *btn, float val, BOOL pressed) {
            triggerButton(209, CGPointMake(388, 140), pressed);
        };

        // 9. Menu Principal: Start (buttonMenu)
        gamepad.buttonMenu.pressedChangedHandler = ^(GCControllerButtonInput *btn, float val, BOOL pressed) {
            triggerButton(301, CGPointMake(384, 16), pressed);
        };

        // 10. Salvar Jogo: Select / Options / Back (botão SAVE em 270, 16)
        if (@available(iOS 13.0, *)) {
            if (gamepad.buttonOptions) {
                gamepad.buttonOptions.pressedChangedHandler = ^(GCControllerButtonInput *btn, float val, BOOL pressed) {
                    triggerButton(302, CGPointMake(270, 16), pressed);
                };
            }
        }
    }
};
