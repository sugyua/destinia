import {SaveStatus} from './save-status.mjs';
const saveStatus=new SaveStatus();
import {writeFeatureStorage} from './features-storage.mjs';
if('serviceWorker' in navigator){navigator.serviceWorker.getRegistrations().then(rs=>{for(const r of rs)r.unregister();});caches.keys().then(ks=>ks.forEach(k=>caches.delete(k)));}
import {writeInventoryProtection} from './inventory-storage.mjs';
import {lootOptions as validateLootOptions} from './loot-qol.mjs';
import {TouchVisibility,toggleFullscreen} from './display.mjs';
import {WebGamepad} from './gamepad.mjs';
import {preferredLanguage} from './language.mjs';
import {mapPoint,InputSources} from './input.mjs';
import {installKeyboard,panelShortcuts} from './keyboard.mjs';
const $=id=>document.getElementById(id),canvas=$('game'),context=canvas.getContext('2d',{alpha:false}),image=context.createImageData(800,480);
const lut=new Uint32Array(65536);for(let p=0;p<65536;p++){const r=(p>>11)&31,g=(p>>5)&63,b=p&31;lut[p]=0xff000000|(((b<<3)|(b>>2))<<16)|(((g<<2)|(g>>4))<<8)|((r<<3)|(r>>2));}
const image32=new Uint32Array(image.data.buffer);
let worker=null,ready=false,paused=false,surfacePointer=null,lastPoint={x:0,y:0},lastFrame=0,loadTimer=null,fpsFrames=0,fpsStart=0;
let language="pt";try{language=preferredLanguage(localStorage.getItem("destinia.language"));}catch{}$("language").value=language;
const uiScale=100; // Web usa apenas a escala nativa; ignora preferencia antiga sem apagar outros dados.
let gameMode="classico";try{if(localStorage.getItem("destinia.gameMode")==="expandido")gameMode="expandido";}catch{}
$("game-mode").value=gameMode;
let gamepad=null,pageFocused=document.hasFocus();
let virtualControls=true,storeWasPaused=false,storeOpen=false;
try{virtualControls=localStorage.getItem('destinia.virtualControls')!=='0';}catch{}
const touchVisibility=new TouchVisibility(virtualControls);
function updateVirtualControls(){const button=$('virtual-controls');button.textContent=virtualControls?'Ocultar botões do jogo':'Mostrar botões do jogo';button.setAttribute('aria-pressed',String(!virtualControls));}
updateVirtualControls();
const send=message=>{if(worker)worker.postMessage(message);};
window.addEventListener('storage',event=>{if(event.key==='destinia:inventory-protect-v1:'+gameMode)send({type:'inventoryProtectionSnapshot',snapshot:{text:event.newValue}});});
let lootOptions=validateLootOptions();try{const stored=localStorage.getItem('destinia.lootQol.v1');if(stored)lootOptions=validateLootOptions(JSON.parse(stored));}catch{$('loot-status').textContent='Preferências de loot inválidas ou indisponíveis; usando padrões (ouro ativo).';}
function displayLootOptions(){for(const [id,key] of [['loot-visual','visual'],['loot-gold','autoGold'],['loot-materials','autoMaterials']])$(id).checked=lootOptions[key];$('loot-rank').value=lootOptions.minRank;$('loot-radius').value=lootOptions.radius;document.querySelectorAll('[data-loot-category]').forEach(input=>input.checked=!!(lootOptions.categories&Number(input.dataset.lootCategory)));}
displayLootOptions();
for(const type of ['keydown','keyup'])$('loot-settings').addEventListener(type,event=>event.stopPropagation());
$('loot-settings').addEventListener('focusin',()=>sources.clear());
$('loot-settings').addEventListener('change',()=>{try{
 const categories=[...document.querySelectorAll('[data-loot-category]')].reduce((mask,input)=>mask|(input.checked?Number(input.dataset.lootCategory):0),0);
 const next=validateLootOptions({visual:$('loot-visual').checked,autoGold:$('loot-gold').checked,autoMaterials:$('loot-materials').checked,categories,minRank:Number($('loot-rank').value),radius:Number($('loot-radius').value)});
 lootOptions=next;send({type:'lootOptions',options:next});try{localStorage.setItem('destinia.lootQol.v1',JSON.stringify(next));$('loot-status').textContent='Preferências guardadas neste navegador.';}catch{$('loot-status').textContent='Preferências ativas só nesta sessão; armazenamento indisponível.';}
 }catch(error){$('loot-status').textContent=error.message;displayLootOptions();}});
window.addEventListener('blur',()=>send({type:'lootFocus',focused:false}));window.addEventListener('focus',()=>send({type:'lootFocus',focused:true}));

const sources=new InputSources((key,down)=>send({type:'key',key,down}));
const dropSources=new Set();
function dropSource(source,on){if(on)dropSources.add(source);else dropSources.delete(source);const enabled=dropSources.size>0;$('drop-names').setAttribute('aria-pressed',String(enabled));send({type:'dropNames',enabled});}
$('drop-names').addEventListener('click',()=>{if(ready&&!paused)dropSource('button',!dropSources.has('button'));});
window.addEventListener('keydown',event=>{if(ready&&!paused&&!event.repeat&&!event.ctrlKey&&!event.metaKey&&!event.altKey&&event.key.toLowerCase()==='v'){event.preventDefault();dropSource('keyboard',true);}});
window.addEventListener('keyup',event=>{if(event.key.toLowerCase()==='v')dropSource('keyboard',false);});
function applyTouchVisibility(){
 $('controls').hidden=!ready||!touchVisibility.siteVisible;
 document.body.classList.toggle('controller-mode',touchVisibility.controller);
 $('virtual-controls').disabled=!ready||touchVisibility.controller;
 if(ready)send({type:'virtualControls',visible:touchVisibility.gameVisible,preserveKeys:true});
}
function autoHideTouch(){if(touchVisibility.controller)return;
 for(const id of Array.from(sources.sources.keys()))if(id.startsWith('pointer:'))sources.release(id);
 document.querySelectorAll('[data-key]').forEach(b=>b.classList.remove('active'));
 if(surfacePointer!==null){send({type:'touch',...lastPoint,phase:1});surfacePointer=null;}
 touchVisibility.useGamepad();applyTouchVisibility();
}
function release(){gamepad?.suspend();dropSources.clear();dropSource('keyboard',false);sources.clear();document.querySelectorAll('[data-key]').forEach(b=>b.classList.remove('active'));if(surfacePointer!==null){send({type:'touch',...lastPoint,phase:1});surfacePointer=null;}}
function showError(message){ready=false;release();$('loading').hidden=true;$('error').hidden=false;$('error-text').textContent=message;$('status').textContent='O teste parou. Toque em Reiniciar para tentar novamente.';$('restart').disabled=false;$('pause').disabled=true;$('virtual-controls').disabled=true;$('drop-names').disabled=true;}
function setPaused(value){if(!ready)return;release();paused=value;fpsFrames=0;fpsStart=performance.now();send({type:'pause',value});$('paused').hidden=!value||storeOpen;$('pause').textContent=value?'Continuar':'Pausar';$('status').textContent=value?'Pausado · progresso somente nesta aba':'Experimental · progresso temporário · sem som';}
function boot(){
 $('save-status').textContent=saveStatus.reset();
 if(worker)worker.terminate();storeOpen=false;$('store-notice').hidden=true;release();ready=false;paused=false;surfacePointer=null;document.body.classList.add('running');$('welcome').hidden=true;$('error').hidden=true;$('paused').hidden=true;$('controls').hidden=true;$('loading').hidden=false;$('loading-text').textContent='Carregando o jogo…';$('pause').disabled=true;$('pause').setAttribute('aria-pressed','false');$('restart').disabled=true;$('virtual-controls').disabled=true;$('drop-names').disabled=true;
 worker=new Worker('./worker.js?v=0.17.2-r1');loadTimer=setTimeout(()=>{$('loading-text').textContent='Ainda carregando. Mantenha esta aba aberta…';$('restart').disabled=false;},20000);
 worker.onerror=event=>{clearTimeout(loadTimer);showError(event.message||'O navegador não conseguiu iniciar este teste.');};
 worker.onmessage=event=>{const data=event.data;
  if(data.type==='ready'){clearTimeout(loadTimer);ready=true;$('virtual-controls').disabled=false;$('drop-names').disabled=false;if($('weapons-catalog')) $('weapons-catalog').disabled=false; if($('shared-stash')) $('shared-stash').disabled=false; if($('bounties-btn')) $('bounties-btn').disabled=false; if($('difficulty-btn')) $('difficulty-btn').disabled=false;lastFrame=performance.now();fpsStart=lastFrame;fpsFrames=0;$('loading').hidden=true;applyTouchVisibility();$('pause').disabled=false;$('restart').disabled=false;$('pause').textContent='Pausar';$('status').textContent='Experimental · progresso temporário · sem som';}
  else if(data.type==='saveStatus'){const message=saveStatus.accept(data),element=$('save-status');if(element.textContent!==message)element.textContent=message;}
  else if(data.type==='featuresStorageWrite'){event.target.postMessage(writeFeatureStorage(localStorage,gameMode,data));}
  else if(data.type==='inventoryProtectionWrite'){let ack;try{ack=writeInventoryProtection(localStorage,gameMode,data);}catch(error){ack={type:'inventoryProtectionAck',id:data.id,ok:false,text:null,error:error.message};}event.target.postMessage(ack);}
  else if(data.type==='status')$('loading-text').textContent=data.message;
  else if(data.type==='frame'){lastFrame=performance.now();const b16=new Uint16Array(data.pixels.buffer,data.pixels.byteOffset,384000);for(let i=0;i<384000;i++)image32[i]=lut[b16[i]];context.putImageData(image,0,0);canvas.dataset.frame=String(data.frame);if(!paused){fpsFrames++;if(lastFrame-fpsStart>=1000){const fps=(fpsFrames*1000/(lastFrame-fpsStart)).toFixed(1);canvas.dataset.fps=fps;$('status').textContent=fps+' FPS · meta 16 · progresso temporário · sem som';fpsStart=lastFrame;fpsFrames=0;}const delay=Math.max(0,Math.max(1000/16,data.wait||60)-(data.elapsed||15));setTimeout(()=>send({type:'nextFrame'}),delay);}}
  else if(data.type==='storeNotice'){storeWasPaused=paused;storeOpen=true;setPaused(true);$('store-notice').hidden=false;$('close-store').focus();}
  else if(data.type==='pauseState')$('pause').setAttribute('aria-pressed',data.value?'true':'false');
  else if(data.type==='error'){clearTimeout(loadTimer);showError(data.message);}
  else if(data.type==='ended'){$('status').textContent='O jogo encerrou. Use Reiniciar para outro teste.';ready=false;release();}
 };
 let inventoryProtection;try{inventoryProtection={text:localStorage.getItem('destinia:inventory-protect-v1:'+gameMode)};}catch(error){inventoryProtection={error:error.message};}
 let featuresSnapshot;try{featuresSnapshot={text:localStorage.getItem('destinia:features0130:'+gameMode)};}catch(error){featuresSnapshot={error:error.message};}
 worker.postMessage({type:'start',featuresSnapshot,virtualControls:touchVisibility.gameVisible,gameMode,uiScale,language,lootOptions,inventoryProtection,lootFocused:document.hasFocus()});
}
$('virtual-controls').addEventListener('click',()=>{
 if(!ready)return;release();virtualControls=!virtualControls;
 try{localStorage.setItem('destinia.virtualControls',virtualControls?'1':'0');}catch{}
 touchVisibility.preferred=virtualControls;updateVirtualControls();applyTouchVisibility();
});
$('language').addEventListener('change',()=>{const next=$('language').value;if(next===language)return;if(ready&&!confirm('Alterar idioma reinicia o teste e perde o progresso temporário. Continuar?')){$('language').value=language;return;}language=next;try{localStorage.setItem('destinia.language',language);}catch{}if(worker)boot();});
$('game-mode').addEventListener('change',()=>{
 const next=$('game-mode').value;if(next===gameMode)return;
 if(ready&&!confirm('Trocar de modo encerra esta partida e perde o progresso temporário. Os modos não compartilham o save. Continuar?')){$('game-mode').value=gameMode;return;}
 gameMode=next;try{localStorage.setItem('destinia.gameMode',gameMode);}catch{}
 if(worker)boot();
});
$('start').addEventListener('click',boot);
$('restart').addEventListener('click',()=>{if(!ready||confirm('Reiniciar perde o progresso desta sessão. Continuar?'))boot();});
$('close-store').addEventListener('click',()=>{storeOpen=false;$('store-notice').hidden=true;setPaused(storeWasPaused);canvas.focus();});
$('pause').addEventListener('click',()=>{if(!storeOpen)setPaused(!paused);});$('resume').addEventListener('click',()=>setPaused(false));
for(const button of document.querySelectorAll('[data-key]')){
 button.addEventListener('pointerdown',event=>{if(!ready||paused)return;event.preventDefault();button.setPointerCapture(event.pointerId);sources.press('pointer:'+event.pointerId,Number(button.dataset.key));button.classList.add('active');});
 for(const type of ['pointerup','pointercancel','lostpointercapture'])button.addEventListener(type,event=>{sources.release('pointer:'+event.pointerId);button.classList.remove('active');});
 button.addEventListener('contextmenu',event=>event.preventDefault());
}
canvas.addEventListener('pointerdown',event=>{if(!ready||paused||surfacePointer!==null)return;event.preventDefault();canvas.setPointerCapture(event.pointerId);surfacePointer=event.pointerId;lastPoint=mapPoint(event.clientX,event.clientY,canvas.getBoundingClientRect());send({type:'touch',...lastPoint,phase:0});if(Number(canvas.dataset.frame||0)<80){send({type:'key',key:17,down:true});setTimeout(()=>send({type:'key',key:17,down:false}),50);}});
canvas.addEventListener('pointermove',event=>{if(event.pointerId!==surfacePointer)return;lastPoint=mapPoint(event.clientX,event.clientY,canvas.getBoundingClientRect());send({type:'touch',...lastPoint,phase:2});});
for(const type of ['pointerup','pointercancel','lostpointercapture'])canvas.addEventListener(type,event=>{if(event.pointerId!==surfacePointer)return;send({type:'touch',...lastPoint,phase:1});surfacePointer=null;});
canvas.addEventListener('contextmenu',event=>event.preventDefault());
for(const shortcut of panelShortcuts)$(shortcut.id).textContent=`${shortcut.label} (${shortcut.key})`;
installKeyboard(window,sources,()=>({ready,paused}));
window.addEventListener('blur',()=>{pageFocused=false;release();});window.addEventListener('focus',()=>{pageFocused=true;gamepad.suspend();});document.addEventListener('visibilitychange',()=>{if(document.hidden){release();setPaused(true);}});
gamepad=new WebGamepad({sources,onUse:autoHideTouch,onStatus:message=>{$('gamepad-status').textContent=message;},onPause:()=>{if(!storeOpen&&$('instructions').hidden)setPaused(!paused);},onDisconnect:()=>{touchVisibility.disconnect();applyTouchVisibility();if(ready)setPaused(true);},onAction:(action)=>{
  if(!ready||paused)return;
  if(action==='toggleWeaponsCatalog')send({type:'toggleWeaponsCatalog'});
  else if(action==='toggleSharedStash')send({type:'toggleSharedStash'});
  else if(action==='toggleBounties')send({type:'toggleBounties'});
  else if(action==='toggleQuestTracker'){send({type:'key',key:'q',down:true});setTimeout(()=>send({type:'key',key:'q',down:false}),50);}
  else if(action==='toggleAutoPotion'){send({type:'key',key:'h',down:true});setTimeout(()=>send({type:'key',key:'h',down:false}),50);}
}});
function pollGamepad(){gamepad.tick({ready,focused:pageFocused&&!document.hidden,paused,blocked:storeOpen||!$('instructions').hidden});requestAnimationFrame(pollGamepad);}
pollGamepad();
$('help').addEventListener('click',()=>{$('instructions').hidden=false;$('help').setAttribute('aria-expanded','true');setPaused(true);$('close-help').focus();});
$('close-help').addEventListener('click',()=>{$('instructions').hidden=true;$('help').setAttribute('aria-expanded','false');$('help').focus();});
setInterval(()=>{if(ready&&!paused&&performance.now()-lastFrame>15000)$('status').textContent='Processando… se não avançar, tente Reiniciar.';},5000);

function updateFullscreen(){const full=!!document.fullscreenElement,expanded=document.body.classList.contains('expanded');$('fullscreen').textContent=full?'Sair da tela cheia':expanded?'Sair da tela ampliada':document.documentElement.requestFullscreen?'Tela cheia':'Ampliar tela';$('fullscreen').setAttribute('aria-pressed',String(full||expanded));}
$('fullscreen').addEventListener('click',async()=>{try{const mode=await toggleFullscreen();if(mode==='expanded')$('display-status').textContent='Tela cheia indisponível neste navegador; modo ampliado ativado. As barras do Safari podem permanecer.';else $('display-status').textContent='';}catch{$('display-status').textContent='O navegador recusou a tela cheia. Tente novamente pelo botão.';}updateFullscreen();});
document.addEventListener('fullscreenchange',updateFullscreen);updateFullscreen();

$('weapons-catalog')?.addEventListener('click', () => { if (ready && !paused) send({ type: 'toggleWeaponsCatalog' }); });
$('shared-stash')?.addEventListener('click', () => { if (ready && !paused) send({ type: 'toggleSharedStash' }); });
$('bounties-btn')?.addEventListener('click', () => { if (ready && !paused) send({ type: 'toggleBounties' }); });
$('difficulty-btn')?.addEventListener('click', () => { if (ready && !paused) send({ type: 'toggleDifficulty' }); });

const features0130Button=document.createElement('button');features0130Button.id='features-0130';features0130Button.textContent='Arena / Refinamento (F9)';features0130Button.addEventListener('click',()=>{if(ready&&!paused){release();send({type:'toggleFeatures0130'});}});$('pause').parentElement.append(features0130Button);

window.addEventListener('keydown',event=>{if(event.key==='F9'&&ready&&!paused&&!event.repeat&&!event.ctrlKey&&!event.metaKey&&!event.altKey){event.preventDefault();release();send({type:'toggleFeatures0130'});}});
