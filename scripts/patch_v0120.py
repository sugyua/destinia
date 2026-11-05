#!/usr/bin/env python3
from pathlib import Path

# 1. Update common/titles.h
p_titles = Path('common/titles.h')
text = p_titles.read_text(encoding='utf-8')

if 'SenhorDoAbismo = 7' not in text:
    text = text.replace(
        '  Complecionista = 6\n};',
        '  Complecionista = 6,\n  SenhorDoAbismo = 7\n};'
    ).replace(
        '  Complecionista = 6\r\n};',
        '  Complecionista = 6,\r\n  SenhorDoAbismo = 7\r\n};'
    )
    
    senhor_entry = '    { TitleId::SenhorDoAbismo, "Senhor do Abismo", "Derrotou Diablo no Modo Abissal. O ápice do poder absoluto.", 150, 150, 150, 150 }\n  };'
    senhor_entry_crlf = '    { TitleId::SenhorDoAbismo, "Senhor do Abismo", "Derrotou Diablo no Modo Abissal. O ápice do poder absoluto.", 150, 150, 150, 150 }\r\n  };'
    
    if '100, 100, 100, 100 }\r\n  };' in text:
        text = text.replace('100, 100, 100, 100 }\r\n  };', '100, 100, 100, 100 },\r\n' + senhor_entry_crlf)
    elif '100, 100, 100, 100 }\n  };' in text:
        text = text.replace('100, 100, 100, 100 }\n  };', '100, 100, 100, 100 },\n' + senhor_entry)

    methods = """
  bool load(const fs::path& dir) {
    fs::path p = dir / "titles-v1.txt";
    if (!fs::exists(p)) return false;
    std::ifstream in(p);
    if (!in.is_open()) return false;
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    return decode(content);
  }

  bool save(const fs::path& dir) const {
    fs::path p = dir / "titles-v1.txt";
    fs::path tmp = dir / "titles-v1.txt.tmp";
    std::ofstream out(tmp);
    if (!out.is_open()) return false;
    out << encode();
    out.close();
    std::error_code ec;
    fs::rename(tmp, p, ec);
    return !ec;
  }
};"""
    if '  bool decode(' in text:
        text = text.replace('    return true;\n  }\n};', '    return true;\n  }\n' + methods)
        text = text.replace('    return true;\r\n  }\r\n};', '    return true;\r\n  }\r\n' + methods.replace('\n', '\r\n'))

    p_titles.write_text(text, encoding='utf-8')
    print('common/titles.h patched!')

# 2. Update web/dist/titles.mjs
p_web_titles = Path('web/dist/titles.mjs')
web_text = p_web_titles.read_text(encoding='utf-8')
if 'SenhorDoAbismo: 7' not in web_text:
    web_text = web_text.replace(
        '  Complecionista: 6\n};',
        '  Complecionista: 6,\n  SenhorDoAbismo: 7\n};'
    ).replace(
        '  Complecionista: 6\r\n};',
        '  Complecionista: 6,\r\n  SenhorDoAbismo: 7\r\n};'
    )
    senhor_web = "  { id: TitleId.SenhorDoAbismo, nome: 'Senhor do Abismo', descricao: 'Derrotou Diablo no Modo Abissal. O ápice do poder absoluto.', atk: 150, def: 150, hp: 150, mp: 150 }\n];"
    senhor_web_crlf = "  { id: TitleId.SenhorDoAbismo, nome: 'Senhor do Abismo', descricao: 'Derrotou Diablo no Modo Abissal. O ápice do poder absoluto.', atk: 150, def: 150, hp: 150, mp: 150 }\r\n];"
    
    if 'hp: 100, mp: 100 }\r\n];' in web_text:
        web_text = web_text.replace('hp: 100, mp: 100 }\r\n];', 'hp: 100, mp: 100 },\r\n' + senhor_web_crlf)
    elif 'hp: 100, mp: 100 }\n];' in web_text:
        web_text = web_text.replace('hp: 100, mp: 100 }\n];', 'hp: 100, mp: 100 },\n' + senhor_web)

    p_web_titles.write_text(web_text, encoding='utf-8')
    print('web/dist/titles.mjs patched!')

# 3. Update tests/titles_native_cases.cpp
p_tests = Path('tests/titles_native_cases.cpp')
test_text = p_tests.read_text(encoding='utf-8')
if 'SenhorDoAbismo' not in test_text:
    extra_test = """
  // Teste 5: atributos e buffs do titulo Senhor do Abismo (0.12.0)
  const TitleDef* sa = findTitle(TitleId::SenhorDoAbismo);
  assert(sa != nullptr);
  assert(sa->atkBonusPct == 150);
  assert(sa->defBonusPct == 150);
  assert(sa->hpBonusPct == 150);
  assert(sa->mpBonusPct == 150);

  CombatStats abyssalBuffed = applyTitleBuffs(baseStats, TitleId::SenhorDoAbismo);
  assert(abyssalBuffed[5] == 12500); // 5000 + 150% = 12500
  assert(abyssalBuffed[6] == 5000);  // 2000 + 150% = 5000
  assert(abyssalBuffed[7] == 3750);  // 1500 + 150% = 3750
  assert(abyssalBuffed[8] == 2000);  // 800 + 150% = 2000

  std::cout << "TITLES_NATIVE_CASES_PASS\\n";
  return 0;
}"""
    test_text = test_text.replace('  std::cout << "TITLES_NATIVE_CASES_PASS\\n";\n  return 0;\n}', extra_test)
    test_text = test_text.replace('  std::cout << "TITLES_NATIVE_CASES_PASS\\n";\r\n  return 0;\r\n}', extra_test.replace('\n', '\r\n'))
    p_tests.write_text(test_text, encoding='utf-8')
    print('tests/titles_native_cases.cpp patched!')

# 4. Update common/difficulty_host.h
p_diff_host = Path('common/difficulty_host.h')
diff_host_text = p_diff_host.read_text(encoding='utf-8')
if '#include "titles.h"' not in diff_host_text:
    diff_host_text = diff_host_text.replace('#include "difficulty.h"', '#include "difficulty.h"\n#include "titles.h"')

old_boss_def = 'static void difficultyBossDefeated(unsigned slot,int species,u32 p){if(difficultyReady&&slot<10&&species==108&&difficultyDeaths.emplace(slot,p).second){const auto tier=difficultyArm.deathDifficulty(p);difficultyKills.emplace(slot,tier?*tier:difficultyProgress.current(slot));}}'
new_boss_def = """static void difficultyBossDefeated(unsigned slot,int species,u32 p){
 if(difficultyReady&&slot<10&&species==108&&difficultyDeaths.emplace(slot,p).second){
  const auto tier=difficultyArm.deathDifficulty(p);
  const unsigned deadTier=tier?*tier:difficultyProgress.current(slot);
  difficultyKills.emplace(slot,deadTier);
  if(deadTier>=4){
   try{
    destinia::TitlesStore ts;
    ts.load(fs::path(saveDir));
    ts.unlock(slot,destinia::TitleId::SenhorDoAbismo);
    ts.save(fs::path(saveDir));
   }catch(...){}
  }
 }
}"""

if old_boss_def in diff_host_text:
    diff_host_text = diff_host_text.replace(old_boss_def, new_boss_def)
    p_diff_host.write_text(diff_host_text, encoding='utf-8')
    print('common/difficulty_host.h patched!')

# 5. Update window titles to Destinia 0.12.0 Beta
for p_h in [Path('host/main.cpp'), Path('host_zoom/main.cpp')]:
    h_text = p_h.read_text(encoding='utf-8')
    h_text = h_text.replace('Destinia 0.10.3 Beta', 'Destinia 0.12.0 Beta')
    p_h.write_text(h_text, encoding='utf-8')
    print(f'{p_h} updated with Destinia 0.12.0 Beta window titles!')

# 6. Update web/tests/features-0110.test.mjs
p_feat = Path('web/tests/features-0110.test.mjs')
feat_text = p_feat.read_text(encoding='utf-8')
old_test_mago = """  ui.handleKey(7, true); // Aba 3: Santuário
  assert.equal(ui.currentTab, 3);
  assert.equal(ui.getItemsCount(), 3); // Reset Atributos, Reset Skills, NG+

  ui.handleKey(7, true); // Volta para Aba 0: Chefes
  assert.equal(ui.currentTab, 0);"""

new_test_mago = """  ui.handleKey(7, true); // Aba 3: Santuario
  assert.equal(ui.currentTab, 3);
  assert.equal(ui.getItemsCount(), 3); // Reset Atributos, Reset Skills, NG+

  ui.handleKey(7, true); // Aba 4: Titulos (0.12.0)
  assert.equal(ui.currentTab, 4);
  assert.equal(ui.getItemsCount(), 8); // 8 titulos

  ui.handleKey(7, true); // Volta para Aba 0: Chefes
  assert.equal(ui.currentTab, 0);"""

if 'Aba 4: Titulos' not in feat_text:
    feat_text = feat_text.replace(old_test_mago, new_test_mago)
    feat_text = feat_text.replace(old_test_mago.replace('\n', '\r\n'), new_test_mago.replace('\n', '\r\n'))
    p_feat.write_text(feat_text, encoding='utf-8')
    print('web/tests/features-0110.test.mjs patched!')

# 7. Update CHANGELOG.md
p_cl = Path('CHANGELOG.md')
cl_text = p_cl.read_text(encoding='utf-8')

entry_0120 = """## 0.12.0 Beta — 26/10/2026

- **Modo Abissal Hardcore Completo**:
  - **100% Aura de Elite:** Todos os monstros do mapa nascem compulsoriamente como monstros Elites em `common/difficulty_arm.h`, garantindo o desafio supremo para heróis no Nível 999.
  - **Escalonamento Brutal:** Multiplicadores nativos de 6.0x de Vida e 6.0x de Dano base em relação ao modo normal.
  - **Zero Bônus de Drop:** Tabela de recompensas e raridades preservada idêntica à da campanha normal sem facilitação, servindo puramente como teste definitivo de maestria.
- **Título Lendário "Senhor do Abismo"**:
  - Título honorífico exclusivo concedido ao derrotar Diablo (`species 108`) no Modo Abissal.
  - Concede **+150% em todos os atributos principais** (ATK, DEF, PV, PM).
  - Persistência isolada no sidecar `titles-v1.txt` (PC) e `localStorage` (Web).
- **Aba de Títulos no Mago dos Bosses (5ª Aba: [TITULOS])**:
  - O Mago dos Bosses agora conta com 5 abas ([CHEFES], [TORRE], [MASCOTES], [SANTUARIO], [TITULOS]).
  - Lista de 8 títulos com descrição de requisitos, badges [EQUIPADO], [LIBERADO] ou [BLOQUEADO], e acionamento por botão/toque para equipar.

"""

if '## 0.12.0 Beta' not in cl_text:
    cl_text = cl_text.replace('## 0.11.0 Beta', entry_0120 + '## 0.11.0 Beta')
    p_cl.write_text(cl_text, encoding='utf-8')
    print('CHANGELOG.md updated with 0.12.0!')

# 8. Update README.md
p_rm = Path('README.md')
rm_text = p_rm.read_text(encoding='utf-8')

entry_rm_0120 = """## Novidades da Versão 0.12.0 — Modo Abissal Supremo e Título Senhor do Abismo

- **Modo Abissal Hardcore Completo**:
  - **100% Aura de Elite:** Todos os monstros do mapa passam a nascer compulsoriamente como elites (`monster+0x4c = 1`), transformando toda a campanha no desafio definitivo.
  - **Escalonamento Brutal:** 6.0x de Vida e 6.0x de Dano base em relação ao modo normal.
  - **Zero Bônus de Drop:** Tabela de recompensas e raridades preservadas nos valores idênticos aos da campanha normal, servindo puramente como teste definitivo de teoria de builds e reflexos.
- **Título Lendário "Senhor do Abismo"**:
  - Desbloqueado ao derrotar Diablo (`species 108`) no Modo Abissal.
  - Concede **+150% em todos os status principais** (ATK, DEF, PV, PM).
  - Persistência isolada no sidecar `titles-v1.txt` (PC) e `localStorage` (Web).
- **Aba de Títulos no Mago dos Bosses (5ª Aba: [TITULOS])**:
  - 5 abas integradas ([CHEFES], [TORRE], [MASCOTES], [SANTUARIO], [TITULOS]).
  - Gerenciamento completo de títulos desbloqueados com aplicação imediata dos buffs no herói.

"""

if '## Novidades da Versão 0.12.0' not in rm_text:
    rm_text = rm_text.replace('## Novidades da Versão 0.11.0', entry_rm_0120 + '## Novidades da Versão 0.11.0')
    p_rm.write_text(rm_text, encoding='utf-8')
    print('README.md updated with 0.12.0!')
