#define SDL_MAIN_HANDLED
#include "../common/user_settings.h"
#include "../common/input_state.h"
#include <cassert>
#include <fstream>
#include <vector>
#include <utility>
#include <iostream>
namespace fs=std::filesystem;
int main(int argc,char** argv) {
  assert(argc==2); fs::path root=argv[1]; fs::create_directories(root);
  auto config=root/"config.ini";
  {
    auto p=destinia::Preferences::load(config,"host_zoom");
    assert(p.fullscreen && p.width==800 && p.height==480);
    assert(p.music==100 && p.effects==100 && !p.pauseUnfocused && !p.pauseDisconnected);
    assert(p.keys.at(SDLK_w)==destinia::Action::Up);
    assert(p.buttons.at(SDL_CONTROLLER_BUTTON_A)==destinia::Action::Attack);
  }
  std::ofstream(config)<<"save_dir=C:\\partida antiga\ncontrole_perfil=alternativo\n"
    <<"video.host_zoom.largura=1600\nvideo.host_zoom.altura=960\nvideo.host_zoom.tela_cheia=0\n"
    <<"audio.musica=35\naudio.efeitos=70\naudio.silenciar_sem_foco=1\n"
    <<"pausa.sem_foco=1\npausa.sem_controle=1\n"
    <<"perfil.alternativo.tecla.cima=Up,T\nperfil.alternativo.botao.ataque=b\nperfil.alternativo.botao.voltar=a\n";
  auto p=destinia::Preferences::load(config,"host_zoom");
  assert(!p.fullscreen && p.width==1600 && p.height==960 && p.music==35 && p.effects==70);
  assert(p.pauseUnfocused && p.pauseDisconnected && p.muteUnfocused);
  assert(p.keys.at(SDLK_t)==destinia::Action::Up && p.keys.count(SDLK_w)==0);
  assert(p.buttons.at(SDL_CONTROLLER_BUTTON_B)==destinia::Action::Attack);
  destinia::saveWindowPreferences(config,"host_zoom",1200,720,true);
  auto raw=destinia::readPreferencesFile(config);
  assert(raw.at("save_dir")=="C:\\partida antiga");
  assert(raw.at("audio.musica")=="35");
  assert(destinia::Preferences::load(config,"host_zoom").fullscreen);
  for(auto bad:{"audio.musica=101\n","video.host.largura=0\n","perfil.padrao.tecla.cima=TeclaInexistente\n",
                "perfil.padrao.tecla.cima=F11\n","perfil.padrao.tecla.cima=A\n"}) {
    std::ofstream(config)<<bad;
    bool failed=false; try {destinia::Preferences::load(config,"host");} catch(const std::exception&) {failed=true;}
    assert(failed);
  }
  destinia::InputState input;
  std::vector<std::pair<destinia::Action,bool>> events;
  auto emit=[&](destinia::Action action,bool down){events.emplace_back(action,down);};
  input.press(1,destinia::Action::Up,emit); input.press(2,destinia::Action::Up,emit);
  assert(events.size()==1 && events.back().second);
  input.release(1,emit); assert(events.size()==1);
  input.release(2,emit); assert(events.size()==2 && !events.back().second);
  input.press(3,destinia::Action::Left,emit); input.press(3,destinia::Action::Right,emit);
  assert(events[events.size()-2]==std::make_pair(destinia::Action::Left,false));
  input.press(4,destinia::Action::Attack,emit); input.releaseAll(emit);
  assert(!input.active(destinia::Action::Attack) && !input.active(destinia::Action::Right));
  auto count=events.size(); input.releaseAll(emit); assert(events.size()==count);
  assert(destinia::scaledVolume(5,35,false)==44);
  assert(destinia::scaledVolume(3,50,false)==38);
  assert(destinia::scaledVolume(5,100,true)==0);
  std::cout<<"OK: preferencias, preservacao do save, perfis, validacao, fontes simultaneas, liberacao e volume\n";
}
