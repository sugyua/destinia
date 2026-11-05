#pragma once
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace destinia { namespace refinement_policy {
// Policy shared by native adapters. Equipment/save bytes are never changed here.
inline constexpr int MaxLevel = 15;
enum class Stone { Common = 0, Pure = 1, Abyssal = 2 };
inline void requireLevel(int level) {
  if (level < 0 || level > MaxLevel) throw std::invalid_argument("invalid refinement level");
}
inline int checkpoint(int level) {
  requireLevel(level);
  return level >= 10 ? 10 : level >= 5 ? 5 : 0;
}
inline Stone stoneForTarget(int target) {
  requireLevel(target);
  if (!target) throw std::invalid_argument("refinement target must be positive");
  return target <= 5 ? Stone::Common : target <= 10 ? Stone::Pure : Stone::Abyssal;
}
inline const char* stoneName(Stone stone) {
  switch (stone) {
    case Stone::Common: return "Pedra de Refino Comum";
    case Stone::Pure: return "Pedra de Refino Pura";
    case Stone::Abyssal: return "Pedra de Refino Abissal";
  }
  throw std::invalid_argument("invalid refinement stone");
}
inline int64_t scale(int64_t base, int level, int percent) {
  requireLevel(level);
  if (base < 0 || percent < 0 || percent > 100) throw std::invalid_argument("invalid refinement stat");
  const int64_t rate = int64_t(level) * percent;
  // Divide first: even a valid int64 base must not overflow an intermediate.
  if (rate && base / 100 > std::numeric_limits<int64_t>::max() / rate)
    throw std::overflow_error("refinement stat overflow");
  const int64_t whole = (base / 100) * rate, tail = (base % 100) * rate / 100;
  if (whole > std::numeric_limits<int64_t>::max() - tail ||
      base > std::numeric_limits<int64_t>::max() - whole - tail)
    throw std::overflow_error("refinement stat overflow");
  return base + whole + tail;
}
inline int64_t weaponAttack(int64_t base, int level) { return scale(base, level, 8); }
inline int64_t armorDefense(int64_t base, int level) { return scale(base, level, 6); }
inline int64_t armorHealth(int64_t base, int level) { return scale(base, level, 4); }
struct Quote { int target; Stone stone; int stones; int64_t gold; int successPercent; };
inline Quote quote(int current) {
  requireLevel(current);
  if (current == MaxLevel) throw std::invalid_argument("maximum refinement reached");
  const int target = current + 1;
  // Guaranteed success is deliberate: no equipment destruction or checkpoint loss.
  return {target, stoneForTarget(target), target, int64_t(500) * target * target, 100};
}
}}
