#pragma once
#include "user_settings.h"
#include "overlay_font.h"
#include "ui_transform.h"
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
struct SkillHintRect {int x,y,w,h;};
inline void paintSkillHints(uint8_t* pixels,int width,int height,const Preferences& p,InputDevice device,const std::array<SkillHintRect,4>& slots,int percent){
 if(device==InputDevice::Touch)return;
 for(size_t i=0;i<slots.size();++i){const auto box=slots[i];if(box.w<=0||box.h<=0||box.x<0||box.y<0||box.x>=width||box.y>=height)continue;auto label=skillBindingLabel(p,quickSlotAction(unsigned(i)),device);if(label=="SEM TECLA")label="-";label=overlayTextClip(label,8);if(label.empty())continue;
  const int tw=overlayTextWidth(label),left=std::clamp(box.x+box.w/2-tw/2,0,std::max(0,width-tw)),top=std::max(2,box.y-9);
  UiTransform t{width,height,percent,box.x>=width/2?(box.y>=height/2?UiAnchor::BottomRight:UiAnchor::TopRight):(box.y>=height/2?UiAnchor::BottomLeft:UiAnchor::TopLeft)};
  auto pixel=[&](int x,int y,uint16_t color){auto a=t.rect(x,y,1,1);for(int yy=std::max(0,a.y);yy<std::min(height,a.y+a.h);++yy)for(int xx=std::max(0,a.x);xx<std::min(width,a.x+a.w);++xx){size_t n=size_t(yy*width+xx)*2;pixels[n]=color&255;pixels[n+1]=color>>8;}};
  for(int y=top-2;y<top+8;++y)for(int x=left-1;x<left+tw;++x)pixel(x,y,0x1946);
  int cursor=left;for(size_t at=0;at<label.size();cursor+=6){auto glyph=overlayGlyph(overlayCodepoint(label,at));for(int x=0;x<5;++x)for(int y=0;y<11;++y)if(glyph[x]&(1<<y))pixel(cursor+x,top+y-2,0xffff);}
 }
}
} // namespace destinia
