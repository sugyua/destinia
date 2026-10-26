#pragma once
#include <string>
#include <functional>
#include <algorithm>
#include <cstdint>
#include "progression.h"
#include "progression_store.h"

namespace destinia {

struct NewGamePlusResult {
  bool success = false;
  uint32_t newCycle = 0;
  std::string message;
};

class NewGamePlusManager {
public:
  static NewGamePlusResult advanceCycle(uint32_t slot,
                                       const fs::path& saveDir,
                                       const std::function<void(int, int, int)>& teleport) {
    if (slot > 9) return {false, 0, "SLOT INVALIDO"};

    try {
      auto ledger = progression::load(saveDir, progression::Mode::Expanded);
      if (!ledger) {
        ledger = progression::Ledger{};
        ledger->mode = progression::Mode::Expanded;
      }

      auto& ch = ledger->characters[slot];
      if (ch.ngCycle >= 5) {
        return {false, ch.ngCycle, "CICLO MAXIMO ATINGIDO (NG+5)!"};
      }

      ch.ngCycle++;
      // Salva snapshot no progression-v1.txt
      auto expected = progression::readText(saveDir);
      progression::save(saveDir, *ledger, AtomicSaveOps{}, expected);

      // Teleporta o heroi para o inicio da jornada (Area 0, Mapa 1)
      if (teleport) {
        teleport(0, 1, 1);
      }

      std::string msg = "NOVO JOGO+ ATIVADO (CICLO NG+" + std::to_string(ch.ngCycle) + ")! DIFICULDADE E RECOMPENSAS AUMENTADAS!";
      return {true, ch.ngCycle, msg};
    } catch (const std::exception& e) {
      return {false, 0, std::string("ERRO NG+: ") + e.what()};
    }
  }

  static uint32_t getCycle(uint32_t slot, const fs::path& saveDir) {
    if (slot > 9) return 0;
    try {
      auto ledger = progression::load(saveDir, progression::Mode::Expanded);
      if (!ledger) return 0;
      auto it = ledger->characters.find(slot);
      if (it != ledger->characters.end()) return it->second.ngCycle;
    } catch (...) {}
    return 0;
  }
};

} // namespace destinia
