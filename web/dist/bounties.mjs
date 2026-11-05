// Sistema de Recompensas e Contratos Offline Web (0.11.0)
// Permite caçar monstros e chefes para obter ouro e moedas de cash offline.

export class BountiesWeb {
  constructor(engine) {
    this.engine = engine;
    this.open = false;
    this.contractsBySlot = new Map();
    this.load();
  }

  load() {
    try {
      if (typeof localStorage !== 'undefined') {
        const raw = localStorage.getItem('destinia_bounties_v1');
        if (raw) {
          this.deserialize(raw);
        }
      }
    } catch (_) {}
  }

  save() {
    try {
      if (typeof localStorage !== 'undefined') {
        localStorage.setItem('destinia_bounties_v1', this.serialize());
      }
    } catch (_) {}
  }

  serialize() {
    const lines = ["bounties_version 1"];
    for (const [slot, list] of this.contractsBySlot.entries()) {
      for (const c of list) {
        lines.push(`bounty ${slot} ${c.id} ${c.type} ${c.targetId} ${c.targetCount} ${c.currentCount} ${c.rewardGold} ${c.rewardCash} ${c.completed ? 1 : 0} ${c.claimed ? 1 : 0}`);
      }
    }
    return lines.join('\n');
  }

  deserialize(text) {
    const lines = text.split('\n');
    this.contractsBySlot.clear();
    let valid = false;
    for (let line of lines) {
      line = line.trim();
      if (!line || line.startsWith('#')) continue;
      const parts = line.split(/\s+/);
      if (parts[0] === 'bounties_version' && parts[1] === '1') {
        valid = true;
      } else if (valid && parts[0] === 'bounty' && parts.length >= 10) {
        const slot = parseInt(parts[1], 10);
        if (!this.contractsBySlot.has(slot)) {
          this.contractsBySlot.set(slot, []);
        }
        const id = parseInt(parts[2], 10);
        const type = parseInt(parts[3], 10);
        const targetId = parseInt(parts[4], 10);
        const targetCount = parseInt(parts[5], 10);
        const currentCount = parseInt(parts[6], 10);
        const rewardGold = parseInt(parts[7], 10);
        const rewardCash = parseInt(parts[8], 10);
        const completed = parts[9] === '1';
        const claimed = parts[10] === '1';

        let title = "Contrato de Caça";
        let description = `Elimine o alvo ${targetId}.`;
        if (id === 1) {
          title = "Caça: Soldados Imperiais";
          description = `Elimine ${targetCount} Soldados Imperiais.`;
        } else if (id === 2) {
          title = "Bounty: Diablo, Senhor do Caos";
          description = "Derrote o Boss Diablo no Boss Rush ou mapa.";
        } else if (id === 3) {
          title = "Bounty: Guardião Ancestral";
          description = `Derrote o Guardião Ancestral (Boss ${targetId}).`;
        }

        this.contractsBySlot.get(slot).push({
          id, type, targetId, targetCount, currentCount, rewardGold, rewardCash, completed, claimed, title, description
        });
      }
    }
  }

  getContracts(slot = 0) {
    if (!this.contractsBySlot.has(slot) || this.contractsBySlot.get(slot).length === 0) {
      this.generateDailyContracts(slot);
    }
    return this.contractsBySlot.get(slot);
  }

  generateDailyContracts(slot = 0) {
    const list = [
      {
        id: 1,
        type: 0,
        targetId: 61, // Imperial Soldier
        targetCount: 15,
        currentCount: 0,
        rewardGold: 15000,
        rewardCash: 200,
        completed: false,
        claimed: false,
        title: "Caça: Soldados Imperiais",
        description: "Elimine 15 Soldados Imperiais nas estradas."
      },
      {
        id: 2,
        type: 1,
        targetId: 108, // Diablo
        targetCount: 1,
        currentCount: 0,
        rewardGold: 35000,
        rewardCash: 500,
        completed: false,
        claimed: false,
        title: "Bounty: Diablo, Senhor do Caos",
        description: "Derrote o Boss Diablo no Boss Rush ou mapa."
      },
      {
        id: 3,
        type: 1,
        targetId: 105, // Guardião Ancestral
        targetCount: 1,
        currentCount: 0,
        rewardGold: 25000,
        rewardCash: 350,
        completed: false,
        claimed: false,
        title: "Bounty: Guardião Ancestral",
        description: "Derrote o Guardião Ancestral (Boss 105)."
      }
    ];
    this.contractsBySlot.set(slot, list);
    this.save();
    return list;
  }

  onMonsterKilled(slot = 0, species = 0) {
    const list = this.getContracts(slot);
    let changed = false;
    for (const c of list) {
      const matches = (!c.completed) && (
        c.targetId === species ||
        (c.id === 1 && (species === 61 || species === 62 || species === 63 || species === 2))
      );
      if (matches) {
        c.currentCount++;
        if (c.currentCount >= c.targetCount) {
          c.currentCount = c.targetCount;
          c.completed = true;
        }
        changed = true;
      }
    }
    if (changed) {
      this.save();
    }
  }

  claim(slot = 0, contractId = 1) {
    const list = this.getContracts(slot);
    const c = list.find(x => x.id === contractId);
    if (!c || !c.completed || c.claimed) return null;
    c.claimed = true;
    this.save();
    return { gold: c.rewardGold, cash: c.rewardCash };
  }

  currentSlot() {
    if (!this.engine?.exports) return 0;
    const g = this.engine.exports.get("Game");
    if (!g) return 0;
    const s = this.engine.get16(g + 0x4886);
    return s < 10 ? s : 0;
  }

  toggle() {
    this.open = !this.open;
    if (this.open) {
      this.load();
    }
    return this.open;
  }

  handleKey(key, down) {
    if (!this.open) return false;
    if (!down) return true;

    if (key === 27 || key === 10 || key === 'c' || key === 'C' || key === 'F8') { // ESC / Voltar / F8
      this.open = false;
      return true;
    }
    return true;
  }

  handleTouch(x,y,type) {
    if(!this.open)return false;if(type!==0)return true;
    const l=this.layout();x=(x-l.x)/l.scale;y=(y-l.y)/l.scale;
    if(x>=320&&x<356&&y>=4&&y<24){this.open=false;return true;}
    const slot=this.currentSlot(),list=this.getContracts(slot);
    for(let i=0;i<Math.min(3,list.length);i++)if(x>=252&&x<346&&y>=62+i*57&&y<82+i*57){
      const c=list[i];if(c.completed&&!c.claimed){const reward=this.claim(slot,c.id);if(reward&&this.engine?.exports){const g=this.engine.exports.get('Game');if(g)this.engine.set32(g+0x4338,this.engine.get32(g+0x4338)+reward.gold);}}return true;
    }
    return true;
  }
  layout() {
    const width=this.engine.width||400,height=this.engine.height||240;
    const scale=Math.min(width/400,height/240);
    return {scale,x:(width-360*scale)/2,y:(height-220*scale)/2};
  }

  paint(pixels) {
    const ui=this.engine.originalUi;
    if(!this.open||!ui||!pixels)return;
    const {scale:s,x,y}=this.layout();
    const panel=(a,b,w,h)=>ui.panel(pixels,{x:x+a*s,y:y+b*s,w:w*s,h:h*s});
    const text=(a,b,value,color=0xffff,w=340-a)=>ui.text(pixels,x+a*s,y+b*s,value,color,w*s,s);
    panel(0,0,360,220);
    text(324,7,'SAIR',0xffc0,30);
    text(10,7,'CONTRATOS DE CACA',0xffc0,280);
    const list=this.getContracts(this.currentSlot());
    list.slice(0,3).forEach((c,i)=>{
      const row=28+i*57;panel(10,row,340,55);
      text(15,row+3,c.title,0xffc0,330);
      text(15,row+15,c.description,0xffff,330);
      text(15,row+28,c.currentCount+' / '+c.targetCount+'  Ouro '+c.rewardGold,0xffff,230);
      text(15,row+40,'Cash '+c.rewardCash,0xffff,230);
      panel(252,row+34,94,20);
      text(257,row+38,c.claimed?'RESGATADO':c.completed?'Resgatar':'EM ANDAMENTO',c.completed&&!c.claimed?0xffc0:0x8c51,84);
    });
    text(10,207,'Toque Resgatar ao concluir. C / ESC: sair',0x8c51,340);
  }
}
