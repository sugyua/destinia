#define SDL_MAIN_HANDLED
#include "../common/player_controls.h"
#include <cassert>
#include <vector>
#include <utility>
int main(){
 using namespace destinia;
 SDL_SetMainReady();assert(SDL_Init(SDL_INIT_GAMECONTROLLER)==0);
 int index=SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_GAMECONTROLLER,6,16,0);assert(index>=0);
 SDL_GameController* pad=SDL_GameControllerOpen(index);assert(pad);
 Preferences p;p.buttons[SDL_CONTROLLER_BUTTON_LEFTSTICK]=Action::Attack;p.buttons[SDL_CONTROLLER_BUTTON_RIGHTSTICK]=Action::Menu;
 int toggles=0,touchReleases=0;std::vector<std::pair<int,bool>> keys;
 PlayerControls c(p,pad,[&](int k,bool d){keys.emplace_back(k,d);},[]{},[&]{touchReleases++;},[&]{toggles++;});
 auto button=[&](SDL_GameControllerButton b,bool pressed){SDL_JoystickSetVirtualButton(SDL_GameControllerGetJoystick(pad),b,pressed);SDL_GameControllerUpdate();SDL_Event e{};e.type=pressed?SDL_CONTROLLERBUTTONDOWN:SDL_CONTROLLERBUTTONUP;e.cbutton.which=c.controllerId();e.cbutton.button=b;c.event(e);};
 c.pollShortcuts();
 button(SDL_CONTROLLER_BUTTON_LEFTSTICK,true);assert(keys.empty());button(SDL_CONTROLLER_BUTTON_RIGHTSTICK,true);assert(toggles==1&&touchReleases==1&&keys.empty());
 for(int i=0;i<5;i++)c.pollSticks();assert(toggles==1);
 button(SDL_CONTROLLER_BUTTON_LEFTSTICK,false);button(SDL_CONTROLLER_BUTTON_LEFTSTICK,true);assert(toggles==1&&keys.empty());
 button(SDL_CONTROLLER_BUTTON_LEFTSTICK,false);button(SDL_CONTROLLER_BUTTON_RIGHTSTICK,false);button(SDL_CONTROLLER_BUTTON_RIGHTSTICK,true);button(SDL_CONTROLLER_BUTTON_LEFTSTICK,true);assert(toggles==2&&keys.empty());
 button(SDL_CONTROLLER_BUTTON_LEFTSTICK,false);button(SDL_CONTROLLER_BUTTON_RIGHTSTICK,false);
 // An isolated remapped click remains available, on release to disambiguate.
 button(SDL_CONTROLLER_BUTTON_LEFTSTICK,true);assert(keys.empty());button(SDL_CONTROLLER_BUTTON_LEFTSTICK,false);assert((keys==std::vector<std::pair<int,bool>>{{17,true},{17,false}}));keys.clear();
 button(SDL_CONTROLLER_BUTTON_LEFTSTICK,true);SDL_Event focus{};focus.type=SDL_WINDOWEVENT;focus.window.event=SDL_WINDOWEVENT_FOCUS_LOST;c.event(focus);button(SDL_CONTROLLER_BUTTON_RIGHTSTICK,true);assert(toggles==2&&keys.empty());focus.window.event=SDL_WINDOWEVENT_FOCUS_GAINED;c.event(focus);c.pollShortcuts();assert(toggles==2);button(SDL_CONTROLLER_BUTTON_LEFTSTICK,false);button(SDL_CONTROLLER_BUTTON_RIGHTSTICK,false);c.pollShortcuts();
 button(SDL_CONTROLLER_BUTTON_LEFTSTICK,true);button(SDL_CONTROLLER_BUTTON_RIGHTSTICK,true);assert(toggles==3&&keys.empty());
 // Other controllers must never toggle this player's setting.
 SDL_Event alien{};alien.type=SDL_CONTROLLERBUTTONDOWN;alien.cbutton.which=c.controllerId()+1;alien.cbutton.button=SDL_CONTROLLER_BUTTON_LEFTSTICK;c.event(alien);assert(toggles==3);
 SDL_Event gone{};gone.type=SDL_CONTROLLERDEVICEREMOVED;gone.cdevice.which=c.controllerId();c.event(gone);assert(!pad);// no callback after removal
 assert(toggles==3);
 SDL_Event added{};added.type=SDL_CONTROLLERDEVICEADDED;added.cdevice.which=index;c.event(added);assert(pad);c.pollShortcuts();assert(toggles==3);
 button(SDL_CONTROLLER_BUTTON_LEFTSTICK,false);button(SDL_CONTROLLER_BUTTON_RIGHTSTICK,false);c.pollShortcuts();
 button(SDL_CONTROLLER_BUTTON_LEFTSTICK,true);button(SDL_CONTROLLER_BUTTON_RIGHTSTICK,true);assert(toggles==4);
 SDL_GameControllerClose(pad);pad=nullptr;SDL_JoystickDetachVirtual(index);SDL_Quit();
}
