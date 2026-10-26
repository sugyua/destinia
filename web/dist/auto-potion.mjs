import {overlayGlyph} from './overlay-font.mjs';

export class AutoPotionController {
  constructor(engine) {
    this.engine = engine;
    this.enabled = true;
    this.hpThresholdPercent = 35;
    this.mpThresholdPercent = 25;
    this.cooldownFrames = 60;
    this.cooldownHpTimer = 0;
    this.cooldownMpTimer = 0;
    this.notice = "";
    this.noticeTimer = 0;
  }

  rd16(addr) {
    if (!this.engine) return 0;
    if (typeof this.engine.rd16 === 'function') return this.engine.rd16(addr);
    if (typeof this.engine.get16 === 'function') return this.engine.get16(addr);
    return 0;
  }

  wr16(addr, val) {
    if (!this.engine) return;
    if (typeof this.engine.wr16 === 'function') return this.engine.wr16(addr, val);
    if (typeof this.engine.set16 === 'function') return this.engine.set16(addr, val);
  }

  toggle() {
    this.enabled = !this.enabled;
    this.notice = this.enabled ? "AUTO-POCAO: ATIVADO" : "AUTO-POCAO: DESATIVADO";
    this.noticeTimer = 90;
  }

  tick() {
    if (this.cooldownHpTimer > 0) this.cooldownHpTimer--;
    if (this.cooldownMpTimer > 0) this.cooldownMpTimer--;
    if (this.noticeTimer > 0) this.noticeTimer--;
  }

  update() {
    this.tick();
    if (!this.enabled || !this.engine || !this.engine.exports) return;
    const g = this.engine.exports.get('Game');
    if (!g) return;

    // Verificar se esta no jogo (0x4854 == 0)
    const state = this.rd16(g + 0x4854);
    if (state !== 0) return;

    const curHp = this.rd16(g + 0x48f2);
    let maxHp = this.rd16(g + 0x48c4);
    if (!maxHp) maxHp = this.rd16(g + 0x48f4);

    const curMp = this.rd16(g + 0x48f6);
    let maxMp = this.rd16(g + 0x48c6);
    if (!maxMp) maxMp = this.rd16(g + 0x48f8);

    if (!curHp || !maxHp) return;

    // HP Potion
    if (this.cooldownHpTimer === 0 && Math.floor((curHp * 100) / maxHp) <= this.hpThresholdPercent) {
      const heal = Math.max(50, Math.floor((maxHp * 40) / 100));
      const newHp = Math.min(maxHp, curHp + heal);
      this.wr16(g + 0x48f2, newHp);
      this.cooldownHpTimer = this.cooldownFrames;
      this.notice = "AUTO-POCAO: PV RECUPERADO!";
      this.noticeTimer = 60;
    }

    // MP Potion
    if (this.cooldownMpTimer === 0 && maxMp > 0 && Math.floor((curMp * 100) / maxMp) <= this.mpThresholdPercent) {
      const healMp = Math.max(30, Math.floor((maxMp * 35) / 100));
      const newMp = Math.min(maxMp, curMp + healMp);
      this.wr16(g + 0x48f6, newMp);
      this.cooldownMpTimer = this.cooldownFrames;
      this.notice = "AUTO-POCAO: PM RECUPERADO!";
      this.noticeTimer = 60;
    }
  }

  paintNotice(fb, width, height) {
    if (this.noticeTimer <= 0 || !this.notice) return;
    let tw = 0;
    for (const ch of this.notice) {
      const g = overlayGlyph(ch);
      tw += g ? g.width + 1 : 6;
    }
    const x = Math.floor((width - tw) / 2);
    const y = height - 42;

    // Fundo escuro
    for (let dy = -2; dy < 12; dy++) {
      for (let dx = -6; dx < tw + 6; dx++) {
        const px = x + dx, py = y + dy;
        if (px >= 0 && px < width && py >= 0 && py < height) {
          const at = (py * width + px) * 2;
          fb[at] = 0x00;
          fb[at + 1] = 0x10;
        }
      }
    }

    const col = this.enabled ? 0x07e0 : 0xf800;
    let cx = x;
    for (const ch of this.notice) {
      const glyph = overlayGlyph(ch);
      if (glyph) {
        for (let gy = 0; gy < glyph.height; gy++) {
          for (let gx = 0; gx < glyph.width; gx++) {
            if (glyph.rows[gy] & (1 << (glyph.width - 1 - gx))) {
              const tx = cx + gx, ty = y + gy;
              if (tx >= 0 && tx < width && ty >= 0 && ty < height) {
                const at = (ty * width + tx) * 2;
                fb[at] = col & 0xff;
                fb[at + 1] = (col >> 8) & 0xff;
              }
            }
          }
        }
        cx += glyph.width + 1;
      } else {
        cx += 6;
      }
    }
  }
}
