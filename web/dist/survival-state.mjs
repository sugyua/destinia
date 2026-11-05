export class CombatState {
 constructor(){this.enabled=false;this.master=false;this.immuneMs=0;this.cooldownMs=0;this.elixirMs=0;}
 tick(ms){if(!Number.isSafeInteger(ms)||ms<0)return;for(const key of ['immuneMs','cooldownMs','elixirMs'])this[key]=Math.max(0,this[key]-ms);}
 reset(){this.immuneMs=this.cooldownMs=this.elixirMs=0;this.master=false;}
 activateElixir(){this.elixirMs=300000;return true;}
 hit(hp,maxHp,damage,roll){if(!hp||!maxHp||!damage||this.immuneMs)return 0;if(this.master&&roll<1500)return 0;const resistance=(this.master?20:0)+(this.elixirMs?20:0);damage=Math.floor(damage*(100-resistance)/100);damage=Math.min(damage,Math.max(1,Math.floor(maxHp*0.6)));if(this.enabled&&!this.cooldownMs&&damage>=hp){this.immuneMs=2000;this.cooldownMs=120000;return hp-1;}return damage;}
}
