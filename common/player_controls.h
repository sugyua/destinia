#pragma once
#include "user_settings.h"
#include "skill_labels.h"
#include <functional>

namespace destinia {
// Aggregates physical inputs so aliases and controllers cannot release each other.
class PlayerControls {
  const Preferences& preferences;
  InputState inputs;
  SDL_GameController*& controller;
  std::function<void(int,bool)> key;
  std::function<void()> save, releaseTouch;
  InputDevice lastDevice=InputDevice::Keyboard;
  Action previousLeft=Action::None,previousRight=Action::None;
  bool manualPause=false, disconnectedPause=false, neutralRequired=false;
  static uint64_t source(unsigned type,unsigned value) { return (uint64_t(type)<<32)|value; }
  void emit(Action action,bool down) {
    static const int codes[]={3,9,5,7,17,10,27,1,2,11,4,8,12};
    const int index=static_cast<int>(action);
    if(index>=0 && index<13) key(codes[index],down);
    else if(action==Action::Save && down) save();
    else if(action==Action::DropNames) dropNamesHeld=down;
    else if(action==Action::FastForward) accelerating=down;
    else if(action==Action::Pause && down) {
      if(disconnectedPause) { disconnectedPause=false; manualPause=false; }
      else manualPause=!manualPause;
    }
  }
  void input(uint64_t id,Action action,bool down) {
    auto callback=[&](Action a,bool pressed){emit(a,pressed);};
    if(down) {
      if(focused && (!paused() || action==Action::Pause)) inputs.press(id,action,callback);
    } else inputs.release(id,callback);
  }
  static Action axis(int x,int y,bool skill) {
    if(std::abs(x)<16000 && std::abs(y)<16000) return Action::None;
    if(std::abs(x)>std::abs(y)) return x<0?(skill?Action::SkillLeft:Action::Left):(skill?Action::SkillRight:Action::Right);
    return y<0?(skill?Action::SkillUp:Action::Up):(skill?Action::SkillDown:Action::Down);
  }
public:
  bool focused=true,accelerating=false,dropNamesHeld=false;
  PlayerControls(const Preferences& p,SDL_GameController*& pad,std::function<void(int,bool)> k,
                 std::function<void()> s,std::function<void()> touch):preferences(p),controller(pad),key(k),save(s),releaseTouch(touch) {}
  InputDevice lastInputDevice() const { return lastDevice; }
  bool paused() const { return manualPause || disconnectedPause || (!focused && preferences.pauseUnfocused); }
  SDL_JoystickID controllerId() const { return controller?SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller)):-1; }
  void release() {
    inputs.releaseAll([&](Action a,bool down){emit(a,down);});
    releaseTouch(); neutralRequired=true;
  }
  void event(const SDL_Event& ev) {
    if(ev.type==SDL_WINDOWEVENT) {
      if(ev.window.event==SDL_WINDOWEVENT_FOCUS_LOST) { focused=false; release(); }
      else if(ev.window.event==SDL_WINDOWEVENT_FOCUS_GAINED) focused=true;
    } else if(ev.type==SDL_MOUSEBUTTONDOWN || ev.type==SDL_FINGERDOWN) {
      if(focused && !paused())lastDevice=InputDevice::Touch;
    } else if(ev.type==SDL_KEYDOWN || ev.type==SDL_KEYUP) {
      if(ev.key.repeat) return;
      auto found=preferences.keys.find(ev.key.keysym.sym);
      if(found!=preferences.keys.end()) {
        if(ev.type==SDL_KEYDOWN && focused && !paused())lastDevice=InputDevice::Keyboard;
        input(source(1,(unsigned)ev.key.keysym.sym),found->second,ev.type==SDL_KEYDOWN);
      }
    } else if(ev.type==SDL_CONTROLLERDEVICEADDED && !controller) {
      controller=SDL_GameControllerOpen(ev.cdevice.which);
      if(controller) { disconnectedPause=false; neutralRequired=true; hostReport("[host] Controle conectado: %s\n",SDL_GameControllerName(controller)); }
    } else if(ev.type==SDL_CONTROLLERDEVICEREMOVED && controller && ev.cdevice.which==controllerId()) {
      release(); SDL_GameControllerClose(controller); controller=nullptr;
      disconnectedPause=preferences.pauseDisconnected;
      hostReport("[host] Controle desconectado; comandos liberados.\n");
    } else if((ev.type==SDL_CONTROLLERBUTTONDOWN || ev.type==SDL_CONTROLLERBUTTONUP) && controller && ev.cbutton.which==controllerId()) {
      auto found=preferences.buttons.find((SDL_GameControllerButton)ev.cbutton.button);
      if(found!=preferences.buttons.end()) {
        if(ev.type==SDL_CONTROLLERBUTTONDOWN && focused && !paused())lastDevice=InputDevice::Controller;
        input(source(2,ev.cbutton.button),found->second,ev.type==SDL_CONTROLLERBUTTONDOWN);
      }
    }
  }
  void pollSticks() {
    if(!controller || !focused || paused()) return;
    auto left=axis(SDL_GameControllerGetAxis(controller,SDL_CONTROLLER_AXIS_LEFTX),SDL_GameControllerGetAxis(controller,SDL_CONTROLLER_AXIS_LEFTY),false);
    auto right=axis(SDL_GameControllerGetAxis(controller,SDL_CONTROLLER_AXIS_RIGHTX),SDL_GameControllerGetAxis(controller,SDL_CONTROLLER_AXIS_RIGHTY),true);
    if(neutralRequired) { if(left==Action::None && right==Action::None) neutralRequired=false; else return; }
    if((left!=Action::None && left!=previousLeft) || (right!=Action::None && right!=previousRight))lastDevice=InputDevice::Controller;
    previousLeft=left;previousRight=right;
    input(source(3,0),left,left!=Action::None);
    input(source(3,1),right,right!=Action::None);
  }
};
} // namespace destinia
