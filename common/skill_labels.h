#pragma once
#include "user_settings.h"
namespace destinia {
enum class InputDevice { Keyboard, Controller, Touch };
// Verified against GameCanvas_mKeyPressed -> _pressedQuickSlot in the original ARM.
inline Action quickSlotAction(unsigned index) {
 switch(index){case 0:return Action::SkillLeft;case 1:return Action::SkillDown;case 2:return Action::SkillRight;case 3:return Action::SkillUp;default:return Action::None;}
}
// Show only configured aliases. Right-stick skills are implemented independently
// of the button map in PlayerControls, so retain that actual input route.
inline std::string skillBindingLabel(const Preferences& preferences,Action action,InputDevice device) {
 if(device==InputDevice::Touch) return {};
 std::string label;
 auto append=[&](const char* name){if(!name || !*name)return;if(!label.empty())label+=" / ";label+=name;};
 if(device==InputDevice::Keyboard) {
  for(const auto& binding:preferences.keys)if(binding.second==action)append(SDL_GetKeyName(binding.first));
  return label.empty()?"SEM TECLA":label;
 }
 for(const auto& binding:preferences.buttons)if(binding.second==action)append(SDL_GameControllerGetStringForButton(binding.first));
 switch(action) {
  case Action::SkillLeft:append("RS <");break;
  case Action::SkillRight:append("RS >");break;
  case Action::SkillUp:append("RS ^");break;
  case Action::SkillDown:append("RS v");break;
  default:break;
 }
 return label;
}
} // namespace destinia
