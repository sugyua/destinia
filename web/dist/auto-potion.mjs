
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
    if (typeof this.engine.write === 'function') return this.engine.write(addr, Uint8Array.of(val & 255, (val >>> 8) & 255));
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

  useNative(type) {
    const e=this.engine, g=e?.exports?.get('Game'), fn=e?.exports?.get('_pressedQuickSlot'), table=e?.assets?.get('item_en.gdt');
    if (!g || !fn || !table || table.length!==158*166 || typeof e.call!=='function' ||
        !(typeof e.wr16==='function' || typeof e.set16==='function' || typeof e.write==='function')) return false;
    const group=this.rd16(g+0x4a8c); if(group>4) return false;
    let id=-1;
    for(let bag=0;bag<4;bag++) {
      const base=g+0x433c+bag*0x92, count=this.rd16(base); if(count>24)return false;
      for(let i=0;i<count;i++) {
        const rec=base+2+6*i, kind=this.rd16(rec), candidate=this.rd16(rec+2), qty=this.rd16(rec+4);
        if(kind!==1 || !qty || candidate<type*5 || candidate>=type*5+5)continue;
        const at=candidate*166, u16=n=>table[at+n]|(table[at+n+1]<<8);
        if(u16(0)===candidate && u16(4)===type && u16(6)===candidate%5 && id<0)id=candidate;
      }
    }
    if(id<0)return false;
    const total=()=>{let n=0;for(let bag=0;bag<4;bag++){const base=g+0x433c+bag*0x92,count=this.rd16(base);if(count>24)return -1;for(let i=0;i<count;i++){const rec=base+2+6*i;if(this.rd16(rec)===1&&this.rd16(rec+2)===id)n+=this.rd16(rec+4);}}return n;};
    const before=total(),slot=g+0x4a90+group*10,old=this.rd16(slot);
    if(before<=0)return false;
    this.wr16(slot,0x4000|id);
    try {e.call(fn,[0]);} finally {this.wr16(slot,old);}
    const after=total();return after>=0&&before===after+1;
  }

  update() {
    this.tick();
    const g=this.engine?.exports?.get('Game');
    if(!this.enabled || !g || this.rd16(g+0x4854)!==0)return false;
    const mp=this.rd16(g+0x48f2), hp=this.rd16(g+0x48f4), maxMp=this.rd16(g+0x48f6), maxHp=this.rd16(g+0x48f8);
    if(!hp || hp>32767)return false;
    let used=false;
    if(this.cooldownHpTimer===0 && maxHp>0 && hp<maxHp && hp*100<=maxHp*this.hpThresholdPercent && this.useNative(0)) {
      this.cooldownHpTimer=this.cooldownFrames;this.notice="AUTO-POCAO: PV RECUPERANDO!";this.noticeTimer=60;used=true;
    }
    if(this.cooldownMpTimer===0 && maxMp>0 && mp<maxMp && mp*100<=maxMp*this.mpThresholdPercent && this.useNative(1)) {
      this.cooldownMpTimer=this.cooldownFrames;this.notice="AUTO-POCAO: PM RECUPERANDO!";this.noticeTimer=60;used=true;
    }
    return used;
  }

  paintNotice(fb,width,height){
    if(this.noticeTimer<=0||!this.notice||!this.engine?.originalUi)return;
    const s=Math.max(1,Math.floor(Math.min(width/400,height/240))),w=Math.min(width-12*s,(this.notice.length*6+16)*s),h=24*s,x=Math.floor((width-w)/2),y=height-48*s,ui=this.engine.originalUi;
    ui.panel(fb,{x,y,w,h});ui.text(fb,x+8*s,y+6*s,this.notice,this.enabled?0x07e0:0xf800,w-16*s,s);
  }
}
