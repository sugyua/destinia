// Baú Compartilhado Web (0.11.0)
// Permite transferir equipamentos e consumíveis entre personagens de forma segura.

export const MAX_STASH_CAPACITY = 48; // 2 páginas de 24 slots

export class SharedStashWeb {
  constructor(engine) {
    this.engine = engine;
    this.open = false;
    this.page = 0; // 0 ou 1
    this.selectedIndex = 0;
    this.items = [];
    this.load();
  }

  load() {
    try {
      if (typeof localStorage !== 'undefined') {
        const raw = localStorage.getItem('destinia_shared_stash_v1');
        if (raw) {
          this.deserialize(raw);
        }
      }
    } catch (_) {}
  }

  save() {
    try {
      if (typeof localStorage !== 'undefined') {
        localStorage.setItem('destinia_shared_stash_v1', this.serialize());
      }
    } catch (_) {}
  }

  serialize() {
    const lines = ["shared_stash_version 1", `count ${this.items.length}`];
    for (const it of this.items) {
      lines.push(`item ${it.pool} ${it.count} ${it.durability} ${it.itemType} ${it.rank} ${it.level} ${it.jobClass} ${it.name}`);
    }
    return lines.join('\n');
  }

  deserialize(text) {
    const lines = text.split('\n');
    this.items = [];
    let valid = false;
    for (let line of lines) {
      line = line.trim();
      if (!line || line.startsWith('#')) continue;
      const parts = line.split(/\s+/);
      if (parts[0] === 'shared_stash_version' && parts[1] === '1') {
        valid = true;
      } else if (valid && parts[0] === 'item' && parts.length >= 8) {
        if (this.items.length < MAX_STASH_CAPACITY) {
          const name = parts.slice(8).join(' ') || 'Item do Baú';
          this.items.push({
            pool: parseInt(parts[1], 10) || 0,
            count: parseInt(parts[2], 10) || 1,
            durability: parseInt(parts[3], 10) || 100,
            itemType: parseInt(parts[4], 10) || 0,
            rank: parseInt(parts[5], 10) || 0,
            level: parseInt(parts[6], 10) || 1,
            jobClass: parseInt(parts[7], 10) || 0,
            name
          });
        }
      }
    }
  }

  isFull() {
    return this.items.length >= MAX_STASH_CAPACITY;
  }

  deposit(item) {
    if (this.isFull() || !item) return false;
    this.items.push({
      pool: item.pool || 0,
      count: item.count || 1,
      durability: item.durability || 100,
      itemType: item.itemType || 0,
      rank: item.rank || 0,
      level: item.level || 1,
      jobClass: item.jobClass || 0,
      name: item.name || 'Item do Baú'
    });
    this.save();
    return true;
  }

  withdraw(index) {
    if (index < 0 || index >= this.items.length) return null;
    const item = this.items.splice(index, 1)[0];
    this.save();
    return item;
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

    if (key === 27 || key === 10 || key === 'b' || key === 'B' || key === 'F5') { // ESC / Voltar / F5
      this.open = false;
      return true;
    }
    if (key === 'p' || key === 'P' || key === '\t') { // Alternar página
      this.page = this.page === 0 ? 1 : 0;
      return true;
    }
    return true;
  }

  handleTouch(x,y,type) {
    if(!this.open)return false;if(type!==0)return true;
    const l=this.layout();x=(x-l.x)/l.scale;y=(y-l.y)/l.scale;
    if(x>=320&&x<356&&y>=4&&y<24){this.open=false;return true;}
    if(y>=30&&y<52){if(x>=10&&x<110)this.page=0;if(x>=120&&x<220)this.page=1;return true;}
    for(let row=0;row<4;row++)for(let col=0;col<6;col++)if(x>=10+col*56&&x<64+col*56&&y>=56+row*25&&y<80+row*25){this.selectedIndex=this.page*24+row*6+col;return true;}
    if(x>=10&&x<110&&y>=184&&y<207&&this.selectedIndex<this.items.length)this.withdraw(this.selectedIndex);
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
    text(10,7,'BAU COMPARTILHADO',0xffc0,230);
    text(230,7,this.items.length+'/'+MAX_STASH_CAPACITY,0xffff,85);
    for(let i=0;i<2;i++){panel(10+i*110,30,100,22);text(15+i*110,35,'Pagina '+(i+1),this.page===i?0xffc0:0xffff,90);}
    for(let row=0;row<4;row++)for(let col=0;col<6;col++){
      const index=this.page*24+row*6+col,item=this.items[index],a=10+col*56,b=56+row*25;
      panel(a,b,54,24);text(a+4,b+3,String(index+1),index===this.selectedIndex?0xffc0:0x8c51,46);
      if(index===this.selectedIndex)ui.cursor?.(pixels,x+(a-5)*s,y+(b+5)*s);
      if(item)text(a+4,b+13,'x'+item.count,0xffff,46);
    }
    const selected=this.items[this.selectedIndex];
    text(10,160,selected?selected.name:'Slot '+(this.selectedIndex+1)+' vazio',0xffc0,340);
    if(selected)text(10,173,'Nv '+selected.level+'  Quantidade '+selected.count+'  Raridade '+selected.rank,0xffff,340);
    panel(10,184,100,22);text(15,189,'RETIRAR',selected?0xffc0:0x8c51,90);
    text(125,189,'P / TAB: pagina',0xffff,220);
    text(10,207,'Toque um slot para selecionar. B: sair',0x8c51,340);
  }
}
