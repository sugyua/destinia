#pragma once
#include <cstdint>
#include <string>
#include <algorithm>
#include <functional>

namespace destinia {

struct ResetResult {
  bool success = false;
  std::string message;
  int pointsRefunded = 0;
};

inline ResetResult resetHeroStats(uint32_t gameBase,
                                  const std::function<int()>& getGold,
                                  const std::function<void(int)>& setGold,
                                  const std::function<bool()>& calcHeroStat,
                                  const std::function<uint16_t(uint32_t)>& rd16,
                                  const std::function<void(uint32_t, uint16_t)>& wr16,
                                  int goldCost = 5000) {
  if (!gameBase || !getGold || !setGold || !rd16 || !wr16) {
    return {false, "ERRO: SISTEMA DE HEROI INDISPONIVEL", 0};
  }

  int curGold = getGold();
  if (curGold < goldCost) {
    return {false, "OURO INSUFICIENTE (REQUER " + std::to_string(goldCost) + " G)", 0};
  }

  uint16_t level = rd16(gameBase + 0x48fa);
  if (level < 1) level = 1;

  // Pontos de atributo concedidos por nivel: (level - 1) * 3 pontos
  uint16_t refundedPoints = (level > 1) ? uint16_t((level - 1) * 3) : 0;

  // Restaurar atributos base (FOR, DES, CON, INT) para 10 base de classe
  wr16(gameBase + 0x48ba, 10); // FOR
  wr16(gameBase + 0x48bc, 10); // DES
  wr16(gameBase + 0x48be, 10); // CON
  wr16(gameBase + 0x48c0, 10); // INT

  // Atribuir pontos nao gastos a reserva
  uint16_t existingPoints = rd16(gameBase + 0x48f0);
  wr16(gameBase + 0x48f0, existingPoints + refundedPoints);

  // Descontar ouro
  setGold(curGold - goldCost);

  // Recalcular status de combate no motor ARM
  if (calcHeroStat) {
    calcHeroStat();
  }

  return {true, "ATRIBUTOS RESETADOS! +" + std::to_string(refundedPoints) + " PONTOS", int(refundedPoints)};
}

inline ResetResult resetHeroSkills(uint32_t gameBase,
                                   const std::function<int()>& getGold,
                                   const std::function<void(int)>& setGold,
                                   const std::function<uint16_t(uint32_t)>& rd16,
                                   const std::function<void(uint32_t, uint16_t)>& wr16,
                                   int goldCost = 5000) {
  if (!gameBase || !getGold || !setGold || !rd16 || !wr16) {
    return {false, "ERRO: SISTEMA DE HABILIDADES INDISPONIVEL", 0};
  }

  int curGold = getGold();
  if (curGold < goldCost) {
    return {false, "OURO INSUFICIENTE (REQUER " + std::to_string(goldCost) + " G)", 0};
  }

  uint16_t level = rd16(gameBase + 0x48fa);
  if (level < 1) level = 1;

  // 1 ponto de habilidade por nivel
  uint16_t refundedPoints = (level > 1) ? uint16_t(level - 1) : 0;

  uint16_t existingPoints = rd16(gameBase + 0x48ee);
  wr16(gameBase + 0x48ee, existingPoints + refundedPoints);

  // Descontar ouro
  setGold(curGold - goldCost);

  return {true, "HABILIDADES RESETADAS! +" + std::to_string(refundedPoints) + " PONTOS", int(refundedPoints)};
}

} // namespace destinia
