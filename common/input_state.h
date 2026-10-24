#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <map>

namespace destinia {
enum class Action { None=-1,Up,Down,Left,Right,Attack,Back,Menu,Start,SkillLeft,SkillRight,SkillUp,SkillDown,Extra,Save,FastForward,Pause,DropNames,Count };

// Conta fontes fisicas por acao: W + seta, ou teclado + controle, nao se anulam.
class InputState {
  std::map<uint64_t,Action> sources;
  std::array<unsigned,static_cast<size_t>(Action::Count)> counts{};
public:
  template<class Emit> void release(uint64_t source,Emit emit) {
    auto it=sources.find(source); if(it==sources.end()) return;
    const auto action=it->second; sources.erase(it);
    if(--counts[static_cast<size_t>(action)]==0) emit(action,false);
  }
  template<class Emit> void press(uint64_t source,Action action,Emit emit) {
    auto it=sources.find(source);
    if(it!=sources.end() && it->second==action) return;
    release(source,emit);
    if(action==Action::None) return;
    sources[source]=action;
    if(counts[static_cast<size_t>(action)]++==0) emit(action,true);
  }
  template<class Emit> void releaseAll(Emit emit) {
    sources.clear();
    for(size_t i=0;i<counts.size();++i) if(counts[i]) { counts[i]=0; emit(static_cast<Action>(i),false); }
  }
  bool active(Action action) const { return action!=Action::None && counts[static_cast<size_t>(action)]!=0; }
};
} // namespace destinia
