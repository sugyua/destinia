#define SDL_MAIN_HANDLED
#include "../common/player_controls.h"
#include "../common/skill_labels.h"
#include <cassert>
#include <iostream>
int main() {
 using namespace destinia;
 assert(quickSlotAction(0)==Action::SkillLeft);
 assert(quickSlotAction(1)==Action::SkillDown);
 assert(quickSlotAction(2)==Action::SkillRight);
 assert(quickSlotAction(3)==Action::SkillUp);
 assert(quickSlotAction(4)==Action::None);
 Preferences p;
 std::vector<uint8_t> hint(400*240*2);std::array<SkillHintRect,4> boxes{{{100,100,32,32},{0,0,0,0},{0,0,0,0},{0,0,0,0}}};
 paintSkillHints(hint.data(),400,240,p,InputDevice::Touch,boxes,75);assert(std::all_of(hint.begin(),hint.end(),[](auto b){return b==0;}));
 p.keys[SDLK_q]=Action::SkillLeft;paintSkillHints(hint.data(),400,240,p,InputDevice::Keyboard,boxes,75);assert(std::any_of(hint.begin(),hint.end(),[](auto b){return b!=0;}));
 p.keys[SDLK_q]=Action::SkillLeft;
 p.keys[SDLK_2]=Action::SkillRight;
 p.buttons[SDL_CONTROLLER_BUTTON_LEFTSHOULDER]=Action::SkillUp;
 assert(skillBindingLabel(p,Action::SkillLeft,InputDevice::Keyboard)=="Q");
 assert(skillBindingLabel(p,Action::SkillRight,InputDevice::Keyboard)=="2");
 assert(skillBindingLabel(p,Action::SkillUp,InputDevice::Keyboard)=="SEM TECLA");
 assert(skillBindingLabel(p,Action::SkillDown,InputDevice::Controller)=="RS v");
 assert(skillBindingLabel(p,Action::SkillUp,InputDevice::Controller)=="leftshoulder / RS ^");
 assert(skillBindingLabel(p,Action::SkillLeft,InputDevice::Touch).empty());
 SDL_GameController* pad=nullptr;
 PlayerControls controls(p,pad,[](int,bool){},[]{},[]{});
 assert(controls.lastInputDevice()==InputDevice::Keyboard);
 SDL_Event event{};event.type=SDL_MOUSEBUTTONDOWN;event.button.button=SDL_BUTTON_LEFT;
 controls.event(event);assert(controls.lastInputDevice()==InputDevice::Touch);
 event={};event.type=SDL_KEYDOWN;event.key.keysym.sym=SDLK_q;
 controls.event(event);assert(controls.lastInputDevice()==InputDevice::Keyboard);
 // An unbound key must not invent a matching keyboard skill hint.
 event={};event.type=SDL_MOUSEBUTTONDOWN;event.button.button=SDL_BUTTON_LEFT;controls.event(event);
 event={};event.type=SDL_KEYDOWN;event.key.keysym.sym=SDLK_F1;controls.event(event);
 assert(controls.lastInputDevice()==InputDevice::Touch);
 // Exercise SDL's virtual controller, including the actual right-stick route.
 assert(SDL_Init(SDL_INIT_GAMECONTROLLER)==0);
 const int index=SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_GAMECONTROLLER,6,16,0);
 assert(index>=0);pad=SDL_GameControllerOpen(index);assert(pad);
 event={};event.type=SDL_CONTROLLERBUTTONDOWN;event.cbutton.which=controls.controllerId();event.cbutton.button=SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
 controls.event(event);assert(controls.lastInputDevice()==InputDevice::Controller);
 event={};event.type=SDL_KEYDOWN;event.key.keysym.sym=SDLK_q;controls.event(event);
 assert(controls.lastInputDevice()==InputDevice::Keyboard);
 controls.pollSticks();assert(controls.lastInputDevice()==InputDevice::Keyboard);
 assert(SDL_JoystickSetVirtualAxis(SDL_GameControllerGetJoystick(pad),SDL_CONTROLLER_AXIS_RIGHTX,20000)==0);
 SDL_GameControllerUpdate();controls.pollSticks();assert(controls.lastInputDevice()==InputDevice::Controller);
 event={};event.type=SDL_KEYDOWN;event.key.keysym.sym=SDLK_q;controls.event(event);
 controls.pollSticks();assert(controls.lastInputDevice()==InputDevice::Keyboard);
 SDL_GameControllerClose(pad);pad=nullptr;SDL_JoystickDetachVirtual(index);SDL_Quit();
 std::cout<<"SKILL_LABELS_OK\n";
}
