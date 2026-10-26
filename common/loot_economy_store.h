#pragma once
#include "loot_economy.h"
#include "safe_save.h"
#include <fstream>

namespace destinia {

inline fs::path lootEconomyFile(const fs::path& directory) {
  return directory / "economy-v1.txt";
}

inline void validateLootEconomyFile(const fs::path& directory) {
  const auto file = lootEconomyFile(directory);
  if (isLink(directory) || isLink(file) ||
      (fs::exists(file) && (!fs::is_regular_file(file) || fs::file_size(file) > 100000))) {
    throw std::runtime_error("SIDECAR INVALIDO - ORIGINAL PRESERVADO");
  }
}

inline void loadLootEconomy(const fs::path& directory, LootEconomyState& state) {
  validateLootEconomyFile(directory);
  const auto file = lootEconomyFile(directory);
  if (!fs::exists(file)) return;
  std::ifstream in(file, std::ios::binary);
  if (!in) throw std::runtime_error("FALHA LENDO ECONOMIA");
  const std::string text{std::istreambuf_iterator<char>(in), {}};
  if (in.bad()) throw std::runtime_error("FALHA LENDO ECONOMIA");
  state.decode(text);
}

inline void saveLootEconomy(
    const fs::path& directory,
    const LootEconomyState& state,
    const AtomicSaveOps& ops = AtomicSaveOps{}) {
  validateLootEconomyFile(directory);
  fs::create_directories(directory);
  const auto file = lootEconomyFile(directory);
  const fs::path temp = file.string() + "." + uniqueId() + ".tmp";
  try {
    const auto text = state.encode();
    FILE* out = std::fopen(temp.string().c_str(), "wb");
    if (!out) throw std::runtime_error("FALHA CRIANDO SIDECAR");
    bool okay = std::fwrite(text.data(), 1, text.size(), out) == text.size() && std::fflush(out) == 0;
#ifdef _WIN32
    if (okay) okay = ::_commit(::_fileno(out)) == 0;
#else
    if (okay) okay = ::fsync(::fileno(out)) == 0;
#endif
    if (std::fclose(out) != 0) okay = false;
    if (!okay) throw std::runtime_error("FALHA GRAVANDO ECONOMIA");
    validateLootEconomyFile(directory);
    ops.replace(temp, file);
  } catch (...) {
    std::error_code ec;
    fs::remove(temp, ec);
    throw;
  }
}

} // namespace destinia
