// The original ARM description frame and font; identical geometry for paint and touch.
export class QuestTrackerWeb {
 constructor(engine){this.engine=engine;this.visible=true;this.minimized=false;this.questTitle='Caça: Soldados Imperiais';this.questObjective='Abates: 0 / 15';this.destination='Estrada Imperial';}
 setQuest(title,objective,dest=''){this.questTitle=title;this.questObjective=objective;this.destination=dest;}
 toggle(){this.minimized=!this.minimized;}
 handleKey(key,down){if(!down||!['q','Q'].includes(key))return false;this.toggle();return true;}
 active(){const r=this.engine;if(!r)return false;if(r.codexOpen||r.inventory?.open||r.bossRushUI?.open||r.features0130?.open||r.weaponsCatalog?.open||r.sharedStash?.open||r.bounties?.open||r.difficulty?.open)return false;const g=r.exports?.get('Game');return !g||!r.get16||(r.get16(g+0x4854)===0&&r.get16(g+0x48f2)>0);}
 layout(){const w=this.engine?.width||400,h=this.engine?.height||240,bw=w>=800?220:174;return{x:w-bw-8,y:w>=800?10:6,w:bw,h:this.minimized?20:52};}
 handleTouch(x,y,type){if(!this.visible||!this.active()||type!==0)return false;const b=this.layout();if(x<b.x||y<b.y||x>=b.x+b.w||y>=b.y+b.h)return false;this.toggle();return true;}
 updateFromBounties(){if(!this.engine?.bounties)return;const contracts=this.engine.bounties.getContracts(this.engine.bounties.currentSlot());const active=contracts?.find(c=>!c.completed)||contracts?.[0];if(active)this.setQuest(active.title,`Abates: ${active.currentCount} / ${active.targetCount}`,active.type===1?'Boss Rush / Mapa':'Estrada Imperial');}
 paint(pixels){if(!this.visible||!this.active()||!pixels||!this.engine?.originalUi)return;this.updateFromBounties();const b=this.layout(),ui=this.engine.originalUi;ui.panel(pixels,b);ui.text(pixels,b.x+7,b.y+4,this.minimized?'MISSAO [Q]':this.questTitle,0xffc0,b.w-14);if(!this.minimized){ui.text(pixels,b.x+7,b.y+18,this.questObjective,0xffff,b.w-14);ui.text(pixels,b.x+7,b.y+32,this.destination,0x7bef,b.w-14);}}
}
