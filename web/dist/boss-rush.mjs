import { ALL_TITLES, TitlesStorage, TitleId } from './titles.mjs';

export const BOSS_RUSH_LIST = [
  {species: 100, namePt: "Guardiao da Masmorra", nameEn: "Dungeon Guardian", goldCost: 500, area: 1, map: 5},
  {species: 101, namePt: "Defcan", nameEn: "Defcan", goldCost: 1000, area: 1, map: 16},
  {species: 102, namePt: "Salas", nameEn: "Salas", goldCost: 2500, area: 2, map: 10},
  {species: 103, namePt: "Defcan Desperto", nameEn: "Awakened Defcan", goldCost: 5000, area: 1, map: 21},
  {species: 104, namePt: "Sileth", nameEn: "Sileth", goldCost: 7500, area: 1, map: 31},
  {species: 105, namePt: "Berserk Tamur", nameEn: "Berserk Tamur", goldCost: 10000, area: 1, map: 41},
  {species: 106, namePt: "Imperador Kaiser", nameEn: "Emperor Kaiser", goldCost: 15000, area: 2, map: 28},
  {species: 107, namePt: "Baramos", nameEn: "Baramos", goldCost: 25000, area: 2, map: 16},
  {species: 108, namePt: "Diablo", nameEn: "Diablo", goldCost: 35000, area: 1, map: 51},
  {species: 109, namePt: "Guardiao de Lava", nameEn: "Lava Guardian", goldCost: 50000, area: 2, map: 48},
  {species: 999, namePt: "ARENA GAUNTLET (10 CHEFES)", nameEn: "BOSS GAUNTLET (10 BOSSES)", goldCost: 10000, area: 1, map: 5}
];

export const ABYSS_FLOORS = Array.from({length: 50}, (_, i) => {
  const f = i + 1;
  const isBoss = (f % 5 === 0);
  return {
    floor: f,
    recommendedLevel: f * 2,
    goldCost: f * 150,
    area: (f % 2 === 0) ? 2 : 1,
    map: 5 + (f % 45),
    isBoss,
    titlePt: isBoss ? `Chefe Abissal (Andar ${f})` : `Camara dos Renegados (Andar ${f})`
  };
});

export const PETS_LIST = [
  {id: 0, namePt: "Lobo Glacial", descPt: "+10% Velocidade | +15% Critico", level: 1, xp: 0},
  {id: 1, namePt: "Fenix Ignea", descPt: "+20% Dano de Ataque | +10% Regen PV", level: 1, xp: 0},
  {id: 2, namePt: "Golem de Pedra", descPt: "+25% Defesa Fisica | +15% Vida Max", level: 1, xp: 0},
  {id: 3, namePt: "Mini-Diablo", descPt: "+15% Dano Geral | +5% Roubo de Vida", level: 1, xp: 0}
];

export class BossRushUI {
  constructor(engine) {
    this.engine = engine;
    this.open = false;
    this.currentTab = 0; // 0 = Chefes, 1 = Torre Abissal, 2 = Mascotes, 3 = Santuario
    this.selectedIndex = 0;
    this.scrollOffset = 0;
    this.statusMessage = "";
    this.maxUnlockedFloor = 5;
    this.activePetId = 0;
    this.ngCycle = 0;
    this.titlesStore = new TitlesStorage();
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

  openMenu() {
    this.open = true;
    this.currentTab = 0;
    this.selectedIndex = 0;
    this.scrollOffset = 0;
    this.statusMessage = "";
  }

  closeMenu() {
    this.open = false;
    this.statusMessage = "";
  }

  getItemsCount() {
    if (this.currentTab === 0) return BOSS_RUSH_LIST.length;
    if (this.currentTab === 1) return this.maxUnlockedFloor;
    if (this.currentTab === 2) return PETS_LIST.length;
    if (this.currentTab === 3) return 3;
    if (this.currentTab === 4) return ALL_TITLES.length;
    if(this.currentTab===5)return this.engine?.features0130?.rows().length||0;
    return 0;
  }

  handleKey(key, down) {
    if (!this.open) return false;
    if (!down) return true;

    // Tabs: 5 (Left), 7 (Right)
    if (key === 5) {
      this.currentTab = (this.currentTab - 1 + 6) % 6;
      this.selectedIndex = 0;
      this.scrollOffset = 0;
      this.statusMessage = "";
      return true;
    }
    if (key === 7) {
      this.currentTab = (this.currentTab + 1) % 6;
      this.selectedIndex = 0;
      this.scrollOffset = 0;
      this.statusMessage = "";
      return true;
    }

    const count = this.getItemsCount();
    // Cima: 3
    if (key === 3) {
      if (count > 0) {
        this.selectedIndex = (this.selectedIndex - 1 + count) % count;
        if (this.selectedIndex < this.scrollOffset) this.scrollOffset = this.selectedIndex;
        if (this.selectedIndex >= this.scrollOffset + 7) this.scrollOffset = this.selectedIndex - 6;
      }
      this.statusMessage = "";
      return true;
    }
    // Baixo: 9
    if (key === 9) {
      if (count > 0) {
        this.selectedIndex = (this.selectedIndex + 1) % count;
        if (this.selectedIndex >= this.scrollOffset + 7) this.scrollOffset = this.selectedIndex - 6;
        if (this.selectedIndex < this.scrollOffset) this.scrollOffset = this.selectedIndex;
      }
      this.statusMessage = "";
      return true;
    }
    // Voltar / Fechar: 10 ou 27
    if (key === 10 || key === 27) {
      this.closeMenu();
      return true;
    }
    // Confirmar / Ação: 6 ou 17
    if (key === 6 || key === 17) {
      const curGold = this.engine && this.engine.exports ? this.engine.call(this.engine.exports.get('getMyGold'), []) : 999999;

      if (this.currentTab === 0) {
        const boss = BOSS_RUSH_LIST[this.selectedIndex];
        if (boss) {
          if (curGold < boss.goldCost) {
            this.statusMessage = "OURO INSUFICIENTE!";
          } else {
            if (this.engine && this.engine.exports) {
              this.engine.call(this.engine.exports.get('setMyGold'), [curGold - boss.goldCost]);
              this.engine.call(this.engine.exports.get('Map_loadData'), [boss.area, boss.map, 1]);
            }
            this.closeMenu();
          }
        }
      } else if (this.currentTab === 1) {
        const fl = ABYSS_FLOORS[this.selectedIndex];
        if (fl) {
          if (curGold < fl.goldCost) {
            this.statusMessage = "OURO INSUFICIENTE!";
          } else {
            if (this.engine && this.engine.exports) {
              this.engine.call(this.engine.exports.get('setMyGold'), [curGold - fl.goldCost]);
              this.engine.call(this.engine.exports.get('Map_loadData'), [fl.area, fl.map, 1]);
            }
            this.statusMessage = `DESAFIANDO ANDAR ${fl.floor}!`;
            this.closeMenu();
          }
        }
      } else if (this.currentTab === 2) {
        this.activePetId = this.selectedIndex;
        this.statusMessage = `MASCOTE ${PETS_LIST[this.selectedIndex].namePt} INVOCADO!`;
      } else if (this.currentTab === 3) {
        if (this.selectedIndex === 0) {
          if (curGold < 5000) {
            this.statusMessage = "OURO INSUFICIENTE (REQUER 5000 G)";
          } else {
            if (this.engine && this.engine.exports) {
              this.engine.call(this.engine.exports.get('setMyGold'), [curGold - 5000]);
              const g = this.engine.exports.get('Game');
              const lvl = this.rd16(g + 0x48fa) || 1;
              const refund = (lvl > 1) ? (lvl - 1) * 3 : 0;
              this.wr16(g + 0x48ba, 10);
              this.wr16(g + 0x48bc, 10);
              this.wr16(g + 0x48be, 10);
              this.wr16(g + 0x48c0, 10);
              this.wr16(g + 0x48f0, (this.rd16(g + 0x48f0) || 0) + refund);
              this.engine.call(this.engine.exports.get('GameUI_calcHeroStat'), []);
            }
            this.statusMessage = "ATRIBUTOS RESETADOS! PONTOS DEVOLVIDOS";
          }
        } else if (this.selectedIndex === 1) {
          if (curGold < 5000) {
            this.statusMessage = "OURO INSUFICIENTE (REQUER 5000 G)";
          } else {
            if (this.engine && this.engine.exports) {
              this.engine.call(this.engine.exports.get('setMyGold'), [curGold - 5000]);
              const g = this.engine.exports.get('Game');
              const lvl = this.rd16(g + 0x48fa) || 1;
              const refund = (lvl > 1) ? lvl - 1 : 0;
              this.wr16(g + 0x48ee, (this.rd16(g + 0x48ee) || 0) + refund);
            }
            this.statusMessage = "HABILIDADES RESETADAS! PONTOS DEVOLVIDOS";
          }
        } else if (this.selectedIndex === 2) {
          if (this.ngCycle >= 5) {
            this.statusMessage = "CICLO MAXIMO ATINGIDO (NG+5)!";
          } else {
            this.ngCycle++;
            if (this.engine && this.engine.exports) {
              this.engine.call(this.engine.exports.get('Map_loadData'), [0, 1, 1]);
            }
            this.statusMessage = `NOVO JOGO+ ATIVADO (NG+${this.ngCycle})!`;
            this.closeMenu();
          }
        }
      }
      return true;
    }
    return true;
  }

  layout(width=this.engine.width,height=this.engine.height) {
    const scale=Math.max(1,Math.floor(Math.min(width/400,height/240)));
    const w=Math.min(360*scale,width-20*scale),h=Math.min(220*scale,height-20*scale);
    const x=Math.floor((width-w)/2),y=Math.floor((height-h)/2),tabW=Math.floor((w-16*scale)/3);
    return {x,y,w,h,scale,top:y+58*scale,row:(this.currentTab===2||this.currentTab===3?24:16)*scale,
      close:{x:x+w-26*scale,y:y+3*scale,w:22*scale,h:17*scale},
      tabs:Array.from({length:6},(_,i)=>({x:x+8*scale+i%3*tabW,y:y+(20+Math.floor(i/3)*16)*scale,w:tabW,h:16*scale}))};
  }
  rowRects(b=this.layout()) {
    return Array.from({length:Math.min(7,this.getItemsCount()-this.scrollOffset)},(_,i)=>({x:b.x+8*b.scale,y:b.top+i*b.row,w:b.w-16*b.scale,h:b.row,index:i+this.scrollOffset}));
  }
  handleTouch(x,y,phase) {
    if(!this.open)return false;
    const b=this.layout(),inside=r=>x>=r.x&&x<r.x+r.w&&y>=r.y&&y<r.y+r.h;
    if(!inside(b)||inside(b.close)){if(phase===0)this.closeMenu();return true;}
    const tab=b.tabs.findIndex(inside);
    if(tab>=0){if(phase===0){this.currentTab=tab;this.selectedIndex=0;this.scrollOffset=0;this.statusMessage='';}return true;}
    const row=this.rowRects(b).find(inside);
    if(row&&phase===0){if(this.selectedIndex===row.index)this.handleKey(17,true);else{this.selectedIndex=row.index;this.statusMessage='';}}
    return true;
  }
  paint(fb,width,height) {
    if(!this.open||!fb||!this.engine.originalUi)return;
    const b=this.layout(width,height),s=b.scale,ui=this.engine.originalUi;
    const text=(x,y,value,color=0xffff,w=b.x+b.w-8*s-x)=>ui.text(fb,x,y,value,color,Math.max(1,w),s);
    ui.panel(fb,b);
    text(b.x+8*s,b.y+5*s,'MAGO DOS BOSSES -- SALAO ANCESTRAL',0xd7bf,b.w-40*s);
    text(b.close.x,b.y+5*s,'[X]',0xf800,22*s);
    const tabNames=['[CHEFES]','[TORRE]','[MASCOTES]','[SANTUARIO]','[TITULOS]','[SOBREVIVENCIA]'];
    b.tabs.forEach((r,i)=>text(r.x+2*s,r.y+2*s,tabNames[i],i===this.currentTab?0x07e0:0xce59,r.w-4*s));
    for(const r of this.rowRects(b)){
      const i=r.index,sel=i===this.selectedIndex,prefix=sel?'> ':'  ';
      let label='',detail='',cost='',color=sel?0xffff:0xce59;
      if(this.currentTab===0){const v=BOSS_RUSH_LIST[i];label=(i+1)+'. '+v.namePt;cost=v.goldCost+' G';}
      if(this.currentTab===1){const v=ABYSS_FLOORS[i];label='Andar '+v.floor+': '+v.titlePt;cost='(Lv'+v.recommendedLevel+') '+v.goldCost+' G';if(v.isBoss)color=sel?0xfd00:0xf800;}
      if(this.currentTab===2){const v=PETS_LIST[i],active=this.activePetId===i;label=(active?'[ATIVO] ':'[GUARDADO] ')+v.namePt;detail=v.descPt;if(active)color=0x07e0;}
      if(this.currentTab===3)label=['Resetar Atributos (FOR/DES/CON/INT)','Resetar Arvore de Habilidades',`Avancar Ciclo Novo Jogo+ [NG+${this.ngCycle}]`][i];
      if(this.currentTab===4)label=ALL_TITLES[i].nome;
      if(this.currentTab===5)label=this.engine.features0130?.rows()[i]||'';
      const costW=cost?Math.min(100*s,(cost.length*6+4)*s):0;
      text(r.x+4*s,r.y,prefix+label,color,r.w-8*s-costW);
      if(cost)text(r.x+r.w-costW,r.y,cost,sel?0x07e0:0xa510,costW);
      if(detail)text(r.x+12*s,r.y+12*s,detail,0xd7bf,r.w-16*s);
    }
    const gold=this.engine.exports?this.engine.call(this.engine.exports.get('getMyGold'),[]):0;
    text(b.x+8*s,b.y+b.h-30*s,this.statusMessage||'OURO: '+gold+' G',this.statusMessage?0xf800:0xd7bf);
    text(b.x+8*s,b.y+b.h-16*s,'[< >]: ABAS  [ENTER]: CONFIRMAR  [ESC]: SAIR',0xd7bf);
  }
}
