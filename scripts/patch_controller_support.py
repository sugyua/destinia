import re

code_guide = '''
static bool controllerBackHeld = false;
static bool controllerChordTriggered = false;
static uint32_t lastAxisTick = 0;

static void paintControllerGuide(uint8_t* pixels, int width, int height) {
  if (!pixels) return;
  const std::string guide = "[SELECT]+  [X] Catalogo   [Y] Bau   [B] Bounties   [A] Missao   [R3] Auto-Pocao";
  int tw = destinia::overlayTextWidth(guide);
  int x = (width - tw) / 2;
  int y = height - 24;
  for (int dy = -3; dy < 13; ++dy) {
    for (int dx = -8; dx < tw + 8; ++dx) {
      int px = x + dx, py = y + dy;
      if (px >= 0 && px < width && py >= 0 && py < height) {
        size_t at = (size_t(py) * width + px) * 2;
        bool border = (dy == -3 || dy == 12 || dx == -8 || dx == tw + 7);
        if (border) {
          pixels[at] = 0xbf; pixels[at + 1] = 0xd7;
        } else {
          pixels[at] = 0x00; pixels[at + 1] = 0x10;
        }
      }
    }
  }
  destinia::paintOverlayText(pixels, width, height, x + 1, y + 1, guide, 0x0000);
  destinia::paintOverlayText(pixels, width, height, x, y, guide, 0xd7bf);
}
'''

def patch_file(filepath):
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    # Add globals if not present
    if 'controllerBackHeld' not in content:
        content = content.replace(
            'static destinia::AutoPotionController autoPotion;',
            'static destinia::AutoPotionController autoPotion;\n' + code_guide
        )

    # In event loop, replace bossRushUI.open controller check and add modal routing
    old_boss_check = '''        if(bossRushUI.open) {
          if(ev.type==SDL_CONTROLLERBUTTONDOWN && ev.cbutton.which==controls.controllerId()) {
            auto binding=preferences.buttons.find((SDL_GameControllerButton)ev.cbutton.button);
            if(binding!=preferences.buttons.end()) {const int k[]={3,9,5,7,17,10,27};int a=int(binding->second);if(a>=0 && a<7)bossRushUI.handleKey(k[a],true);}
          }
          if((ev.type==SDL_KEYDOWN) && !ev.key.repeat) {
            if(ev.key.keysym.sym==SDLK_ESCAPE)bossRushUI.handleKey(10,true);
            else if(ev.key.keysym.sym==SDLK_UP)bossRushUI.handleKey(3,true);
            else if(ev.key.keysym.sym==SDLK_DOWN)bossRushUI.handleKey(9,true);
            else if(ev.key.keysym.sym==SDLK_RETURN || ev.key.keysym.sym==SDLK_SPACE)bossRushUI.handleKey(17,true);
            else {auto binding=preferences.keys.find(ev.key.keysym.sym);if(binding!=preferences.keys.end()) {const int k[]={3,9,5,7,17,10,27};int a=int(binding->second);if(a>=0 && a<7)bossRushUI.handleKey(k[a],true);}}
          }
          continue;
        }'''

    new_boss_and_modals = '''        // Roteamento completo de controle USB para menus modais
        if(ev.type==SDL_CONTROLLERBUTTONDOWN && ev.cbutton.which==controls.controllerId()) {
          uint8_t btn = ev.cbutton.button;

          // 1. Menu Mago dos Bosses (Boss Rush / Torre / Mascotes / Santuario)
          if(bossRushUI.open) {
            if(btn==SDL_CONTROLLER_BUTTON_B || btn==SDL_CONTROLLER_BUTTON_BACK) bossRushUI.handleKey(10, true);
            else if(btn==SDL_CONTROLLER_BUTTON_A) bossRushUI.handleKey(17, true);
            else if(btn==SDL_CONTROLLER_BUTTON_DPAD_UP) bossRushUI.handleKey(3, true);
            else if(btn==SDL_CONTROLLER_BUTTON_DPAD_DOWN) bossRushUI.handleKey(9, true);
            else if(btn==SDL_CONTROLLER_BUTTON_DPAD_LEFT || btn==SDL_CONTROLLER_BUTTON_LEFTSHOULDER) bossRushUI.handleKey(5, true);
            else if(btn==SDL_CONTROLLER_BUTTON_DPAD_RIGHT || btn==SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) bossRushUI.handleKey(7, true);
            continue;
          }

          // 2. Catalogo de Equipamentos (F4)
          if(weaponsCatalogHost.open) {
            if(btn==SDL_CONTROLLER_BUTTON_B || btn==SDL_CONTROLLER_BUTTON_BACK) weaponsCatalogHost.open = false;
            else if(btn==SDL_CONTROLLER_BUTTON_DPAD_LEFT || btn==SDL_CONTROLLER_BUTTON_LEFTSHOULDER) weaponsCatalogHost.handleKey(3, true);
            else if(btn==SDL_CONTROLLER_BUTTON_DPAD_RIGHT || btn==SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) weaponsCatalogHost.handleKey(9, true);
            else if(btn==SDL_CONTROLLER_BUTTON_DPAD_UP || btn==SDL_CONTROLLER_BUTTON_DPAD_DOWN || btn==SDL_CONTROLLER_BUTTON_Y) weaponsCatalogHost.handleKey('\\t', true);
            else if(btn==SDL_CONTROLLER_BUTTON_X || btn==SDL_CONTROLLER_BUTTON_A) weaponsCatalogHost.handleKey('t', true);
            continue;
          }

          // 3. Bau Compartilhado (F5 / B)
          if(sharedStashHost.open) {
            if(btn==SDL_CONTROLLER_BUTTON_B || btn==SDL_CONTROLLER_BUTTON_BACK) sharedStashHost.open = false;
            else if(btn==SDL_CONTROLLER_BUTTON_DPAD_LEFT) sharedStashHost.handleKey(3, true, exportsMap.at("Game"), [](u32 a){return rd16(a);}, [](u32 a, uint16_t v){wr16(a, v);});
            else if(btn==SDL_CONTROLLER_BUTTON_DPAD_RIGHT) sharedStashHost.handleKey(9, true, exportsMap.at("Game"), [](u32 a){return rd16(a);}, [](u32 a, uint16_t v){wr16(a, v);});
            else if(btn==SDL_CONTROLLER_BUTTON_DPAD_UP) sharedStashHost.handleKey(5, true, exportsMap.at("Game"), [](u32 a){return rd16(a);}, [](u32 a, uint16_t v){wr16(a, v);});
            else if(btn==SDL_CONTROLLER_BUTTON_DPAD_DOWN) sharedStashHost.handleKey(7, true, exportsMap.at("Game"), [](u32 a){return rd16(a);}, [](u32 a, uint16_t v){wr16(a, v);});
            else if(btn==SDL_CONTROLLER_BUTTON_X || btn==SDL_CONTROLLER_BUTTON_Y || btn==SDL_CONTROLLER_BUTTON_LEFTSHOULDER || btn==SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) sharedStashHost.handleKey('\\t', true, exportsMap.at("Game"), [](u32 a){return rd16(a);}, [](u32 a, uint16_t v){wr16(a, v);});
            else if(btn==SDL_CONTROLLER_BUTTON_A) sharedStashHost.handleKey(13, true, exportsMap.at("Game"), [](u32 a){return rd16(a);}, [](u32 a, uint16_t v){wr16(a, v);});
            continue;
          }

          // 4. Mural de Contratos / Bounties (F8 / C)
          if(bountiesHost.open) {
            if(btn==SDL_CONTROLLER_BUTTON_B || btn==SDL_CONTROLLER_BUTTON_BACK) bountiesHost.open = false;
            else if(btn==SDL_CONTROLLER_BUTTON_DPAD_UP) bountiesHost.handleKey(5, true, rd16(exportsMap.at("Game")+0x4886), [&](uint32_t gold, uint16_t cash){ if(fnGetCash && fnSetCash) { u32 curCash = 0; callFn(fnGetCash, {}, &curCash); callFn(fnSetCash, {curCash + cash}); } u32 curGold = rd32(exportsMap.at("Game")+0x4894); wr32(exportsMap.at("Game")+0x4894, curGold + gold); });
            else if(btn==SDL_CONTROLLER_BUTTON_DPAD_DOWN) bountiesHost.handleKey(7, true, rd16(exportsMap.at("Game")+0x4886), [&](uint32_t gold, uint16_t cash){ if(fnGetCash && fnSetCash) { u32 curCash = 0; callFn(fnGetCash, {}, &curCash); callFn(fnSetCash, {curCash + cash}); } u32 curGold = rd32(exportsMap.at("Game")+0x4894); wr32(exportsMap.at("Game")+0x4894, curGold + gold); });
            else if(btn==SDL_CONTROLLER_BUTTON_A) bountiesHost.handleKey(13, true, rd16(exportsMap.at("Game")+0x4886), [&](uint32_t gold, uint16_t cash){ if(fnGetCash && fnSetCash) { u32 curCash = 0; callFn(fnGetCash, {}, &curCash); callFn(fnSetCash, {curCash + cash}); } u32 curGold = rd32(exportsMap.at("Game")+0x4894); wr32(exportsMap.at("Game")+0x4894, curGold + gold); });
            continue;
          }

          // Atalhos Diretos de Controle em Gameplay:
          if(btn==SDL_CONTROLLER_BUTTON_RIGHTSTICK) { autoPotion.toggle(); continue; }
          if(btn==SDL_CONTROLLER_BUTTON_LEFTSTICK) { questTrackerUI.toggle(); continue; }

          // Modificador [SELECT / BACK]:
          if(btn==SDL_CONTROLLER_BUTTON_BACK) {
            controllerBackHeld = true;
            controllerChordTriggered = false;
            continue;
          }

          if(controllerBackHeld) {
            if(btn==SDL_CONTROLLER_BUTTON_X) {
              controllerChordTriggered = true;
              weaponsCatalogHost.toggle(rd16(exportsMap.at("Game")+0x4886), exportsMap.at("Game"), [](u32 a){return rd16(a);});
              continue;
            }
            if(btn==SDL_CONTROLLER_BUTTON_Y) {
              controllerChordTriggered = true;
              sharedStashHost.toggle();
              continue;
            }
            if(btn==SDL_CONTROLLER_BUTTON_B) {
              controllerChordTriggered = true;
              bountiesHost.toggle();
              continue;
            }
            if(btn==SDL_CONTROLLER_BUTTON_A) {
              controllerChordTriggered = true;
              questTrackerUI.toggle();
              continue;
            }
            if(btn==SDL_CONTROLLER_BUTTON_DPAD_UP) {
              controllerChordTriggered = true;
              bossRushUI.openMenu();
              continue;
            }
            if(btn==SDL_CONTROLLER_BUTTON_DPAD_DOWN) {
              controllerChordTriggered = true;
              autoPotion.toggle();
              continue;
            }
          }
        }

        // Soltura de botoes do controle
        if(ev.type==SDL_CONTROLLERBUTTONUP && ev.cbutton.which==controls.controllerId()) {
          if(ev.cbutton.button==SDL_CONTROLLER_BUTTON_BACK) {
            controllerBackHeld = false;
            if(!controllerChordTriggered) {
              touch(0, 0, 3);
            }
            controllerChordTriggered = false;
            continue;
          }
        }

        if(bossRushUI.open) {
          if((ev.type==SDL_KEYDOWN) && !ev.key.repeat) {
            if(ev.key.keysym.sym==SDLK_ESCAPE)bossRushUI.handleKey(10,true);
            else if(ev.key.keysym.sym==SDLK_UP)bossRushUI.handleKey(3,true);
            else if(ev.key.keysym.sym==SDLK_DOWN)bossRushUI.handleKey(9,true);
            else if(ev.key.keysym.sym==SDLK_RETURN || ev.key.keysym.sym==SDLK_SPACE)bossRushUI.handleKey(17,true);
            else {auto binding=preferences.keys.find(ev.key.keysym.sym);if(binding!=preferences.keys.end()) {const int k[]={3,9,5,7,17,10,27};int a=int(binding->second);if(a>=0 && a<7)bossRushUI.handleKey(k[a],true);}}
          }
          continue;
        }'''

    content = content.replace(old_boss_check, new_boss_and_modals)

    # Add guide overlay paint if controllerBackHeld
    target_paint = 'autoPotion.paintNotice(frameBuffer.data(), SCREEN_W, SCREEN_H);'
    replacement_paint = target_paint + '\n      if (controllerBackHeld) { paintControllerGuide(frameBuffer.data(), SCREEN_W, SCREEN_H); }'
    if 'paintControllerGuide(frameBuffer.data()' not in content:
        content = content.replace(target_paint, replacement_paint)

    with open(filepath, 'w', encoding='utf-8', newline='') as f:
        f.write(content)
    print(f'Controller support patched into {filepath}')

patch_file('host/main.cpp')
patch_file('host_zoom/main.cpp')
