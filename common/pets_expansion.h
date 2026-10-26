#pragma once
#include <string>
#include <vector>
#include <array>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include "safe_save.h"
#include "overlay_font.h"

namespace destinia {

struct PetData {
  int id;
  const char* namePt;
  const char* nameEn;
  const char* descPt;
  const char* descEn;
  int level;
  uint32_t xp;
  bool unlocked;
  uint16_t primaryColor;
  uint16_t secondaryColor;
};

class PetsManager {
public:
  int activePetId = 0; // -1 = nenhum, 0..3
  std::array<PetData, 4> pets = {{
    {0, "Lobo Glacial", "Frost Wolf", "+10% Velocidade | +15% Critico", "+10% Move Speed | +15% Crit Rate", 1, 0, true, 0x6e7f, 0xffff},
    {1, "Fenix Ignea", "Fire Phoenix", "+20% Dano de Ataque | +10% Regen PV", "+20% ATK Power | +10% HP Regen", 1, 0, true, 0xf920, 0xfd00},
    {2, "Golem de Pedra", "Stone Golem", "+25% Defesa Fisica | +15% Vida Max", "+25% Physical DEF | +15% Max HP", 1, 0, true, 0x8410, 0x5aeb},
    {3, "Mini-Diablo", "Mini-Diablo", "+15% Dano Geral | +5% Roubo de Vida", "+15% All Damage | +5% Life Steal", 1, 0, true, 0xb800, 0x2800}
  }};

  void load(const fs::path& saveDir) {
    fs::path p = saveDir / "pets-v1.txt";
    if (!fs::exists(p)) return;
    std::ifstream in(p);
    if (!in) return;
    std::string tag;
    if (!(in >> tag) || tag != "DESTINIA_PETS_V1") return;
    std::string key;
    while (in >> key) {
      if (key == "active_pet") {
        in >> activePetId;
      } else if (key == "pet") {
        int id, lvl, unl;
        uint32_t x;
        std::string sLvl, sXp, sUnl;
        if (in >> id >> sLvl >> lvl >> sXp >> x >> sUnl >> unl) {
          if (id >= 0 && id < 4) {
            pets[id].level = std::clamp(lvl, 1, 50);
            pets[id].xp = x;
            pets[id].unlocked = (unl != 0);
          }
        }
      } else if (key == "end") {
        break;
      }
    }
  }

  void save(const fs::path& saveDir) const {
    try {
      fs::create_directories(saveDir);
      fs::path p = saveDir / "pets-v1.txt";
      fs::path temp = saveDir / "pets-v1.tmp";
      std::ofstream out(temp);
      if (!out) return;
      out << "DESTINIA_PETS_V1\n";
      out << "active_pet " << activePetId << "\n";
      for (int i = 0; i < 4; ++i) {
        const auto& pt = pets[i];
        out << "pet " << pt.id << " level " << pt.level << " xp " << pt.xp << " unlocked " << (pt.unlocked ? 1 : 0) << "\n";
      }
      out << "end\n";
      out.close();
      fs::rename(temp, p);
    } catch (...) {}
  }

  void selectPet(int id) {
    if (id >= -1 && id < 4) {
      activePetId = id;
    }
  }

  void addExpToActive(uint32_t amount, const fs::path& saveDir) {
    if (activePetId < 0 || activePetId >= 4) return;
    auto& pt = pets[activePetId];
    if (pt.level >= 50) return;
    pt.xp += amount;
    uint32_t needed = uint32_t(pt.level * pt.level * 100);
    while (pt.xp >= needed && pt.level < 50) {
      pt.xp -= needed;
      pt.level++;
      needed = uint32_t(pt.level * pt.level * 100);
    }
    save(saveDir);
  }

  // Desenha o mascote ao lado do heroi no mapa (coordenadas de tela)
  void paintCompanion(uint8_t* pixels, int width, int height, int screenHeroX, int screenHeroY, uint64_t frame) const {
    if (activePetId < 0 || activePetId >= 4 || !pixels) return;
    const auto& pt = pets[activePetId];

    // Animacao de flutuacao senoidal suave
    int bob = int(std::sin(double(frame % 60) * 6.2831853 / 60.0) * 3.0);
    // Mascote fica a esquerda e acima do heroi
    int px = screenHeroX - 22;
    int py = screenHeroY - 26 + bob;

    // Sprite 12x12 do mascote
    static const uint8_t wolfSprite[12][12] = {
      {0,0,1,0,0,0,0,1,0,0,0,0},
      {0,1,2,1,0,0,1,2,1,0,0,0},
      {0,1,2,2,1,1,2,2,1,0,0,0},
      {1,2,2,2,2,2,2,2,2,1,0,0},
      {1,2,3,2,2,2,2,3,2,1,0,0},
      {1,2,2,2,1,1,2,2,2,1,0,0},
      {0,1,2,2,2,2,2,2,1,0,0,0},
      {0,0,1,2,2,2,2,1,0,0,0,0},
      {0,1,2,2,2,2,2,2,1,0,1,0},
      {0,1,2,1,0,0,1,2,1,1,2,1},
      {0,1,1,0,0,0,0,1,1,0,1,0},
      {0,0,0,0,0,0,0,0,0,0,0,0}
    };

    uint16_t c1 = pt.primaryColor;
    uint16_t c2 = pt.secondaryColor;
    uint16_t cEye = 0x07e0; // olhos brilhantes esmeralda

    // Sombra do mascote no chao
    int shadowY = screenHeroY - 2;
    for (int sx = -5; sx <= 5; ++sx) {
      int tx = px + 6 + sx;
      if (tx >= 0 && tx < width && shadowY >= 0 && shadowY < height) {
        size_t at = (size_t(shadowY) * width + tx) * 2;
        uint16_t sc = pixels[at] | (pixels[at + 1] << 8);
        sc = (sc >> 1) & 0x7bef; // escurecer 50%
        pixels[at] = uint8_t(sc);
        pixels[at + 1] = uint8_t(sc >> 8);
      }
    }

    // Desenha corpo
    for (int y = 0; y < 12; ++y) {
      for (int x = 0; x < 12; ++x) {
        uint8_t val = wolfSprite[y][x];
        if (val == 0) continue;
        int tx = px + x, ty = py + y;
        if (tx < 0 || tx >= width || ty < 0 || ty >= height) continue;
        size_t at = (size_t(ty) * width + tx) * 2;
        uint16_t col = (val == 1) ? 0x0000 : (val == 2) ? c1 : (val == 3) ? cEye : c2;
        pixels[at] = uint8_t(col);
        pixels[at + 1] = uint8_t(col >> 8);
      }
    }

    // Nivel do mascote flutuante em cima (ex: Lv5)
    std::string lvlStr = "Lv" + std::to_string(pt.level);
    paintOverlayText(pixels, width, height, px - 2 + 1, py - 9 + 1, lvlStr, 0x0000);
    paintOverlayText(pixels, width, height, px - 2, py - 9, lvlStr, 0xd7bf);
  }
};

} // namespace destinia
