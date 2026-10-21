#pragma once
#include "runtime_support.h"
#include "input_state.h"
#include <SDL.h>
#include <cctype>
#include <set>

namespace destinia {
using PreferenceValues=std::map<std::string,std::string>;
inline PreferenceValues readPreferencesFile(const fs::path& file) {
  PreferenceValues result;
  if(file.empty() || !fs::exists(file)) return result;
  std::ifstream in(file,std::ios::binary);
  if(!in) throw std::runtime_error("Nao foi possivel ler as preferencias.");
  std::string line;
  while(std::getline(in,line)) {
    if(line.compare(0,3,"\xef\xbb\xbf")==0) line.erase(0,3);
    line=trim(line); if(line.empty() || line[0]=='#' || line[0]==';') continue;
    const auto equal=line.find('=');
    if(equal==std::string::npos) throw std::runtime_error("Linha invalida nas preferencias.");
    auto key=trim(line.substr(0,equal));
    if(key.empty() || result.count(key)) throw std::runtime_error("Chave vazia ou repetida nas preferencias: "+key);
    result[key]=trim(line.substr(equal+1));
  }
  return result;
}
inline fs::path preferencesFile() { return localDataDir().empty()?fs::path():localDataDir()/"config.ini"; }
inline int preferenceInt(const PreferenceValues& values,const std::string& key,int fallback,int low,int high) {
  auto it=values.find(key); if(it==values.end()) return fallback;
  size_t used=0; int parsed=0;
  try { parsed=std::stoi(it->second,&used); } catch(...) { throw std::runtime_error("Valor invalido em "+key); }
  if(used!=it->second.size() || parsed<low || parsed>high) throw std::runtime_error("Valor fora dos limites em "+key);
  return parsed;
}
inline bool validProfileName(const std::string& value) {
  if(value.empty() || value.size()>32) return false;
  for(unsigned char c:value) if(!std::isalnum(c) && c!='_' && c!='-') return false;
  return true;
}
struct ActionDefaults {
  Action action; const char* name;
  std::vector<SDL_Keycode> keys;
  std::vector<SDL_GameControllerButton> buttons;
};
inline const std::vector<ActionDefaults>& actionDefaults() {
  static const std::vector<ActionDefaults> data={
    {Action::Up,"cima",{SDLK_UP,SDLK_w},{SDL_CONTROLLER_BUTTON_DPAD_UP}},
    {Action::Down,"baixo",{SDLK_DOWN,SDLK_s},{SDL_CONTROLLER_BUTTON_DPAD_DOWN}},
    {Action::Left,"esquerda",{SDLK_LEFT,SDLK_a},{SDL_CONTROLLER_BUTTON_DPAD_LEFT}},
    {Action::Right,"direita",{SDLK_RIGHT,SDLK_d},{SDL_CONTROLLER_BUTTON_DPAD_RIGHT}},
    {Action::Attack,"ataque",{SDLK_RETURN,SDLK_SPACE,SDLK_z},{SDL_CONTROLLER_BUTTON_A}},
    {Action::Back,"voltar",{SDLK_ESCAPE,SDLK_x},{SDL_CONTROLLER_BUTTON_B}},
    {Action::Menu,"inventario",{SDLK_i,SDLK_c},{SDL_CONTROLLER_BUTTON_Y}},
    {Action::Start,"iniciar",{SDLK_TAB},{SDL_CONTROLLER_BUTTON_START}},
    {Action::SkillLeft,"habilidade_esquerda",{},{}},
    {Action::SkillRight,"habilidade_direita",{},{}},
    {Action::SkillUp,"habilidade_cima",{},{}},
    {Action::SkillDown,"habilidade_baixo",{},{}},
    {Action::Extra,"acao_extra",{},{SDL_CONTROLLER_BUTTON_X,SDL_CONTROLLER_BUTTON_RIGHTSHOULDER}},
    {Action::Save,"salvar",{SDLK_F5},{SDL_CONTROLLER_BUTTON_BACK}},
    {Action::FastForward,"acelerar",{SDLK_LSHIFT},{SDL_CONTROLLER_BUTTON_LEFTSHOULDER}},
    {Action::Pause,"pausar",{SDLK_F10},{}}
  };
  return data;
}
inline std::vector<std::string> bindingNames(const std::string& input) {
  std::vector<std::string> names; size_t start=0;
  do {
    auto comma=input.find(',',start); auto name=trim(input.substr(start,comma==std::string::npos?comma:comma-start));
    std::string lowered=name; std::transform(lowered.begin(),lowered.end(),lowered.begin(),[](unsigned char c){return static_cast<char>(std::tolower(c));});
    if(!name.empty() && lowered!="none" && lowered!="nenhum" && name!="-") names.push_back(name);
    if(comma==std::string::npos) break;
    start=comma+1;
  } while(start<=input.size());
  return names;
}
inline int scaledVolume(int nativeLevel,int percent,bool muted) {
  return muted?0:128*std::clamp(nativeLevel,0,5)*std::clamp(percent,0,100)/500;
}
struct Preferences {
  bool fullscreen=false,muteUnfocused=false,pauseUnfocused=false,pauseDisconnected=false;
  int width=1200,height=720,music=100,effects=100;
  std::string profile="padrao";
  std::map<SDL_Keycode,Action> keys;
  std::map<SDL_GameControllerButton,Action> buttons;
  static Preferences load(const fs::path& file,const std::string& host) {
    Preferences p; const auto values=readPreferencesFile(file); const bool zoom=host=="host_zoom";
    const std::string video="video."+host+".";
    p.width=preferenceInt(values,video+"largura",zoom?800:1200,400,7680);
    p.height=preferenceInt(values,video+"altura",zoom?480:720,240,4320);
    p.fullscreen=preferenceInt(values,video+"tela_cheia",zoom?1:0,0,1)!=0;
    p.music=preferenceInt(values,"audio.musica",100,0,100);
    p.effects=preferenceInt(values,"audio.efeitos",100,0,100);
    p.muteUnfocused=preferenceInt(values,"audio.silenciar_sem_foco",0,0,1)!=0;
    p.pauseUnfocused=preferenceInt(values,"pausa.sem_foco",0,0,1)!=0;
    p.pauseDisconnected=preferenceInt(values,"pausa.sem_controle",0,0,1)!=0;
    if(values.count("controle_perfil")) p.profile=values.at("controle_perfil");
    if(!validProfileName(p.profile)) throw std::runtime_error("Nome de perfil invalido.");
    for(const auto& action:actionDefaults()) {
      auto keys=action.keys; auto buttons=action.buttons;
      const std::string prefix="perfil."+p.profile+".";
      auto keyboard=values.find(prefix+"tecla."+action.name),pad=values.find(prefix+"botao."+action.name);
      if(keyboard!=values.end()) {
        keys.clear();
        for(const auto& name:bindingNames(keyboard->second)) {
          auto code=SDL_GetKeyFromName(name.c_str());
          if(code==SDLK_UNKNOWN) throw std::runtime_error("Tecla desconhecida em "+keyboard->first+": "+name);
          keys.push_back(code);
        }
      }
      if(pad!=values.end()) {
        buttons.clear();
        for(const auto& name:bindingNames(pad->second)) {
          auto code=SDL_GameControllerGetButtonFromString(name.c_str());
          if(code==SDL_CONTROLLER_BUTTON_INVALID) throw std::runtime_error("Botao desconhecido em "+pad->first+": "+name);
          buttons.push_back(code);
        }
      }
      for(auto key:keys) {
        if(key==SDLK_F9 || key==SDLK_F11) throw std::runtime_error("F9 e F11 sao reservados para trace e tela cheia.");
        if(p.keys.count(key) && p.keys[key]!=action.action) throw std::runtime_error("Tecla em conflito: "+std::string(SDL_GetKeyName(key)));
        p.keys[key]=action.action;
      }
      for(auto button:buttons) {
        if(p.buttons.count(button) && p.buttons[button]!=action.action) throw std::runtime_error("Botao em conflito: "+std::string(SDL_GameControllerGetStringForButton(button)));
        p.buttons[button]=action.action;
      }
    }
    return p;
  }
};

inline void saveWindowPreferences(const fs::path& file,const std::string& host,int width,int height,bool fullscreen) {
  if(file.empty()) return;
  fs::create_directories(file.parent_path());
  FileLock lock;
  if(!lock.acquire(file.parent_path()/"config.lock")) throw std::runtime_error("Preferencias em uso; tamanho da janela nao foi salvo.");
  auto values=readPreferencesFile(file); const std::string prefix="video."+host+".";
  values[prefix+"largura"]=std::to_string(std::clamp(width,400,7680));
  values[prefix+"altura"]=std::to_string(std::clamp(height,240,4320));
  values[prefix+"tela_cheia"]=fullscreen?"1":"0";
  const fs::path temporary=file.string()+"."+uniqueId()+".tmp";
  try {
    std::ofstream out(temporary,std::ios::binary);
    for(const auto& entry:values) out<<entry.first<<"="<<entry.second<<"\n";
    out.flush(); out.close(); if(!out) throw std::runtime_error("Falha ao escrever preferencias.");
#ifdef _WIN32
    if(!MoveFileExW(temporary.c_str(),file.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)) throw std::runtime_error("Falha ao substituir preferencias.");
#else
    fs::rename(temporary,file);
#endif
  } catch(...) { std::error_code ignored; fs::remove(temporary,ignored); throw; }
}
} // namespace destinia
