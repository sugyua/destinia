#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>
#include <unicorn/unicorn.h>
#include <cstdio>
#include <cstdlib>
#include <string>
static std::string scenario;
static int tick=0,stage=0,runCalls=0,virtualIndex=-1;
static SDL_JoystickID virtualInstance=-1;
static bool mutedDuringFocusLoss=false,volumeRestored=false;
static int fixtureInit(Uint32 flags) {
  int result=SDL_Init(flags);
  if(result==0 && scenario=="disconnect") {
    virtualIndex=SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_GAMECONTROLLER,SDL_CONTROLLER_AXIS_MAX,SDL_CONTROLLER_BUTTON_MAX,0);
    virtualInstance=SDL_JoystickGetDeviceInstanceID(virtualIndex);
  }
  return result;
}
static int fixturePoll(SDL_Event* e) {
  SDL_zero(*e);
  auto key=[&](SDL_Keycode code,bool down){e->type=down?SDL_KEYDOWN:SDL_KEYUP;e->key.keysym.sym=code;return 1;};
  if(scenario=="focus") {
    if(tick==0 && stage<3) { int step=stage++; return key(step==1?SDLK_UP:SDLK_w,step!=2); }
    if((tick==1 || tick==3) && stage++==0) {e->type=SDL_WINDOWEVENT;e->window.event=tick==1?SDL_WINDOWEVENT_FOCUS_LOST:SDL_WINDOWEVENT_FOCUS_GAINED;return 1;}
    if(tick==2) mutedDuringFocusLoss=Mix_VolumeMusic(-1)==0;
    if(tick==4) {volumeRestored=Mix_VolumeMusic(-1)>0;e->type=SDL_QUIT;++tick;return 1;}
  } else if(scenario=="disconnect") {
    if(tick==0 && stage<2) {
      if(stage++==0) return key(SDLK_d,true);
      e->type=SDL_CONTROLLERBUTTONDOWN;e->cbutton.button=SDL_CONTROLLER_BUTTON_DPAD_RIGHT;e->cbutton.which=virtualInstance;return 1;
    }
    if((tick==1 || tick==2) && stage++==0) {
      e->type=SDL_CONTROLLERDEVICEREMOVED;e->cdevice.which=virtualInstance+(tick==1?100:0);
      if(tick==2) SDL_JoystickDetachVirtual(virtualIndex);
      return 1;
    }
    if(tick==4 && stage++==0) return key(SDLK_F10,true);
    if(tick==5) {e->type=SDL_QUIT;++tick;return 1;}
  }
  if(tick>6) return 0;
  ++tick;stage=0;return 0;
}
static uc_err fixtureEmulate(uc_engine* u,uint64_t begin,uint64_t end,uint64_t timeout,size_t count) {
  if(begin==0x10007c5d) ++runCalls;
  return uc_emu_start(u,begin,end,timeout,count);
}
#define SDL_Init fixtureInit
#define SDL_PollEvent fixturePoll
#define uc_emu_start fixtureEmulate
#define main hostMain
#ifdef DESTINIA_TEST_ZOOM
#include "../host_zoom/main.cpp"
#else
#include "../host/main.cpp"
#endif
#undef main
#undef SDL_Init
#undef SDL_PollEvent
#undef uc_emu_start

int main(int argc,char** argv) {
  if(argc<3) return 20;
  scenario=argv[1]; int result=hostMain(argc-1,argv+1);
  if(result) return result;
  if(scenario=="audio") {
    if(SDL_Init(SDL_INIT_AUDIO)!=0) return 21;
    if((Mix_Init(MIX_INIT_OGG|MIX_INIT_MP3)&(MIX_INIT_OGG|MIX_INIT_MP3))!=(MIX_INIT_OGG|MIX_INIT_MP3)) {std::fprintf(stderr,"codecs: %s\n",Mix_GetError());return 22;}
    if(Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,1024)!=0) return 23;
    Mix_AllocateChannels(16);audioReady=true;currentBgTrack=-1;
    setSoundVol(1,5);setSoundVol(0,5);
    if(Mix_VolumeMusic(-1)!=44 || Mix_Volume(-1,-1)!=89) {std::fprintf(stderr,"volumes=%d,%d\n",Mix_VolumeMusic(-1),Mix_Volume(-1,-1));return 24;}
    setSoundVol(1,3);setSoundVol(0,2);
    if(Mix_VolumeMusic(-1)!=26 || Mix_Volume(-1,-1)!=35) return 25;
    playSoundId(100);playSoundId(0);
    if(!Mix_PlayingMusic() || !Mix_Playing(-1)) {std::fprintf(stderr,"decodificacao: %s\n",Mix_GetError());return 26;}
    Mix_HaltMusic();Mix_HaltChannel(-1);
    for(auto& p:sfxCache) Mix_FreeChunk(p.second);
    for(auto& p:musCache) Mix_FreeMusic(p.second);
    Mix_CloseAudio();Mix_Quit();SDL_Quit();
  }
  std::printf("CASE_OK runs=%d muted=%d restored=%d\n",runCalls,mutedDuringFocusLoss,volumeRestored);
  return 0;
}
