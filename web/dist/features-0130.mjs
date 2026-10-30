import {Arena,wave,encode,decode} from './survival.mjs';
import {Store,tag,scaleRelevant} from './refinement.mjs';
import {stoneName} from './refinement-policy.mjs';
import {CombatState} from './survival-state.mjs';
export class Features0130 {
 constructor(engine,backend={},snapshot={}){this.engine=engine;this.backend=backend;this.arena=new Arena();this.refinement=new Store();this.combat=new CombatState();this.masterEquipped=Array.from({length:10},(_,i)=>snapshot.masterEquipped?.[i]===true);this.open=false;this.selected=0;this.message='';this.runSequence=0;if(snapshot.survival)this.arena.ledger=decode(snapshot.survival);if(snapshot.refinement)this.refinement.decode(snapshot.refinement);}
 snapshot(){return {masterEquipped:this.masterEquipped.slice(),survival:encode(this.arena.ledger),refinement:this.refinement.encode()};}
 changed(){this.backend.persist?.(this.snapshot());}
 start(slot,mapId){if(this.backend.failed||this.backend.inFlight)return false;if(this.arena.active)return false;if(!this.backend.spawnWave){this.message='ARENA: BACKEND INDISPONIVEL';return false;}if(!this.arena.start(slot,++this.runSequence,mapId))return false;const ok=this.spawn();if(ok){this.open=false;if(this.engine.bossRushUI)this.engine.bossRushUI.open=false;}return ok;}
 spawn(){try{const plan=wave(this.arena.currentWave),members=this.backend.spawnWave(plan,this.arena.runId);if(!Array.isArray(members)||members.length!==plan.count)throw Error('ONDA INCOMPLETA');for(const m of members)if(!this.arena.registerMonster(m.instance,m.species))throw Error('MONSTRO INVALIDO');this.message='ONDA '+plan.number;return true;}catch(error){this.backend.cleanupWave?.();this.arena.abort();this.message=String(error.message||error);return false;}}
 next(){if(!this.arena.beginNextWave())return false;return this.spawn();}
 kill(run,instance,map,slot){const accepted=this.arena.observeKill(run,instance,map,slot);if(accepted){if(this.arena.remaining===0)this.message=this.arena.currentWave===20?'MESTRE DA SOBREVIVENCIA LIBERADO':'ONDA CONCLUIDA';this.changed();}return accepted;}
 tick(ms,paused=false){if(paused)return;this.combat.tick(ms);if(!this.arena.active)return;const before=this.arena.ledger.slots[this.arena.slot]?.bestTimeSeconds??0;this.arena.tick(ms);if(this.arena.ledger.slots[this.arena.slot]?.bestTimeSeconds!==before)this.changed();}
 observeContext(slot,map,alive){if(this.arena.active&&(!alive||slot!==this.arena.slot||map!==this.arena.arenaMap)){this.backend.cleanupWave?.();this.arena.abort();this.message='ARENA INTERROMPIDA';}}
 abort(){this.backend.cleanupWave?.();this.arena.abort();this.message='ARENA INTERROMPIDA';this.changed();}
 settle(){const c=this.backend.context?.();if(c)this.claim(c.slot);if(this.arena.active&&this.arena.cleared&&!this.open&&!this.engine.bossRushUI?.open)this.next();}
 claim(slot){const state=this.arena.ledger.slots[slot];if(!state||!this.backend.creditRewards)return false;const reward={gold:state.pendingGold,crystals:state.pendingCrystals,cash:state.pendingCash,pure:state.pendingPure,abyssal:state.pendingAbyssal};if(!Object.values(reward).some(Boolean))return false;if(this.backend.creditRewards(reward,slot)!==true)return false;this.arena.takePending(slot);this.changed();return true;}
 upgradeSelected(){if(this.backend.inFlight)return false;if(this.arena.active){this.message='ENCERRE A ARENA ANTES DE REFINAR';return false;}if(this.backend.failed){this.message='PERSISTENCIA INDISPONIVEL';return false;}const item=this.backend.selectedItem?.();if(!item){this.message='SELECIONE ARMA OU ARMADURA';return false;}const wallet={gold:this.backend.getGold()};const q=this.refinement.upgrade(item.slot,item.pool,item.bytes,item.kind,wallet);if(q.error){this.message='REFINO: OURO/PEDRA INSUFICIENTE OU MAX +15';return false;}this.backend.setGold(wallet.gold);this.backend.recalculate?.();this.changed();this.message='REFINADO '+tag(q.level)+' - SUCESSO 100%';return true;}
 itemTag(slot,pool,bytes){return tag(this.refinement.level(slot,pool,bytes));}
 itemStat(base,slot,pool,bytes){return scaleRelevant(base,this.refinement.level(slot,pool,bytes));}
 toggle(){this.open=!this.open;return this.open;}
 handleKey(key,down){if(key==='F9'){if(down)this.toggle();return true;}if(!this.open)return false;if(!down)return true;if(key===27||key===10)this.open=false;else if(key===3)this.selected=(this.selected+7)%8;else if(key===9)this.selected=(this.selected+1)%8;else if(key===6||key===17)this.action(this.selected);return true;}
 equipMaster(slot){if(this.backend.inFlight||this.backend.failed||!this.arena.ledger.slots[slot]?.titleUnlocked)return false;this.masterEquipped[slot]=!this.masterEquipped[slot];this.backend.recalculate?.();this.changed();return true;}
 action(row){if(this.backend.inFlight)return;if(row===0){const c=this.backend.context?.();if(c)this.start(c.slot,c.map);else this.message='ENTRE NO JOGO PRIMEIRO';}if(row===1)this.next();if(row===2)this.upgradeSelected();if(row===3)this.abort();if(row===4){const c=this.backend.context?.();if(c&&!this.equipMaster(c.slot))this.message='CONCLUA A ONDA 20 PARA EQUIPAR';}if(row===5)this.combat.enabled=!this.combat.enabled;if(row===6)this.combat.activateElixir();if(row===7)this.open=false;}
 rows(){const c=this.backend.context?.(),best=c?this.arena.ledger.slots[c.slot]?.bestWave??0:0;return ['INICIAR ARENA - RECORDE '+best,'PROXIMA ONDA','REFINAR ITEM SELECIONADO','ENCERRAR / VOLTAR A CAMPANHA','MESTRE DA SOBREVIVENCIA +120%','ULTIMO SUSPIRO: '+(this.combat.enabled?'ATIVO':'DESATIVADO'),'ELIXIR: 20% MITIGACAO / 5 MIN','FECHAR'];}
 layout(){const w=this.engine.width||800,h=this.engine.height||480,s=Math.max(1,Math.floor(Math.min(w/400,h/240))),pw=Math.min(460*s,w-8*s),ph=Math.min(300*s,h-8*s),x=Math.floor((w-pw)/2),y=Math.floor((h-ph)/2),row=Math.max(12*s,Math.floor((ph-88*s)/8));return {x,y,w:pw,h:ph,scale:s,row,top:y+40*s};}
 handleTouch(x,y,phase){if(!this.open)return false;const b=this.layout();if(phase===0&&x>=b.x+4*b.scale&&x<b.x+b.w-4*b.scale&&y>=b.top&&y<b.top+b.row*8)this.action(Math.floor((y-b.top)/b.row));return true;}
 paint(pixels){
  if((!this.open&&!this.arena.active)||!this.engine.originalUi)return;
  const w=this.engine.width||800,b=this.layout(),s=b.scale,ui=this.engine.originalUi;
  const text=(x,y,value,color=0xffff,width=b.x+b.w-8*s-x)=>ui.text(pixels,x,y,value,color,Math.max(1,width),s);
  if(this.arena.active){ui.panel(pixels,{x:4*s,y:4*s,w:Math.min(440*s,w-8*s),h:22*s});text(8*s,8*s,'ONDA '+this.arena.currentWave+' RESTAM '+this.arena.remaining,0xffc0,Math.min(432*s,w-16*s));}
  if(!this.open)return;
  ui.panel(pixels,b);text(b.x+8*s,b.y+5*s,'SOBREVIVENCIA / REFINO +15',0xffc0);
  const item=this.backend.selectedItem?.(),ctx=this.backend.context?.();
  if(ctx){let summary='PEDRAS C/P/A '+[0,1,2].map(t=>this.refinement.crystals(ctx.slot,t)).join('/');if(item){const q=this.refinement.quote(item.slot,item.pool,item.bytes,item.kind);summary+=' '+tag(q.level)+' '+q.goldCost+' G';}text(b.x+8*s,b.y+21*s,summary,0xffc0);}
  this.rows().forEach((value,i)=>text(b.x+8*s,b.top+i*b.row,(i===this.selected?'> ':'  ')+value,i===this.selected?0xffff:0xce59));
  text(b.x+8*s,b.top+8*b.row+4*s,this.message,0xffc0);
  text(b.x+8*s,b.top+8*b.row+18*s,'ARMAS +8% ATK | ARMADURAS +6% DEF/+4% PV');
 }
}
