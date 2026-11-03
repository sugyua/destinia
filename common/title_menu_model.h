#pragma once
#include "titles.h"

namespace destinia {
enum class TitleMenuAction { Consumed, Equip, Locked, Back, Cancel };

// Extra titles have their own page after the seven unmodified native pages.
// This model never performs IO or mutates a title or character.
struct TitleMenuModel {
 bool open=false;
 int slot=-1;
 size_t selected=0;
 size_t count() const { return allTitles().size()-1; }
 TitleId selectedTitle() const { return allTitles().at(selected+1).id; }
 bool enter(int gameMode,uint32_t menuState,int originalIndex,int character,int key) {
  if(gameMode!=16 || menuState!=2 || originalIndex<48 || originalIndex>49 ||
     character<0 || character>=10 || key!=7) return false;
  open=true;slot=character;selected=0;return true;
 }
 void validate(int gameMode,uint32_t menuState,int character) {
  if(open && (gameMode!=16 || menuState!=2 || character!=slot)) {open=false;slot=-1;}
 }
 TitleMenuAction key(int key,const TitlesStore& titles) {
  if(key>=13&&key<=16) {const int directions[]={3,9,5,7};key=directions[key-13];}
  if(key==3 && selected>0) --selected;
  else if(key==9 && selected+1<count()) ++selected;
  else if(key==7 && (selected/8+1)*8<count()) selected=(selected/8+1)*8;
  else if(key==5 && selected>=8) selected=(selected/8-1)*8;
  else if(key==5) {open=false;return TitleMenuAction::Back;}
  else if(key==10 || key==27) {open=false;return TitleMenuAction::Cancel;}
  else if(key==6 || key==17) return titles.isUnlocked(slot,selectedTitle())?TitleMenuAction::Equip:TitleMenuAction::Locked;
  return TitleMenuAction::Consumed;
 }
};
}
