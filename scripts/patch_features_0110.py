import re

def patch_file(filepath):
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    # 1. auto_potion header
    if 'auto_potion.h' not in content:
        content = content.replace(
            '#include "../common/boss_rush_ui.h"\nstatic destinia::BossRushUI bossRushUI;',
            '#include "../common/boss_rush_ui.h"\nstatic destinia::BossRushUI bossRushUI;\n#include "../common/auto_potion.h"\nstatic destinia::AutoPotionController autoPotion;'
        )

    # 2. bossRushUI callbacks
    target_callbacks = '    bossRushUI.teleport = [&](int a, int m, int e) { callFn(exportsMap["Map_loadData"], {uint32_t(a), uint32_t(m), uint32_t(e)}); };'
    replacement_callbacks = target_callbacks + '\n    bossRushUI.calcHeroStat = [&]() -> bool { return callFn(exportsMap["GameUI_calcHeroStat"], {}); };\n    bossRushUI.rd16 = [](u32 a) { return rd16(a); };\n    bossRushUI.wr16 = [](u32 a, uint16_t v) { wr16(a, v); };\n    bossRushUI.gameBase = exportsMap["Game"];\n    bossRushUI.saveDir = saveDir;'
    if 'bossRushUI.calcHeroStat' not in content:
        content = content.replace(target_callbacks, replacement_callbacks)

    # 3. Key toggle for Auto-Potion (H key)
    target_key = 'if(ev.key.keysym.sym==SDLK_q) { if(down) questTrackerUI.toggle(); continue; }'
    replacement_key = target_key + '\n          if(ev.key.keysym.sym==SDLK_h) { if(down) autoPotion.toggle(); continue; }'
    if 'SDLK_h' not in content:
        content = content.replace(target_key, replacement_key)

    # 4. Main loop update & companion render
    target_loop = 'bossMage.paint(frameBuffer.data());'
    replacement_loop = '''bossRushUI.currentSlot = rd16(exportsMap.at("Game")+0x4886);
    autoPotion.update(exportsMap.at("Game"), [](u32 a){return rd16(a);}, [](u32 a, uint16_t v){wr16(a, v);}, playSoundId);
    if(int16_t(rd16(exportsMap.at("Game")+0x4854))==0) {
      int hX = int16_t(rd16(exportsMap.at("Game")+0x4890));
      int hY = int16_t(rd16(exportsMap.at("Game")+0x4892));
      int cX = int16_t(rd16(exportsMap.at("Game")+0x4878));
      int cY = int16_t(rd16(exportsMap.at("Game")+0x487a));
      bossRushUI.pets.paintCompanion(frameBuffer.data(), SCREEN_W, SCREEN_H, hX - cX, hY - cY, frame);
    }
    bossMage.paint(frameBuffer.data());'''
    if 'autoPotion.update' not in content:
        content = content.replace(target_loop, replacement_loop)

    # 5. autoPotion notice render
    target_notice = 'questTrackerUI.paint(frameBuffer.data(), SCREEN_W, SCREEN_H);'
    replacement_notice = 'questTrackerUI.paint(frameBuffer.data(), SCREEN_W, SCREEN_H);\n      autoPotion.paintNotice(frameBuffer.data(), SCREEN_W, SCREEN_H);'
    if 'autoPotion.paintNotice' not in content:
        content = content.replace(target_notice, replacement_notice)

    with open(filepath, 'w', encoding='utf-8', newline='') as f:
        f.write(content)
    print(f'Patched {filepath} successfully.')

patch_file('host/main.cpp')
patch_file('host_zoom/main.cpp')
