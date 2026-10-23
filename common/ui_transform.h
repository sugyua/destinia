#pragma once
#include <algorithm>
#include <cmath>
#include <utility>
#include <limits>
namespace destinia {
enum class UiAnchor { TopLeft, TopRight, BottomLeft, BottomRight, Center };
struct UiRect { int x,y,w,h; };
struct UiTransform {
 int width=400,height=240,percent=100;UiAnchor anchor=UiAnchor::Center;int fitWidth=0,fitHeight=0;int fitX=std::numeric_limits<int>::min(),fitY=std::numeric_limits<int>::min();
 double scale() const {double s=percent/100.;if(fitWidth>0)s=std::min(s,double(width)/fitWidth);if(fitHeight>0)s=std::min(s,double(height)/fitHeight);return s;}
 std::pair<int,int> offset() const {auto s=scale();int x=0,y=0;if(anchor==UiAnchor::Center){x=int(std::floor(width*(1-s)/2));y=int(std::floor(height*(1-s)/2));}else{if(anchor==UiAnchor::TopRight||anchor==UiAnchor::BottomRight)x=int(std::floor(width*(1-s)));if(anchor==UiAnchor::BottomLeft||anchor==UiAnchor::BottomRight)y=int(std::floor(height*(1-s)));}if(fitWidth>0){int origin=fitX==std::numeric_limits<int>::min()?(width-fitWidth)/2:fitX;int lo=int(std::floor(origin*s+x)),hi=int(std::floor((origin+fitWidth)*s+x));if(lo<0)x-=lo;else if(hi>width)x-=hi-width;}
 if(fitHeight>0){int origin=fitY==std::numeric_limits<int>::min()?(height-fitHeight)/2:fitY;int lo=int(std::floor(origin*s+y)),hi=int(std::floor((origin+fitHeight)*s+y));if(lo<0)y-=lo;else if(hi>height)y-=hi-height;}return{x,y};}
 std::pair<int,int> point(int x,int y) const {auto o=offset();auto s=scale();return{int(std::floor(x*s+o.first)),int(std::floor(y*s+o.second))};}
 UiRect rect(int x,int y,int w,int h) const {auto a=point(x,y),b=point(x+w,y+h);return{a.first,a.second,b.first-a.first,b.second-a.second};}
 std::pair<int,int> inverse(int x,int y) const {auto o=offset();auto s=scale();return{int(std::ceil((x+1-o.first)/s))-1,int(std::ceil((y+1-o.second)/s))-1};}
};
}
