/* The emulator owns its memory inside this worker. Terminating it discards saves. */
importScripts('./vendor/unicorn_arm.js');
let inventoryStorage=null;
let engine=null,inputs=null,timer=null,paused=false,booting=false,frameDelay=null,pendingLootOptions,lootFocused=true;
const fail=error=>{clearTimeout(timer);paused=true;self.postMessage({type:'error',message:String(error?.message||error)});};
async function fetchBytes(url){const response=await fetch(url);if(!response.ok)throw Error('Não foi possível carregar os arquivos do teste ('+response.status+').');return new Uint8Array(await response.arrayBuffer());}
async function start(virtualControls=true,gameMode="classico",uiScale=100,language="pt",initialLootOptions,initialLootFocused=true,protectionSnapshot){
 if(engine||booting)return;booting=true;pendingLootOptions=initialLootOptions;lootFocused=!!initialLootFocused;
 try {
  self.postMessage({type:'status',message:'Carregando o jogo…'});
  ({frameDelay}=await import('./frame-timing.mjs'));
  const [{DestiniaWeb},uc,library,pack]=await Promise.all([import('./engine.mjs'),MUnicorn(),fetchBytes('./game/libdestinia_jni.so'),fetchBytes('./game/assets.bin')]);
  const manifestLength=new DataView(pack.buffer).getUint32(0,true);if(manifestLength>200000||manifestLength+4>pack.length)throw Error('Pacote de recursos inválido.');
  const entries=JSON.parse(new TextDecoder().decode(pack.subarray(4,4+manifestLength))),assets=new Map();
  for(const entry of entries){if(typeof entry.name!=='string'||entry.offset<0||entry.size<0||4+manifestLength+entry.offset+entry.size>pack.length)throw Error('Recurso inválido.');assets.set(entry.name,pack.subarray(4+manifestLength+entry.offset,4+manifestLength+entry.offset+entry.size));}
  const digest=new Uint8Array(await crypto.subtle.digest('SHA-256',library));const hex=Array.from(digest,b=>b.toString(16).padStart(2,'0')).join('');
  if(hex!=='f12f1d20ab42646ef81df27516851acccb9fffe7be07a054db83890d992df7e9')throw Error('Biblioteca original não corresponde ao teste verificado.');
  const {InventoryStorageBridge}=await import('./inventory-storage.mjs');inventoryStorage=new InventoryStorageBridge(protectionSnapshot,message=>self.postMessage(message));
  engine=new DestiniaWeb(uc,assets,{gameMode,uiScale:100,language,lootOptions:pendingLootOptions,inventoryStorage});engine.load(library);engine.lootQol.focused=!!lootFocused;await engine.prepareCodexPortraits();await engine.loadCloudSaves();engine.init();engine.setVirtualControls(virtualControls);const {FrameInputs}=await import('./input.mjs');inputs=new FrameInputs(event=>{if(event.type==='key')engine.key(event.key,event.down);else engine.touch(event.x,event.y,event.phase);});booting=false;self.postMessage({type:'ready'});tick();
 }catch(error){fail(error);}
}
function tick(){if(!engine||paused)return;try{const t=performance.now(),wait=engine.step();inputs.afterFrame(engine.frame);if(engine.storeNotice){engine.storeNotice=false;inputs.clear();self.postMessage({type:"storeNotice"});}const pixels=engine.framebuffer();self.postMessage({type:'frame',pixels,frame:engine.frame},[pixels.buffer]);if(!wait){self.postMessage({type:'ended'});return;}timer=setTimeout(tick,frameDelay(wait,performance.now()-t));}catch(error){fail(error);}}
self.onmessage=event=>{const message=event.data;try{
 if(message.type==='start'){start(message.virtualControls!==false,message.gameMode,message.uiScale,message.language,message.lootOptions,message.lootFocused,message.inventoryProtection);return;}
 if(message.type==='inventoryProtectionSnapshot'){inventoryStorage?.refresh(message.snapshot);return;}
 if(message.type==='inventoryProtectionAck'){inventoryStorage?.acknowledge(message);return;}
 if(message.type==='lootOptions'){pendingLootOptions=message.options;if(engine?.lootQol)engine.lootQol.configure(message.options);return;}
 if(message.type==='lootFocus'){lootFocused=!!message.focused;if(engine?.lootQol)engine.lootQol.focused=lootFocused;return;}
 if(!engine||booting)return;
 if(message.type==='dropNames'){engine.dropNames.enabled=!!message.enabled&&!paused;}
 else if(message.type==='virtualControls'){if(message.preserveKeys)inputs.clearTouch();else inputs.clear();engine.setVirtualControls(message.visible);}
 else if(message.type==='pause'){engine.dropNames.enabled=false;inputs.clear();paused=!!message.value;clearTimeout(timer);self.postMessage({type:'pauseState',value:paused});if(!paused)tick();}
 else if((message.type==='touch'||message.type==='key')&&!paused){if(message.type==='touch'&&(engine.codexBarTouch(message.x,message.y)||engine.combatPower.captured||engine.combatPower.footerAt(message.x,message.y))){if(message.phase===0)inputs.clearTouch();engine.touch(message.x,message.y,message.phase);}else if(engine.inventory.open||engine.inventory.captured||engine.codexOpen||engine.codexTouchCaptured){if(message.type==='key')engine.key(message.key,message.down);else engine.touch(message.x,message.y,message.phase);}else{const wasOpen=engine.inventory.open||engine.codexOpen;inputs.handle(message,engine.frame);if(!wasOpen&&(engine.inventory.open||engine.codexOpen))inputs.clear();}}
}catch(error){fail(error);}};
