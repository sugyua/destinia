#include "../common/quest_tracker_ui.h"
#include <cassert>
#include <vector>
int main(){
 for(int width:{400,800}){
  int height=width*3/5,panels=0,texts=0;int bx=0,by=0,bw=0,bh=0;
  destinia::originalPanelPainter=[&](uint8_t*,int,int,int x,int y,int w,int h){panels++;bx=x;by=y;bw=w;bh=h;return true;};
  destinia::originalTextPainter=[&](uint8_t*,int,int,int,int,const std::string&,uint16_t,int,int){texts++;return true;};
  std::vector<uint8_t> frame(width*height*2);destinia::QuestTrackerUI q;q.paint(frame.data(),width,height);
  assert(panels==1&&texts>=3);assert(bx>=0&&by>=0&&bx+bw<=width&&by+bh<=height);
  assert(q.handleTouch(bx+bw-4,by+4,0,width,height));assert(q.minimized);
 }
}
