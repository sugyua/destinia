export const panelShortcuts=Object.freeze([
 {key:'F4',id:'weapons-catalog',label:'Catálogo'},
 {key:'F5',id:'shared-stash',label:'Baú'},
 {key:'F8',id:'bounties-btn',label:'Bounties'},
]);
const keys={F6:10016,F7:10014,p:10014,P:10014,ArrowUp:3,w:3,ArrowDown:9,s:9,ArrowLeft:5,a:5,ArrowRight:7,d:7,Enter:17,' ':17,z:17,Escape:10,x:10,i:27,c:27,'1':2,'2':11,'3':4,'4':8};
for(const {key} of panelShortcuts)keys[key]=key;
export function installKeyboard(target,sources,getState){
 // Capture protects the loaded session even when an editor stops bubbling.
 target.addEventListener('keydown',event=>{
  if(event.key==='F5'&&getState().ready&&!event.ctrlKey&&!event.metaKey&&!event.altKey)event.preventDefault();
 },true);
 target.addEventListener('keydown',event=>{
  const {ready,paused}=getState();
  if(!ready||event.ctrlKey||event.metaKey||event.altKey||!Object.hasOwn(keys,event.key))return;
  if(event.target?.closest?.('input,select,textarea,[contenteditable="true"]'))return;
  // A paused game still owns panel shortcuts: F5 must not discard its progress.
  if(paused){if(panelShortcuts.some(s=>s.key===event.key))event.preventDefault();return;}
  event.preventDefault();if(event.repeat)return;
  sources.press('key:'+event.code,keys[event.key]);
 });
 target.addEventListener('keyup',event=>sources.release('key:'+event.code));
}
