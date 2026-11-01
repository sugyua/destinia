
export class AutoPotionController {
  constructor(engine) {
    this.engine = engine;
    this.enabled = true;
    this.hpThresholdPercent = 50;
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
    const maxHp = this.rd16(g + 0x48c4);

    const curMp = this.rd16(g + 0x48f6);
    let maxMp = this.rd16(g + 0x48c6);
    if (!maxMp) maxMp = this.rd16(g + 0x48f8);

    if (!curHp) return;

    // HP Potion
    if (this.cooldownHpTimer === 0 && maxHp > 0 && curHp < maxHp && curHp * 100 <= maxHp * this.hpThresholdPercent) {
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

  paintNotice(fb,width,height){
    if(this.noticeTimer<=0||!this.notice||!this.engine?.originalUi)return;
    const s=Math.max(1,Math.floor(Math.min(width/400,height/240))),w=Math.min(width-12*s,(this.notice.length*6+16)*s),h=24*s,x=Math.floor((width-w)/2),y=height-48*s,ui=this.engine.originalUi;
    ui.panel(fb,{x,y,w,h});ui.text(fb,x+8*s,y+6*s,this.notice,this.enabled?0x07e0:0xf800,w-16*s,s);
  }
}
