#pragma once
#include "loot_economy.h"
#include "loot_economy_store.h"
#include <ctime>

namespace destinia {

static LootEconomyState lootEconomy;
static bool lootEconomyReady = false;
static bool lootEconomyOfflineAwarded = false;

inline void initLootEconomy(const std::string& directory) {
  try {
    loadLootEconomy(fs::path(directory), lootEconomy);
    lootEconomyReady = true;
  } catch (const std::exception& e) {
    hostReport("[economia] Falha ao carregar economia: %s; preservando estado.\n", e.what());
    lootEconomyReady = false;
  }
}

inline void lootEconomyOnBossKilled(const std::string& directory, int slot, int species) {
  if (species < 100 || species > 109) return;
  lootEconomy.recordBossKill(slot, species);
  const auto* drop = findBossDrop(species);
  if (drop) {
    hostReport("[boss drop] Chefe derrotado: %s (species %d)! Drop exclusivo: %s\n",
               drop->bossName.c_str(), species, drop->namePt.c_str());
  }
  lootEconomy.checkAndClaimEpicMissions(slot);
  try {
    saveLootEconomy(fs::path(directory), lootEconomy);
  } catch (const std::exception& e) {
    hostReport("[economia] Erro salvando boss drop / missoes: %s\n", e.what());
  }
}

template <typename CallFn>
inline void lootEconomyOnBossKilled(
    const std::string& directory,
    int slot,
    int species,
    u32 fnGetCash,
    u32 fnSetCash,
    CallFn callFn) {
  if (species < 100 || species > 109) return;
  lootEconomy.recordBossKill(slot, species);
  const auto* drop = findBossDrop(species);
  if (drop) {
    hostReport("[boss drop] Chefe derrotado: %s (species %d)! Drop exclusivo: %s\n",
               drop->bossName.c_str(), species, drop->namePt.c_str());
  }
  int missionCash = lootEconomy.checkAndClaimEpicMissions(slot);
  if (missionCash > 0 && fnGetCash && fnSetCash) {
    u32 cur = 0;
    if (callFn(fnGetCash, {}, &cur)) {
      u32 nextCash = cur + u32(missionCash);
      callFn(fnSetCash, {nextCash}, nullptr);
      hostReport("[economia] Missao Epica concluida! +%d Cash creditados (novo saldo: %u Cash).\n",
                 missionCash, nextCash);
    }
  }
  try {
    saveLootEconomy(fs::path(directory), lootEconomy);
  } catch (const std::exception& e) {
    hostReport("[economia] Erro salvando boss drop / missoes: %s\n", e.what());
  }
}

template <typename CallFn>
inline void lootEconomyProcessOffline(
    const std::string& directory,
    u32 fnGetCash,
    u32 fnSetCash,
    CallFn callFn) {
  if (lootEconomyOfflineAwarded || !fnGetCash || !fnSetCash) return;
  const auto now = int64_t(std::time(nullptr));
  auto res = lootEconomy.processOfflineCash(now);
  if (res.cashToCredit > 0) {
    u32 current = 0;
    if (callFn(fnGetCash, {}, &current)) {
      u32 nextCash = current + u32(res.cashToCredit);
      callFn(fnSetCash, {nextCash}, nullptr);
      hostReport("[economia] Cash offline creditado: +%d (anterior: %u, novo: %u, %llds offline, teto 24h: %s)\n",
                 res.cashToCredit, current, nextCash, (long long)res.cappedSeconds, res.capped ? "SIM" : "NAO");
    }
  }
  lootEconomyOfflineAwarded = true;
  try {
    saveLootEconomy(fs::path(directory), lootEconomy);
  } catch (const std::exception& e) {
    hostReport("[economia] Erro salvando timestamp de economia: %s\n", e.what());
  }
}

inline void lootEconomyPeriodicSave(const std::string& directory, uint64_t frame) {
  if (frame > 0 && frame % 3600 == 0 && lootEconomyReady) {
    lootEconomy.lastTimestamp = int64_t(std::time(nullptr));
    try {
      saveLootEconomy(fs::path(directory), lootEconomy);
    } catch (...) {}
  }
}

} // namespace destinia
