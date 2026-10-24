#include <unicorn/unicorn.h>
#include <cassert>
static uc_err keepEngine(uc_engine*){return UC_ERR_OK;}
#define uc_close keepEngine
#define main originalMain
#ifdef DESTINIA_TEST_ZOOM
#include "../host_zoom/main.cpp"
#else
#include "../host/main.cpp"
#endif
#undef main
#undef uc_close
#include "../common/ui_renderer.h"
namespace destinia {struct UiRendererFixture {static void panel(UiRenderer& ui,bool value){ui.kind=value?UiRenderer::Kind::Panel:UiRenderer::Kind::None;}static void alphaMultiply(UiRenderer& ui,uint32_t dst,uint32_t source,uint32_t palette){ui.realBase=dst;ui.matrix={ui.width,ui.height,100,UiAnchor::TopLeft};ui.indexed(UiRenderer::virtualBase+2*((UiRenderer::padding+36)*UiRenderer::virtualSide+UiRenderer::padding+40),source,palette,2,1,2,2,ui.width*2,4,ui.base+0x9468);}};}
int main(int argc,char**argv){
 assert(argc>2);int percent=atoi(argv[1]);if(int result=originalMain(argc-1,argv+1))return result;
 auto grp=exportsMap.at("Grp"),handle=rd32(grp+4);u32 pixels=0;assert(callFn(exportsMap.at("MC_GRP_GET_FRAME_BUFFER_POINTER"),{handle},&pixels));
 destinia::UiRenderer ui;ui.install(uc,SO_BASE,exportsMap,SCREEN_W,SCREEN_H,percent,false,soPath);uc_ctl_remove_cache(uc,SO_BASE,SO_BASE+0x100000);
 std::vector<uint16_t> background(SCREEN_W*SCREEN_H),expected(background.size()),actual(background.size());for(size_t i=0;i<background.size();++i)background[i]=uint16_t(i*31337u);
 destinia::UiTransform matrix{SCREEN_W,SCREEN_H,percent,destinia::UiAnchor::Center,232,240};
 for(int alpha:{0,1})for(int x:{40,-8,395}){
  destinia::UiRendererFixture::panel(ui,false);assert(callFn(exportsMap.at("Grp_setColorPixel"),{0x5c39}));assert(callFn(exportsMap.at("Grp_setDrawOpVal"),{u32(alpha),7}));
  auto box=matrix.rect(x,36,16,48);if(percent!=100){int right=std::min(SCREEN_W,box.x+box.w),bottom=std::min(SCREEN_H,box.y+box.h);box.x=std::max(0,box.x);box.y=std::max(0,box.y);box.w=std::max(0,right-box.x);box.h=std::max(0,bottom-box.y); }wr(pixels,background.data(),FB_BYTES);assert(callFn(exportsMap.at("Grp_fillRect"),{handle,u32(box.x),u32(box.y),u32(box.w),u32(box.h)}));rd(pixels,expected.data(),FB_BYTES);
  wr(pixels,background.data(),FB_BYTES);destinia::UiRendererFixture::panel(ui,true);assert(callFn(exportsMap.at("Grp_fillRect"),{handle,u32(x),36,16,48}));assert(ui.ok());rd(pixels,actual.data(),FB_BYTES);
  if(actual!=expected){fprintf(stderr,"UI_MISMATCH percent=%d alpha=%d x=%d error=%s\n",percent,alpha,x,ui.error.c_str());return 4;}
 }
 destinia::UiRendererFixture::panel(ui,false);
 // Compare alpha-multiply against the original ARM primitive, including background.
 const auto source=hmalloc(2),palette=hmalloc(512);uint8_t indices[2]={0,1};uint16_t colors[256]={0x5c39,0x7bef};wr(source,indices,2);wr(palette,colors,512);
 assert(callFn(exportsMap.at("Grp_setDrawOpVal"),{1,7}));wr(pixels,background.data(),FB_BYTES);
 assert(callFn(exportsMap.at("_Grp_drawAlpMultiply16"),{pixels+2*(36*SCREEN_W+40),source,palette,2,1,0,1,u32(SCREEN_W-2)}));rd(pixels,expected.data(),FB_BYTES);
 wr(pixels,background.data(),FB_BYTES);destinia::UiRendererFixture::alphaMultiply(ui,pixels,source,palette);assert(ui.ok());rd(pixels,actual.data(),FB_BYTES);assert(actual==expected);
 const auto main=exportsMap.at("MMain"),game=exportsMap.at("Game"),gameUi=exportsMap.at("GameUI"),runner=rd32(SO_BASE+0x44240);wr32(runner,exportsMap.at("GameCanvas"));
 assert(callFn(exportsMap.at("GameCanvas_init"),{exportsMap.at("GameCanvas")}));
 const auto table=exportsMap.at("frameBuffer"),screenDescriptor=hmalloc(12),screenPixels=hmalloc(FB_BYTES);wr32(screenDescriptor,SCREEN_W);wr32(screenDescriptor+4,SCREEN_H);wr32(screenDescriptor+8,screenPixels);wr32(table,screenDescriptor);wr(screenPixels,background.data(),FB_BYTES);
 auto beforeControls=ui.draws;
 for(int state:{0,-1,16,17}){uint16_t value=uint16_t(state==17?16:state);wr(game+0x4854,&value,2);wr32(gameUi+0x7c,state==17?0x20000001:0);assert(callFn(exportsMap.at("MMain_drawTouchButton"),{0}));assert(ui.ok());}
 assert(percent==100||ui.draws>beforeControls);
 assert(callFn(exportsMap.at("Map_loadData"),{2,49,1}));assert(callFn(exportsMap.at("Mon_createMonster"),{}));uint16_t active=0;wr(game+0x4854,&active,2);wr32(gameUi+0x7c,0);u32 wait=0,screenArray=0;for(const auto& a:arrays)if(a.second.size()==FB_BYTES){screenArray=a.first;break;}assert(screenArray);
 assert(callFn(exportsMap.at("Java_game_destiniaeng_GameThread_jniRun"),{ENV_OBJ,0x10,screenArray,100},&wait));assert(ui.ok());

 const auto pointBuffer=hmalloc(8);int16_t empty[4]={-1000,-1000,0,0};for(int i=0;i<15;++i)wr(main+0xea+i*8,empty,8);for(u32 at:{0xd2,0xda,0xe2})wr(main+at,empty,8);
 // _getTouchState returns1 for GameCanvas with game state>28. A fresh
 // MenuCanvas instead returns0; selecting it does not prepare touch state1.
 for(int state:{2,5,1}){
  uint16_t gameState=state==5?16:state==1?29:0;
  wr(game+0x4854,&gameState,2);
  wr32(gameUi+0x7c,state==5?0x20000001:0);
  wr32(runner,exportsMap.at("GameCanvas"));
  u32 actualState=0;assert(callFn(exportsMap.at("_getTouchState"),{},&actualState));
  assert(actualState==u32(state));
  const int index=state==2?4:state==5?12:14;
  int16_t hit[4]={100,100,16,16};wr(main+0xea+index*8,hit,8);
  destinia::UiTransform t{SCREEN_W,SCREEN_H,percent,state==1?destinia::UiAnchor::Center:destinia::UiAnchor::TopLeft,state==1?232:0,state==1?240:0};
  const auto p=t.point(105,105),native=ui.touchPoint(p.first,p.second,state);
  wr32(pointBuffer,u32(native.first));wr32(pointBuffer+4,u32(native.second));
  u32 key=0;assert(callFn(exportsMap.at("getKeyPadIdx"),{pointBuffer},&key));
  if(key!=u32(index+1)){fprintf(stderr,"UI_TOUCH_FAIL pct=%d state=%d key=%u expected=%d\n",percent,state,key,index+1);return 8;}
  wr(main+0xea+index*8,empty,8);
 }

 printf("UI_RENDERER_FILL_CLIP_ALPHA_OK percent=%d draws=%llu\n",percent,(unsigned long long)ui.draws);return 0;
}
