#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include "overlay_font.h"

namespace destinia {

struct MageSelectionHook {
  bool active = false;             // Detectou que está na tela de seleção
  bool mageSelected = false;       // Mago (4ª classe) em foco
  int classIndex = 0;              // 0=Guerreiro, 1=Atirador, 2=Ladino, 3=Mago
  int frameSeen = 0;
  
  // Cores RGB565 do jogo original
  const uint16_t COLOR_GOLD = 0xFDE0;     // Amarelo dourado do título
  const uint16_t COLOR_WHITE = 0xFFFF;    // Branco do texto
  const uint16_t COLOR_ORANGE = 0xFD20;   // Laranja das setas
  const uint16_t COLOR_BG = 0x18C3;       // Fundo da caixa de texto

  void notifyDrawMakeMenu() {
    active = true;
    frameSeen = 5; // mantém ativo por alguns frames
  }

  void update() {
    if (frameSeen > 0) {
      frameSeen--;
      if (frameSeen == 0) active = false;
    }
  }

  bool handleKey(int key, bool down) {
    if (!active || !down) return false;
    
    // Tecla ESQUERDA (5 ou seta esquerda)
    if (key == 5 || key == 10004) {
      classIndex = (classIndex + 3) % 4;
      mageSelected = (classIndex == 3);
      return mageSelected; // intercepta se caiu no Mago
    }
    // Tecla DIREITA (7 ou seta direita)
    if (key == 7 || key == 10006) {
      classIndex = (classIndex + 1) % 4;
      mageSelected = (classIndex == 3);
      return mageSelected; // intercepta se caiu no Mago
    }
    return false;
  }

  bool handleTouch(int x, int y, int phase, int sw, int sh) {
    if (!active || phase != 0) return false;
    
    // Coordenadas relativas da seta esquerda e direita no menu de seleção
    // Na resolução 400x240, o centro é (200, 120)
    int cx = sw / 2;
    int cy = sh / 2;

    // Seta esquerda (aprox x: cx - 45 .. cx - 15, y: cy - 10 .. cy + 20)
    if (x >= cx - 60 && x <= cx - 15 && y >= cy - 20 && y <= cy + 30) {
      return handleKey(5, true);
    }
    // Seta direita (aprox x: cx + 15 .. cx + 60, y: cy - 10 .. cy + 20)
    if (x >= cx + 15 && x <= cx + 60 && y >= cy - 20 && y <= cy + 30) {
      return handleKey(7, true);
    }
    return false;
  }

  // Renderiza a interface do Mago no frame buffer (400x240 ou 800x480)
  void paint(uint8_t* fb, int sw, int sh) {
    if (!active || !mageSelected || !fb) return;

    int cx = sw / 2;
    int cy = sh / 2;
    int scale = (sw >= 800) ? 2 : 1;

    // 1. Limpar / Cobrir a caixa de descrição original inferior
    // Caixa inferior fica em Y: cy + 28*scale até cy + 62*scale, X: cx - 60*scale até cx + 60*scale
    int boxX = cx - 58 * scale;
    int boxY = cy + 26 * scale;
    int boxW = 116 * scale;
    int boxH = 34 * scale;

    uint16_t* pixels = reinterpret_cast<uint16_t*>(fb);
    for (int y = boxY; y < boxY + boxH && y < sh; ++y) {
      for (int x = boxX; x < boxX + boxW && x < sw; ++x) {
        if (x >= 0 && y >= 0) {
          pixels[y * sw + x] = 0x18C3; // Fundo escuro do painel
        }
      }
    }

    // 2. Limpar a área do sprite central para desenhar o Mago
    int spriteBoxX = cx - 20 * scale;
    int spriteBoxY = cy - 25 * scale;
    int spriteBoxW = 40 * scale;
    int spriteBoxH = 45 * scale;
    for (int y = spriteBoxY; y < spriteBoxY + spriteBoxH && y < sh; ++y) {
      for (int x = spriteBoxX; x < spriteBoxX + spriteBoxW && x < sw; ++x) {
        if (x >= 0 && y >= 0) {
          pixels[y * sw + x] = 0x0000; // Fundo transparente/preto do círculo
        }
      }
    }

    // 3. Desenhar silhueta / ícone do Mago
    // Desenhamos um cajado com gema brilhante no centro
    int mx = cx;
    int my = cy - 5 * scale;
    for (int i = -12 * scale; i <= 15 * scale; ++i) {
      int px = mx + (i / 4);
      int py = my + i;
      if (px >= 0 && px < sw && py >= 0 && py < sh) {
        pixels[py * sw + px] = 0x8C51; // Madeira do cajado
        pixels[py * sw + px + 1] = 0xDEDB; // Brilho
      }
    }
    // Orbe mágica no topo do cajado
    int orbY = my - 13 * scale;
    for (int dy = -4 * scale; dy <= 4 * scale; ++dy) {
      for (int dx = -4 * scale; dx <= 4 * scale; ++dx) {
        if (dx*dx + dy*dy <= 16 * scale * scale) {
          int px = mx - 3 * scale + dx;
          int py = orbY + dy;
          if (px >= 0 && px < sw && py >= 0 && py < sh) {
            pixels[py * sw + px] = (dx*dx + dy*dy <= 4 * scale * scale) ? 0xFFFF : 0x07FF; // Ciano / Azul arcano
          }
        }
      }
    }

    // 4. Desenhar os textos canônicos do Mago
    // Título: LV.30 Mago (Dourado)
    paintOverlayText(fb, sw, sh, boxX + 4 * scale, boxY + 3 * scale, "LV.30 Mago", COLOR_GOLD, scale);
    
    // Descrição:
    paintOverlayText(fb, sw, sh, boxX + 4 * scale, boxY + 13 * scale, "Possui alto dano magico", COLOR_WHITE, scale);
    paintOverlayText(fb, sw, sh, boxX + 4 * scale, boxY + 22 * scale, "em area e magias arcanas.", COLOR_WHITE, scale);
  }
};

} // namespace destinia