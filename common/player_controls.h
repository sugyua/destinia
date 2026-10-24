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
  std::function<void()> save, releaseTouch,toggleTouch;
  bool clickDown[2]={false,false},clickPending[2]={false,false},chordUsed=false,chordNeutral=true;
  void resetChord(){clickDown[0]=clickDown[1]=clickPending[0]=clickPending[1]=false;chordUsed=false;chordNeutral=true;}
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
                 std::function<void()> s,std::function<void()> touch,std::function<void()> toggle={}):preferences(p),controller(pad),key(k),save(s),releaseTouch(touch),toggleTouch(toggle) {}
  InputDevice lastInputDevice() const { return lastDevice; }
  bool paused() const { return manualPause || disconnectedPause || (!focused && preferences.pauseUnfocused); }
  SDL_JoystickID controllerId() const { return controller?SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller)):-1; }
  void release() {
    inputs.releaseAll([&](Action a,bool down){emit(a,down);});
    releaseTouch(); neutralRequired=true;
  }
  void pollShortcuts() {
    if(!controller || !focused || !chordNeutral)return;
    if(!SDL_GameControllerGetButton(controller,SDL_CONTROLLER_BUTTON_LEFTSTICK) && !SDL_GameControllerGetButton(controller,SDL_CONTROLLER_BUTTON_RIGHTSTICK)) {
      resetChord();chordNeutral=false;
    }
  }
  // Called before Codex routing too. Defer remapped single clicks until release
  // so their individual action cannot escape before the second click arrives.
  bool touchShortcut(const SDL_Event& ev) {
    if((ev.type!=SDL_CONTROLLERBUTTONDOWN && ev.type!=SDL_CONTROLLERBUTTONUP) || !controller || ev.cbutton.which!=controllerId())return false;
    const int side=ev.cbutton.button==SDL_CONTROLLER_BUTTON_LEFTSTICK?0:ev.cbutton.button==SDL_CONTROLLER_BUTTON_RIGHTSTICK?1:-1;
    if(side<0)return false;
    if(!focused){resetChord();return true;}
    if(chordNeutral){pollShortcuts();return true;}
    const bool down=ev.type==SDL_CONTROLLERBUTTONDOWN;
    if(down) {
      if(clickDown[side])return true;
      clickDown[side]=clickPending[side]=true;
      if(clickDown[0] && clickDown[1] && !chordUsed){chordUsed=true;clickPending[0]=clickPending[1]=false;releaseTouch();if(toggleTouch)toggleTouch();}
    } else {
      if(clickPending[side] && !chordUsed && !paused()) {
        auto binding=preferences.buttons.find((SDL_GameControllerButton)ev.cbutton.button);
        if(binding!=preferences.buttons.end()){input(source(2,ev.cbutton.button),binding->second,true);input(source(2,ev.cbutton.button),binding->second,false);}
      }
      clickDown[side]=clickPending[side]=false;
      if(!clickDown[0] && !clickDown[1])chordUsed=false;
    }
    return true;
  }
  void event(const SDL_Event& ev) {
    if(touchShortcut(ev))return;
    if(ev.type==SDL_WINDOWEVENT) {
      if(ev.window.event==SDL_WINDOWEVENT_FOCUS_LOST) { focused=false; release();resetChord(); }
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
      if(controller) { disconnectedPause=false; neutralRequired=true;resetChord(); hostReport("[host] Controle conectado: %s\n",SDL_GameControllerName(controller)); }
    } else if(ev.type==SDL_CONTROLLERDEVICEREMOVED && controller && ev.cdevice.which==controllerId()) {
      release();resetChord(); SDL_GameControllerClose(controller); controller=nullptr;
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
    pollShortcuts();
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
