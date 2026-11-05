// Difficulty Engine e Dificuldades Canônicas (0.10.3)
export const DifficultyTier = Object.freeze({
  Normal: 0,
  Dificil: 1,
  Pesadelo: 2,
  Inferno: 3,
  Abissal: 4
});

export const DifficultyLabels = ["Normal", "Difícil", "Pesadelo", "Inferno", "Abissal"];

export const DifficultyMultipliers = Object.freeze({
  0: { hp: 1.0,  dmg: 1.0, gold: 1.0, xp: 1.0 },
  1: { hp: 1.5,  dmg: 1.3, gold: 1.3, xp: 1.3 },
  2: { hp: 2.5,  dmg: 1.8, gold: 1.8, xp: 1.8 },
  3: { hp: 4.0,  dmg: 2.5, gold: 2.5, xp: 2.5 },
  4: { hp: 6.0,  dmg: 4.0, gold: 3.5, xp: 3.5 }
});

export class DifficultyWeb {
  constructor(engine) {
    this.engine = engine;
    this.currentTier = DifficultyTier.Normal;
    this.unlockedTier = DifficultyTier.Normal;
    this.open = false;
    this.selected = 0;
    this.statusMessage = "";
  }

  install() {
    // No ambiente Web, mantemos a seleção e interface leves sem instalar hooks de ARM
    // pesados em 0x12444 / 0x16b7c que degradam o compilador JIT do Unicorn.js em navegadores móveis.
  }

  onBossDefeated(species) {
    if (species === 108 && this.unlockedTier < DifficultyTier.Abissal) {
      if (this.currentTier === this.unlockedTier) {
        this.unlockedTier++;
        this.statusMessage = "NOVA DIFICULDADE DESBLOQUEADA!";
      }
    }
  }

  setTier(tier) {
    if (tier >= 0 && tier <= this.unlockedTier) {
      this.currentTier = tier;
      return true;
    }
    return false;
  }

  handleKey(key, down) {
    if (!this.open) return false;
    if (!down) return true;

    if (key === 10 || key === 27) {
      this.open = false;
      return true;
    }
    if (key === 3 || key === 5) {
      this.selected = (this.selected + 4) % 5;
      return true;
    }
    if (key === 9 || key === 7) {
      this.selected = (this.selected + 1) % 5;
      return true;
    }
    if (key === 6 || key === 17) {
      if (this.selected <= this.unlockedTier) {
        this.currentTier = this.selected;
        this.open = false;
      } else {
        this.statusMessage = "BLOQUEADA: DERROTE DIABLO";
      }
      return true;
    }
    return true;
  }

  layout(){const width=this.engine?.width||800,height=this.engine?.height||480,scale=Math.max(1,Math.floor(Math.min(width/400,height/240))),w=Math.min(360*scale,width-16*scale),h=Math.min(220*scale,height-16*scale),x=Math.floor((width-w)/2),y=Math.floor((height-h)/2);return {x,y,w,h,scale,rows:Array.from({length:5},(_,i)=>({x:x+8*scale,y:y+(30+i*28)*scale,w:w-16*scale,h:26*scale}))};}
  handleTouch(x,y,type){
    if(!this.open)return false;
    if(type===0){const i=this.layout().rows.findIndex(b=>x>=b.x&&x<b.x+b.w&&y>=b.y&&y<b.y+b.h);if(i>=0){this.selected=i;this.handleKey(17,true);}else this.open=false;}
    return true;
  }
  paint(pixels){
    if(!this.open||!this.engine?.originalUi)return;
    const b=this.layout(),s=b.scale,ui=this.engine.originalUi;
    const text=(x,y,value,color=0xffff,width=b.x+b.w-8*s-x)=>ui.text(pixels,x,y,value,color,width,s);
    ui.panel(pixels,b);text(b.x+8*s,b.y+8*s,'DIFICULDADE',0xe5b2);
    b.rows.forEach((r,i)=>{const unlocked=i<=this.unlockedTier,current=i===this.currentTier,selected=i===this.selected;text(r.x+4*s,r.y+6*s,(selected?'> ':'  ')+DifficultyLabels[i]+(current?' [ATUAL]':unlocked?'':' [BLOQUEADA]'),current?0x07e0:unlocked?0xffff:0xf800,r.w-8*s);});
    text(b.x+8*s,b.y+b.h-38*s,this.statusMessage,0xf800);
    text(b.x+8*s,b.y+b.h-22*s,'ACAO: CONFIRMAR / VOLTAR: SAIR',0xe5b2);
  }
}
