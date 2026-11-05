async function sha256Hex(bytes){
 if(typeof crypto!=='undefined'&&crypto?.subtle?.digest){
  try{
   const digest=new Uint8Array(await crypto.subtle.digest('SHA-256',bytes));
   return Array.from(digest,b=>b.toString(16).padStart(2,'0')).join('');
  }catch(_){}
 }
 const rotr=(n,x)=>(x>>>n)|(x<<(32-n)),ch=(x,y,z)=>(x&y)^(~x&z),maj=(x,y,z)=>(x&y)^(x&z)^(y&z);
 const s0=x=>rotr(2,x)^rotr(13,x)^rotr(22,x),s1=x=>rotr(6,x)^rotr(11,x)^rotr(25,x);
 const g0=x=>rotr(7,x)^rotr(18,x)^(x>>>3),g1=x=>rotr(17,x)^rotr(19,x)^(x>>>10);
 const K=[0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2];
 let H=[0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19];
 const l=bytes.length,n=((l+8)>>6)+1,padded=new Uint8Array(n<<6);padded.set(bytes);padded[l]=0x80;
 const view=new DataView(padded.buffer);view.setBigUint64(padded.length-8,BigInt(l)*8n,false);
 const W=new Uint32Array(64);
 for(let i=0;i<padded.length;i+=64){
  for(let t=0;t<16;t++)W[t]=view.getUint32(i+t*4,false);
  for(let t=16;t<64;t++)W[t]=(g1(W[t-2])+W[t-7]+g0(W[t-15])+W[t-16])>>>0;
  let [a,b,c,d,e,f,g,h]=H;
  for(let t=0;t<64;t++){
   const T1=(h+s1(e)+ch(e,f,g)+K[t]+W[t])>>>0,T2=(s0(a)+maj(a,b,c))>>>0;
   h=g;g=f;f=e;e=(d+T1)>>>0;d=c;c=b;b=a;a=(T1+T2)>>>0;
  }
  H=[H[0]+a,H[1]+b,H[2]+c,H[3]+d,H[4]+e,H[5]+f,H[6]+g,H[7]+h].map(x=>x>>>0);
 }
 return H.map(x=>x.toString(16).padStart(8,'0')).join('');
}

/* The emulator owns its memory inside this worker. Terminating it discards saves. */
importScripts('./vendor/unicorn_arm.js');
let inventoryStorage=null,featureStorage=null;
let engine=null,inputs=null,timer=null,paused=false,booting=false,frameDelay=null,pendingLootOptions,lootFocused=true;
const fail=error=>{clearTimeout(timer);paused=true;self.postMessage({type:'error',message:String(error?.message||error)});};
async function fetchBytes(url){const response=await fetch(url);if(!response.ok)throw Error('Não foi possível carregar os arquivos do teste ('+response.status+').');return new Uint8Array(await response.arrayBuffer());}
async function start(virtualControls=true,gameMode="classico",uiScale=100,language="pt",initialLootOptions,initialLootFocused=true,protectionSnapshot,featuresSnapshot){
 if(engine||booting)return;booting=true;pendingLootOptions=initialLootOptions;lootFocused=!!initialLootFocused;
 try {
  self.postMessage({type:'status',message:'Carregando o jogo…'});
  ({frameDelay}=await import('./frame-timing.mjs?v=0.13.0'));
  const [{DestiniaWeb},uc,library,pack]=await Promise.all([import('./engine.mjs?v=0.13.0'),MUnicorn(),fetchBytes('./game/libdestinia_jni.so'),fetchBytes('./game/assets.bin')]);
  const manifestLength=new DataView(pack.buffer).getUint32(0,true);if(manifestLength>200000||manifestLength+4>pack.length)throw Error('Pacote de recursos inválido.');
  const entries=JSON.parse(new TextDecoder().decode(pack.subarray(4,4+manifestLength))),assets=new Map();
  for(const entry of entries){if(typeof entry.name!=='string'||entry.offset<0||entry.size<0||4+manifestLength+entry.offset+entry.size>pack.length)throw Error('Recurso inválido.');assets.set(entry.name,pack.subarray(4+manifestLength+entry.offset,4+manifestLength+entry.offset+entry.size));}
  const hex=await sha256Hex(library);
  if(hex!=='f12f1d20ab42646ef81df27516851acccb9fffe7be07a054db83890d992df7e9')throw Error('Biblioteca original não corresponde ao teste verificado.');
  const {InventoryStorageBridge}=await import('./inventory-storage.mjs?v=0.13.0');inventoryStorage=new InventoryStorageBridge(protectionSnapshot,message=>self.postMessage(message));
  const {FeatureStorageBridge}=await import("./features-storage.mjs");featureStorage=new FeatureStorageBridge(gameMode,featuresSnapshot,message=>self.postMessage(message));
  engine=new DestiniaWeb(uc,assets,{featureStorage,gameMode,uiScale:100,language,lootOptions:pendingLootOptions,inventoryStorage});engine.load(library);engine.lootQol.focused=!!lootFocused;await engine.prepareCodexPortraits();await engine.loadCloudSaves();engine.init();engine.setVirtualControls(virtualControls);const {FrameInputs}=await import('./input.mjs?v=0.13.0');inputs=new FrameInputs(event=>{if(event.type==='key')engine.key(event.key,event.down);else engine.touch(event.x,event.y,event.phase);});booting=false;self.postMessage({type:'ready'});tick();
 }catch(error){fail(error);}
}
function tick(){if(!engine||paused)return;try{if(engine.frame%10===0&&engine.frame<=50&&engine.menuState()<=0){engine.key(17,true);engine.key(17,false);}const t=performance.now(),wait=engine.step();inputs.afterFrame(engine.frame);if(engine.storeNotice){engine.storeNotice=false;inputs.clear();self.postMessage({type:"storeNotice"});}const pixels=engine.framebuffer();const elapsed=performance.now()-t;self.postMessage({type:'frame',pixels,frame:engine.frame,wait,elapsed},[pixels.buffer]);if(!wait){self.postMessage({type:'ended'});return;}clearTimeout(timer);timer=setTimeout(tick,frameDelay(wait,elapsed));}catch(error){fail(error);}}
self.onmessage=event=>{const message=event.data;try{
 if(message.type==='nextFrame'){if(!engine||paused)return;clearTimeout(timer);tick();return;}
 if(message.type==='start'){start(message.virtualControls!==false,message.gameMode,message.uiScale,message.language,message.lootOptions,message.lootFocused,message.inventoryProtection,message.featuresSnapshot);return;}
 if(message.type==='featuresStorageAck'){featureStorage?.acknowledge(message);return;}
 if(message.type==='inventoryProtectionSnapshot'){inventoryStorage?.refresh(message.snapshot);return;}
 if(message.type==='inventoryProtectionAck'){inventoryStorage?.acknowledge(message);return;}
 if(message.type==='lootOptions'){pendingLootOptions=message.options;if(engine?.lootQol)engine.lootQol.configure(message.options);return;}
 if(message.type==='lootFocus'){lootFocused=!!message.focused;if(engine?.lootQol)engine.lootQol.focused=lootFocused;return;}
 if(!engine||booting)return;
 if(message.type==='dropNames'){engine.dropNames.enabled=!!message.enabled&&!paused;}
 else if(message.type==='virtualControls'){if(message.preserveKeys)inputs.clearTouch();else inputs.clear();engine.setVirtualControls(message.visible);}
 else if(message.type==='toggleFeatures0130'){inputs.clear();engine.features0130.toggle();}
 else if(message.type==='toggleWeaponsCatalog'){engine.weaponsCatalog?.toggle();}
 else if(message.type==='toggleSharedStash'){engine.sharedStash?.toggle();}
 else if(message.type==='toggleBounties'){engine.bounties?.toggle();}
 else if(message.type==='toggleDifficulty'){if(engine.difficulty&&!engine.features0130.arena.active)engine.difficulty.open=!engine.difficulty.open;}
 else if(message.type==='pause'){engine.dropNames.enabled=false;inputs.clear();paused=!!message.value;clearTimeout(timer);self.postMessage({type:'pauseState',value:paused});if(!paused)tick();}
 else if((message.type==='touch'||message.type==='key')&&!paused){if(message.type==='touch'&&(engine.codexBarTouch(message.x,message.y)||engine.combatPower.captured||engine.combatPower.footerAt(message.x,message.y))){if(message.phase===0)inputs.clearTouch();engine.touch(message.x,message.y,message.phase);}else if(engine.features0130?.open||engine.sharedStash?.open||engine.bounties?.open||engine.weaponsCatalog?.open||engine.difficulty?.open||engine.inventory.open||engine.inventory.captured||engine.codexOpen||engine.codexTouchCaptured){if(message.type==='key')engine.key(message.key,message.down);else engine.touch(message.x,message.y,message.phase);}else{const wasOpen=engine.inventory.open||engine.codexOpen;inputs.handle(message,engine.frame);if(!wasOpen&&(engine.inventory.open||engine.codexOpen))inputs.clear();}}
}catch(error){fail(error);}};
