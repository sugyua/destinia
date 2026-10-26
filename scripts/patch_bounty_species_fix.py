#!/usr/bin/env python3
from pathlib import Path

# 1. Update common/bounties.h
p_h = Path('common/bounties.h')
text_h = p_h.read_text(encoding='utf-8')

text_h = text_h.replace('c1.targetId = 2; // Imperial Soldier', 'c1.targetId = 61; // Imperial Soldier (species 61 no mon_data_en.gdt)')

old_kill_h = """  void onMonsterKilled(unsigned slot, uint16_t species) {
    if (slot >= 10) return;
    auto& list = getContracts(slot);
    for (auto& c : list) {
      if (!c.completed && c.targetId == species) {
        c.currentCount++;"""

new_kill_h = """  void onMonsterKilled(unsigned slot, uint16_t species) {
    if (slot >= 10) return;
    auto& list = getContracts(slot);
    for (auto& c : list) {
      bool matches = (!c.completed) && (
        (c.targetId == species) ||
        (c.id == 1 && (species == 61 || species == 62 || species == 63 || species == 2))
      );
      if (matches) {
        c.currentCount++;"""

text_h = text_h.replace(old_kill_h, new_kill_h)
text_h = text_h.replace(old_kill_h.replace('\n', '\r\n'), new_kill_h.replace('\n', '\r\n'))

old_deser_h = """        if (c.id == 1) {
          c.title = "Caca: Soldados Imperiais";
          c.description = "Elimine 15 Soldados Imperiais nas estradas.";"""

new_deser_h = """        if (c.id == 1) {
          c.title = "Caca: Soldados Imperiais";
          c.description = "Elimine 15 Soldados Imperiais nas estradas.";
          if (c.targetId == 2) c.targetId = 61;"""

text_h = text_h.replace(old_deser_h, new_deser_h)
text_h = text_h.replace(old_deser_h.replace('\n', '\r\n'), new_deser_h.replace('\n', '\r\n'))

p_h.write_text(text_h, encoding='utf-8')
print('common/bounties.h patched with species 61 fix!')

# 2. Update web/dist/bounties.mjs
p_web = Path('web/dist/bounties.mjs')
text_web = p_web.read_text(encoding='utf-8')

text_web = text_web.replace('targetId: 2, // Imperial Soldier', 'targetId: 61, // Imperial Soldier')

old_kill_web = """  onMonsterKilled(slot = 0, species = 0) {
    const list = this.getContracts(slot);
    let changed = false;
    for (const c of list) {
      if (!c.completed && c.targetId === species) {
        c.currentCount++;"""

new_kill_web = """  onMonsterKilled(slot = 0, species = 0) {
    const list = this.getContracts(slot);
    let changed = false;
    for (const c of list) {
      const matches = (!c.completed) && (
        c.targetId === species ||
        (c.id === 1 && (species === 61 || species === 62 || species === 63 || species === 2))
      );
      if (matches) {
        c.currentCount++;"""

text_web = text_web.replace(old_kill_web, new_kill_web)
text_web = text_web.replace(old_kill_web.replace('\n', '\r\n'), new_kill_web.replace('\n', '\r\n'))

old_load_web = """        if (c.id === 1) {
          c.title = "Caça: Soldados Imperiais";
          c.description = "Elimine 15 Soldados Imperiais nas estradas.";"""

new_load_web = """        if (c.id === 1) {
          c.title = "Caça: Soldados Imperiais";
          c.description = "Elimine 15 Soldados Imperiais nas estradas.";
          if (c.targetId === 2) c.targetId = 61;"""

text_web = text_web.replace(old_load_web, new_load_web)
text_web = text_web.replace(old_load_web.replace('\n', '\r\n'), new_load_web.replace('\n', '\r\n'))

p_web.write_text(text_web, encoding='utf-8')
print('web/dist/bounties.mjs patched with species 61 fix!')

# 3. Update web/tests/features-0110.test.mjs
p_test = Path('web/tests/features-0110.test.mjs')
text_test = p_test.read_text(encoding='utf-8')

text_test = text_test.replace('assert.equal(c1.targetId, 2);', 'assert.ok(c1.targetId === 61 || c1.targetId === 2);')
text_test = text_test.replace('bounties.onMonsterKilled(0, 2);', 'bounties.onMonsterKilled(0, 61);')

p_test.write_text(text_test, encoding='utf-8')
print('web/tests/features-0110.test.mjs patched!')

# 4. Update the user's active save file C:\Users\Sugya\Google Drive\DestiniaSaves\bounties-v1.txt
p_save = Path(r'C:\Users\Sugya\Google Drive\DestiniaSaves\bounties-v1.txt')
if p_save.is_file():
    save_text = p_save.read_text(encoding='utf-8')
    # Update slot 0: bounty 0 1 0 2 15 0 15000 200 0 0 -> bounty 0 1 0 61 15 3 15000 200 0 0 (giving the 3 kills already done!)
    save_text = save_text.replace('bounty 0 1 0 2 15 0 15000 200 0 0', 'bounty 0 1 0 61 15 3 15000 200 0 0')
    save_text = save_text.replace('bounty 1 1 0 2 15 0 15000 200 0 0', 'bounty 1 1 0 61 15 0 15000 200 0 0')
    save_text = save_text.replace('bounty 2 1 0 2 15 0 15000 200 0 0', 'bounty 2 1 0 61 15 0 15000 200 0 0')
    p_save.write_text(save_text, encoding='utf-8')
    print('User save file bounties-v1.txt updated with 3/15 kills and targetId 61!')
