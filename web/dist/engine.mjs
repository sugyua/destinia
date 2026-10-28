import {NativeFeatures0130Backend} from './features0130-native.mjs';
import {Features0130Stats} from './features0130-stats.mjs';
import {Features0130} from './features-0130.mjs';
import {DifficultyWeb} from './difficulty.mjs';
import {WeaponsCatalogWeb} from './weapons-catalog.mjs';
import {SharedStashWeb} from './shared-stash.mjs';
import {BountiesWeb} from './bounties.mjs';
import {QuestTrackerWeb} from './quest-tracker.mjs';
import {AutoPotionController} from './auto-potion.mjs';
import {MapViewport} from './map-viewport.mjs';
import {BossMage} from './boss-mage.mjs';
import {BossRushUI} from './boss-rush.mjs';
import {LootQol} from './loot-qol.mjs';
import {LootEconomy} from './loot-economy.mjs';
import {Inventory} from './inventory.mjs';
import {CombatPower} from './combat-power.mjs';
import {DropNames} from './drop-names.mjs';
import {translatedAssets} from './portuguese-text.mjs';
import {NativeCodex,nativeTransform,nativeTabHit,nativeBack as codexBack,nativePrevious as codexPrevious,nativeNext as codexNext,nativeHit as codexHitTest,nativeDetailUp as codexDetailUp,nativeDetailDown as codexDetailDown,nativeDetailsButton,nativeFilterHit} from './codex-native.mjs';
import {UiRenderer} from './ui-renderer.mjs';
import {loadMonsterMetadata,monsterDetailView} from './monster-details.mjs';
import {installFastGraphics} from './graphics-fast.mjs';
import {loadPortraits} from './codex-portraits.mjs';
import {Codex,monsterNames} from './codex.mjs';
import {contains,entriesPerPage,detailListPages} from './codex-ui.mjs';
// Experimental browser host. ARM/JNI/file ABI follows Destinia Windows 0.6.
// ARM save payloads use the existing file/cloud shim; inventory protection persists separately in browser storage.
const BASE=0x10000000,HEAP=0x20000000,HEAP_SIZE=0x4000000,STUB=0x30000000,COUNT=0x4000,RETURN=0x30100000;
const DATA=0x40000000,STACK=0x7f000000,STACK_SIZE=0x200000,ENV=DATA,ENV_TAB=DATA+0x100,VM=DATA+0x1000,VM_TAB=DATA+0x1100,GUARD=DATA+0x2000;
const encoder=new TextEncoder(),decoder=new TextDecoder();
const requireValue=(value,message)=>{if(!value)throw Error(message);};
export class DestiniaWeb {
  constructor(uc,assets,{onLog=()=>{},onSound=()=>{},fastGraphics=true,fastMemory=true,gameMode="classico",uiScale=100,language="pt",inventoryStorage,lootOptions,featureBackend,featureSnapshot,featureStorage,onOpenBossRushMenu=null,width=800,height=480}={}) {
    this.featureBackendOption=featureBackend;this.featureStorage=featureStorage;this.featureSnapshotOption=featureSnapshot;
    this.lootMaterialTable=assets.get('item_en.gdt');
    this.economy=new LootEconomy();
    if(!["en","pt"].includes(language))throw Error("Idioma inválido.");this.language=language;if(language==="pt")assets=translatedAssets(assets);
    if(!["classico","expandido"].includes(gameMode))throw Error("Modo de jogo inválido.");this.gameMode=gameMode;if(![75,100,125].includes(uiScale))throw Error("Escala de interface inválida.");this.uiScale=uiScale;
    this.onOpenBossRushMenu=onOpenBossRushMenu;this.lootOptions=lootOptions;this.inventoryStorage=inventoryStorage;this.width=width;this.height=height;this.screenBytes=width*height*2;this.codexFilter=0;
    this.pendingPurchase=false;this.storeNotice=false;this.virtualControls=true;this.fastGraphics=fastGraphics;this.fastMemory=fastMemory;this.graphicsStats={draws:0,supported:false};this.portraits=new Map();this.codexOpen=false;this.codexTabSelected=false;this.codexTouchCaptured=false;this.codexPage=0;this.codexSelection=0;this.codexDetailsOpen=false;this.codexDetailPage=0;this.codexDetailScroll=0;this.monsterMetadata=null;this.codexNativeMenuPending=-1;this.codexNativeMenuTarget=0;this.codexNativeMenuBypass=0;this.codexNativeMenuDraining=false;this.nativeTouchDown=false;this.nativeTouchPoint={x:0,y:0};
    this.codex=new Codex(monsterNames(assets.get('mon_data_en.gdt')));this.uc=uc;this.assets=assets;this.log=onLog;this.onSound=onSound;this.e=new uc.Unicorn(uc.ARCH_ARM,uc.MODE_ARM);
    this.regs=Array.from({length:13},(_,i)=>uc['ARM_REG_R'+i]);this.handlers=new Map();this.imports=new Map();this.exports=new Map();
    this.arrays=new Map();this.files=new Map();this.saved=new Map();this.nextArray=0x1000;this.nextFile=0x5000;this.nextClass=0x100;this.nextMissing=0xc00;
    this.heap=HEAP+16;this.chunks=new Map();this.freeLists=new Map();this.frame=0;this.gMenu=0;this.error=null;this.started=performance.now();
    for(const [address,size] of [[HEAP,HEAP_SIZE],[STUB,COUNT*4],[RETURN,4096],[DATA,65536],[STACK,STACK_SIZE]])this.e.mem_map(address,size,uc.PROT_ALL);
    const stubs=new Uint32Array(COUNT);stubs.fill(0xe12fff1e);this.write(STUB,new Uint8Array(stubs.buffer));this.put32(RETURN,0xe12fff1e);
    this.setupStubs();this.put32(ENV,ENV_TAB);this.put32(VM,VM_TAB);this.put32(GUARD,0xa5c3f00d);
    for(let i=0;i<235;i++){const id=0x400+i;if(!this.handlers.has(id))this.handlers.set(id,()=>{throw Error('JNI não implementado: '+i);});this.put32(ENV_TAB+i*4,STUB+id*4);}
    for(let i=0;i<8;i++){const id=0x800+i;if(!this.handlers.has(id))this.handlers.set(id,()=>{throw Error('JavaVM não implementado: '+i);});this.put32(VM_TAB+i*4,STUB+id*4);}
    this.e.hook_add(uc.HOOK_CODE,(_,address)=>{const id=(Number(address)-STUB)/4;try{const fn=this.handlers.get(id);if(!fn)throw Error('Stub desconhecido '+id);fn();}catch(error){this.error=error;this.e.emu_stop();}},null,STUB,STUB+COUNT*4-1);
  }
  r(i){return this.e.reg_read_i32(this.regs[i])>>>0;}
  w(i,value){this.e.reg_write_i32(this.regs[i],Number(value)>>>0);}
  read(address,size){
    if(!size)return new Uint8Array();
    requireValue(Number.isSafeInteger(size)&&size>0&&size<=HEAP_SIZE,'Leitura de memoria excessiva');
    if(!this.fastMemory||!this.uc.readArrayFromMemory)return this.e.mem_read(address>>>0,size);
    const p=this.uc._malloc(size);requireValue(p,'Memoria do navegador esgotada');
    try {const error=this.uc._uc_mem_read(this.uc.getValue(this.e.handle_ptr,'*'),BigInt(address>>>0),p,BigInt(size));requireValue(error===this.uc.ERR_OK,'Unicorn mem_read: '+this.uc.strerror(error));return this.uc.readArrayFromMemory(p,size);}
    finally{this.uc._free(p);}
  }
  write(address,bytes){if(bytes.length)this.e.mem_write(address>>>0,bytes);}
  get32(address){const b=this.read(address,4);return new DataView(b.buffer,b.byteOffset,4).getUint32(0,true);}
  put32(address,value){const b=new Uint8Array(4);new DataView(b.buffer).setUint32(0,Number(value)>>>0,true);this.write(address,b);}
  cstr(address,max=65536){let text=[];for(let at=address;at<address+max;){const n=Math.min(64,4096-(at&4095),address+max-at);const b=this.read(at,n);const end=b.indexOf(0);text.push(b.slice(0,end<0?n:end));if(end>=0)return decoder.decode(this.join(text));at+=n;}throw Error('String ARM sem terminador');}
  join(parts){const result=new Uint8Array(parts.reduce((n,p)=>n+p.length,0));let at=0;for(const p of parts){result.set(p,at);at+=p.length;}return result;}
  text(address,value){this.write(address,this.join([encoder.encode(value),new Uint8Array(1)]));}
  arg(i){return i<4?this.r(i):this.get32((this.e.reg_read_i32(this.uc.ARM_REG_SP)>>>0)+(i-4)*4);}
  malloc(size){size=Math.ceil(Math.max(size,1)/16)*16;requireValue(size<=HEAP_SIZE,'Alocação ARM excessiva');const list=this.freeLists.get(size);let p;if(list?.length)p=list.pop();else{requireValue(this.heap+size<=HEAP+HEAP_SIZE,'Memória do jogo esgotada');p=this.heap;this.heap+=size;}this.chunks.set(p,size);return p;}
  free(p){const size=this.chunks.get(p);if(!size)return;const list=this.freeLists.get(size)||[];list.push(p);this.freeLists.set(size,list);this.chunks.delete(p);}
  array(data){const id=this.nextArray++;this.arrays.set(id,data);return id;}
  arrText(id){return decoder.decode(this.arrays.get(id)||new Uint8Array());}
  printf(format,first){if(format==='$%d.%d'||format==='[%d& MORE]')return '';let arg=first;return format.replace(/%([-+ #0]*)(\*|\d+)?(?:\.(\*|\d+))?[hlLzjt]*([diuxXocps%])/g,(_,flags,width,precision,type)=>{
    const wide=width==='*'?this.arg(arg++):Number(width||0);const prec=precision==='*'?this.arg(arg++):precision===undefined?null:Number(precision);
    if(type==='%')return '%';const value=this.arg(arg++);let text;
    if(type==='s')text=value?this.cstr(value):'(null)';else if(type==='c')text=String.fromCharCode(value&255);else if(type==='d'||type==='i')text=String(value|0);else if(type==='p')text='0x'+value.toString(16);else text=value.toString(type==='o'?8:type==='u'?10:16);
    if(type==='X')text=text.toUpperCase();if(type==='s'&&prec!==null)text=text.slice(0,prec);
    if(wide>0&&wide<4096)text=flags.includes('-')?text.padEnd(wide,' '):text.padStart(wide,flags.includes('0')?'0':' ');return text;
  });}
  setupStubs(){
    let id=0;const L=(name,fn)=>{this.imports.set(name,id);this.handlers.set(id++,fn);};const J=(index,fn)=>this.handlers.set(0x400+index,fn);
    L('memcpy',()=>{this.write(this.r(0),this.read(this.r(1),this.r(2)));});
    L('memset',()=>{const p=this.r(0),n=this.r(2);requireValue(n<=HEAP_SIZE,'memset excessivo');this.write(p,new Uint8Array(n).fill(this.r(1)&255));if(n===0x214&&!this.gMenu)this.gMenu=p;});
    L('strlen',()=>this.w(0,encoder.encode(this.cstr(this.r(0))).length));
    L('strcpy',()=>this.text(this.r(0),this.cstr(this.r(1))));
    L('strncmp',()=>{const a=this.read(this.r(0),this.r(2)),b=this.read(this.r(1),this.r(2));let diff=0;for(let i=0;i<a.length;i++){diff=a[i]-b[i];if(diff||!a[i])break;}this.w(0,diff);});
    L('strstr',()=>{const start=this.r(0),pos=this.cstr(start).indexOf(this.cstr(this.r(1)));this.w(0,pos<0?0:start+pos);});
    L('sprintf',()=>{const p=this.r(0),s=this.printf(this.cstr(this.r(1)),2);this.text(p,s);this.w(0,encoder.encode(s).length);});
    L('malloc',()=>this.w(0,this.malloc(this.r(0))));L('free',()=>this.free(this.r(0)));
    L('clock',()=>this.w(0,Math.floor((performance.now()-this.started)*1000)));
    L('fopen',()=>{const path=this.cstr(this.r(0)),mode=this.cstr(this.r(1));let bytes=this.saved.get(path);if(mode.includes('w'))bytes=new Uint8Array();if(!bytes&&mode.includes('a'))bytes=new Uint8Array();if(!bytes){this.w(0,0);return;}this.saved.set(path,bytes);const h=this.nextFile;this.nextFile+=16;this.files.set(h,{path,pos:mode.includes('a')?bytes.length:0,mode});this.w(0,h);});
    L('fclose',()=>{const f=this.files.get(this.r(0));if(f&&(f.mode.includes('w')||f.mode.includes('a'))){this.persistCloudSave(f.path);}this.files.delete(this.r(0));this.w(0,0);});
    L('fread',()=>{const f=this.files.get(this.r(3)),size=this.r(1),count=this.r(2);if(!f||!size||!count){this.w(0,0);return;}requireValue(size*count<=HEAP_SIZE,'fread excessivo');const bytes=this.saved.get(f.path);const n=Math.min(size*count,Math.max(0,bytes.length-f.pos));this.write(this.r(0),bytes.slice(f.pos,f.pos+n));f.pos+=n;this.w(0,Math.floor(n/size));});
    L('fwrite',()=>{const f=this.files.get(this.r(3)),size=this.r(1),count=this.r(2);if(!f||!size||!count){this.w(0,0);return;}const n=size*count;requireValue(n<=HEAP_SIZE&&f.pos+n<=HEAP_SIZE,'fwrite excessivo');const old=this.saved.get(f.path),bytes=new Uint8Array(Math.max(old.length,f.pos+n));bytes.set(old);bytes.set(this.read(this.r(0),n),f.pos);this.saved.set(f.path,bytes);f.pos+=n;this.w(0,count);});
    L('fseek',()=>{const f=this.files.get(this.r(0));if(!f){this.w(0,-1);return;}const origin=this.r(2),offset=this.r(1)|0;const pos=(origin===0?0:origin===1?f.pos:this.saved.get(f.path).length)+offset;if(pos<0||pos>HEAP_SIZE){this.w(0,-1);return;}f.pos=pos;this.w(0,0);});
    L('ftell',()=>this.w(0,this.files.get(this.r(0))?.pos??-1));L('remove',()=>{const path=this.cstr(this.r(0)),ok=this.saved.delete(path),match=path.match(/(?:^|\/)save([0-9])\.dat$/);if(ok&&match)this.codex.erase(Number(match[1]));this.w(0,ok?0:-1);});
    L('__android_log_print',()=>this.log(this.printf(this.cstr(this.r(2)),3)));L('__stack_chk_fail',()=>{throw Error('Pilha ARM corrompida');});L('__aeabi_unwind_cpp_pr0',()=>{});L('__aeabi_unwind_cpp_pr1',()=>{});
    for(const [name,signed,remainder] of [['__aeabi_idiv',true,false],['__aeabi_uidiv',false,false],['__aeabi_idivmod',true,true],['__aeabi_uidivmod',false,true]])L(name,()=>{const a=signed?this.r(0)|0:this.r(0),b=signed?this.r(1)|0:this.r(1);this.w(0,b?Math.trunc(a/b):0);if(remainder)this.w(1,b?a%b:0);});
    L('__aeabi_lmul',()=>{const a=(BigInt(this.r(1))<<32n)|BigInt(this.r(0)),b=(BigInt(this.r(3))<<32n)|BigInt(this.r(2)),v=BigInt.asUintN(64,a*b);this.w(0,Number(v&0xffffffffn));this.w(1,Number(v>>32n));});
    L('__aeabi_ldivmod',()=>{const a=BigInt.asIntN(64,(BigInt(this.r(1))<<32n)|BigInt(this.r(0))),b=BigInt.asIntN(64,(BigInt(this.r(3))<<32n)|BigInt(this.r(2)));const q=b?a/b:0n,r=b?a%b:0n;this.w(0,Number(q&0xffffffffn));this.w(1,Number(BigInt.asUintN(64,q)>>32n));this.w(2,Number(r&0xffffffffn));this.w(3,Number(BigInt.asUintN(64,r)>>32n));});
    J(6,()=>this.w(0,this.nextClass++));J(21,()=>this.w(0,this.r(1)||1));
    const methods={playVib:1,gotoURL:2,getAssetResSize:3,getAssetRes:4,playSound:5,setSoundVolume:6,close:7,buyCash:8,logEvent:9};
    J(113,()=>this.w(0,methods[this.cstr(this.r(2))]||0));J(171,()=>this.w(0,this.arrays.get(this.r(1))?.length||0));
    J(176,()=>{const n=this.r(1);requireValue(n<=HEAP_SIZE,'Array JNI excessivo');this.w(0,this.array(new Uint8Array(n)));});
    J(184,()=>{const data=this.arrays.get(this.r(1));if(!data){this.w(0,0);return;}const p=this.malloc(data.length);this.write(p,data);if(this.r(2))this.put32(this.r(2),1);this.w(0,p);});
    J(192,()=>{const data=this.arrays.get(this.r(1)),p=this.r(2),mode=this.r(3);if(data&&mode!==2)data.set(this.read(p,data.length));if(mode!==1)this.free(p);});
    J(200,()=>{const data=this.arrays.get(this.r(1)),start=this.r(2),len=this.r(3);if(data&&start+len<=data.length)this.write(this.arg(4),data.subarray(start,start+len));});
    J(208,()=>{const data=this.arrays.get(this.r(1)),start=this.r(2),len=this.r(3);if(data&&start+len<=data.length)data.set(this.read(this.arg(4),len),start);});
    J(219,()=>{this.put32(this.r(1),VM);this.w(0,0);});
    J(129,()=>{const nm=this.arrText(this.r(3));this.w(0,this.r(2)===3?(this.assets.get(nm)?.length||0):0);});
    J(114,()=>{const nm=this.arrText(this.r(3)),bytes=this.r(2)===4?this.assets.get(nm):null;if(!bytes){this.w(0,0);return;}this.w(0,this.array(bytes.slice()));});
    J(141,()=>{const method=this.r(2);if(method===5)this.onSound(this.r(3));else if(method===6)this.onSound(-1,this.r(3),this.arg(5));else if(method===8){this.pendingPurchase=true;this.log('Compra indisponível; cancelamento offline agendado.');}});
    for(const i of [4,6])this.handlers.set(0x800+i,()=>{this.put32(this.r(1),ENV);this.w(0,0);});
  }
  resolve(name){if(name==='__stack_chk_guard')return GUARD;let id=this.imports.get(name);if(id===undefined){id=this.nextMissing++;requireValue(id<COUNT,'Imports em excesso');this.imports.set(name,id);this.handlers.set(id,()=>{throw Error('Import ARM não implementado: '+name);});}return STUB+id*4;}
  load(file){
    requireValue(file.length>=52&&file.length<=64*1024*1024,'ELF: tamanho inválido');const v=new DataView(file.buffer,file.byteOffset,file.length);
    requireValue(v.getUint32(0,true)===0x464c457f&&file[4]===1&&file[5]===1&&v.getUint16(18,true)===40&&v.getUint16(16,true)===3,'ELF: esperada biblioteca ARM32');
    const phoff=v.getUint32(28,true),stride=v.getUint16(42,true),number=v.getUint16(44,true);requireValue(stride===32&&number>0&&phoff+stride*number<=file.length,'ELF: segmentos inválidos');
    let span=0,dynamic=null;const segments=[];
    for(let i=0;i<number;i++){const at=phoff+i*stride,type=v.getUint32(at,true),offset=v.getUint32(at+4,true),address=v.getUint32(at+8,true),size=v.getUint32(at+16,true),memory=v.getUint32(at+20,true);if(type!==1&&type!==2)continue;requireValue(size<=memory&&offset+size<=file.length&&address+memory<=64*1024*1024,'ELF: segmento fora dos limites');if(type===1){segments.push({offset,address,size,memory});span=Math.max(span,address+memory);}else{requireValue(!dynamic,'ELF: tabela dinâmica ambígua');dynamic={address,size};}}
    requireValue(span>0&&dynamic,'ELF: metadados ausentes');span=Math.ceil(span/4096)*4096;const image=new Uint8Array(span),d=new DataView(image.buffer);for(const s of segments)image.set(file.subarray(s.offset,s.offset+s.size),s.address);
    const range=(at,n)=>{requireValue(Number.isSafeInteger(at)&&Number.isSafeInteger(n)&&n>=0&&segments.some(s=>at>=s.address&&at+n<=s.address+s.memory),'ELF: tabela fora dos limites');};
    range(dynamic.address,dynamic.size);const tags=new Map();let terminated=false;for(let at=dynamic.address;at<dynamic.address+dynamic.size;at+=8){range(at,8);const tag=d.getInt32(at,true);if(!tag){terminated=true;break;}tags.set(tag,d.getUint32(at+4,true));}requireValue(terminated,'ELF: tabela dinâmica sem terminador');
    for(const tag of [4,5,6,10])requireValue(tags.has(tag),'ELF: tabela ausente');const hash=tags.get(4),str=tags.get(5),sym=tags.get(6),strsz=tags.get(10);range(hash,8);const count=d.getUint32(hash+4,true);requireValue(count>0&&count<=65536,'ELF: símbolos excessivos');range(sym,count*16);range(str,strsz);
    const symbols=[];for(let i=0;i<count;i++){const at=sym+i*16,nameOff=d.getUint32(at,true);requireValue(nameOff<strsz,'ELF: nome inválido');const end=image.indexOf(0,str+nameOff);requireValue(end>=str+nameOff&&end<str+strsz&&end-str-nameOff<=4096,'ELF: string sem terminador');const name=decoder.decode(image.subarray(str+nameOff,end)),value=d.getUint32(at+4,true),defined=d.getUint16(at+14,true)!==0;symbols.push({name,value,defined});if(defined&&name)this.exports.set(name,BASE+value);}
    const relocations=[];for(const [offTag,sizeTag] of [[17,18],[23,2]]){const at=tags.get(offTag)||0,n=tags.get(sizeTag)||0;if(!n)continue;range(at,n);requireValue(n%8===0,'ELF: relocações inválidas');for(let pos=at;pos<at+n;pos+=8)relocations.push([d.getUint32(pos,true),d.getUint32(pos+4,true)]);}
    for(const [target,info] of relocations){range(target,4);const type=info&255,index=info>>>8;let value=d.getUint32(target,true);if(type===23)value+=BASE;else if([2,21,22].includes(type)){requireValue(index<symbols.length,'ELF: símbolo inválido');const s=symbols[index],address=s.defined?BASE+s.value:this.resolve(s.name);value=type===2?value+address:address;}else if(type===0)continue;else throw Error('ELF: relocação não suportada '+type);d.setUint32(target,value>>>0,true);}
    this.e.mem_map(BASE,span,this.uc.PROT_ALL);this.write(BASE,image);this.log('Biblioteca ARM carregada: '+this.exports.size+' símbolos.');const featureBackend=this.featureBackendOption||new NativeFeatures0130Backend(this,{storage:this.featureStorage});this.features0130=new Features0130(this,featureBackend,this.featureSnapshotOption||featureBackend.snapshot);featureBackend.attach?.(this.features0130);this.featureStats0130=new Features0130Stats(this);this.installCodex();this.installVirtualControls();this.dropNames=new DropNames(this);this.dropNames.install();this.lootQol=new LootQol(this,this.lootOptions);this.lootQol.install();this.combatPower=new CombatPower(this);this.combatPower.install();this.inventory=new Inventory(this);this.inventory.install();this.uiRenderer=new UiRenderer(this,this.uiScale);this.uiRenderer.install();if(this.fastGraphics)this.graphicsStats=installFastGraphics(this);this.nativeCodex=new NativeCodex(this);this.bossRushUI=new BossRushUI(this);this.bossMage=new BossMage(this,this.onOpenBossRushMenu||(()=>this.bossRushUI.openMenu()));this.bossMage.install();this.mapViewport=new MapViewport(this);this.mapViewport.install();this.difficulty=new DifficultyWeb(this);this.difficulty.install();this.featureStats0130.install();this.weaponsCatalog=new WeaponsCatalogWeb(this);this.sharedStash=new SharedStashWeb(this);this.bounties=new BountiesWeb(this);this.questTracker=new QuestTrackerWeb(this);this.autoPotion=new AutoPotionController(this);
  }
  finishOfflinePurchase(){
    if(!this.pendingPurchase)return;
    this.pendingPurchase=false;
    const before=this.call(this.exports.get('getMyCash')),comm=this.exports.get('Comm');
    if(this.get32(comm)===this.exports.get('Menu_netProc')&&this.get32(comm+28)===30)
      this.call(this.exports.get('Java_game_destiniaeng_GameThread_procCommMsg'),[ENV,16,1,0xfffffff6]);
    this.call(this.exports.get('MMain_setNetState'),[0]);
    requireValue(this.call(this.exports.get('getMyCash'))===before&&this.get16(this.exports.get('MMain')+14624)===0,'Falha no cancelamento offline.');
    this.storeNotice=true;this.log('Loja offline: saldo preservado; rede liberada.');
  }
  replyComm(cmd,payload){
    const b=new Uint8Array(payload.length+8);
    const v=new DataView(b.buffer);
    v.setUint32(0,payload.length+4,false);
    v.setUint32(4,cmd,false);
    b.set(payload,8);
    const arr=this.array(b);
    this.call(this.exports.get('Java_game_destiniaeng_GameThread_procCommMsg'),[ENV,16,1,0]);
    this.call(this.exports.get('Java_game_destiniaeng_GameThread_procCommRead'),[ENV,16,arr,b.length]);
    for(let i=0;i<4;i++)this.call(this.exports.get('Comm_mRun'));
  }
  persistCloudSave(filePath){
    const fileName=filePath.split('/').pop().split('\\').pop();
    if(!fileName.includes('save')&&!fileName.includes('msave')&&!fileName.includes('codex'))return;
    const bytes=this.saved.get(filePath)||this.saved.get(fileName);
    if(!bytes||typeof fetch!=='function')return;
    try{
      let b64='';
      for(let i=0;i<bytes.length;i++)b64+=String.fromCharCode(bytes[i]);
      b64=btoa(b64);
      const base=(typeof location!=='undefined'&&location.origin)?location.origin:'http://localhost:8000';
      fetch(base+'/api/save',{
        method:'POST',
        headers:{'Content-Type':'application/json'},
        body:JSON.stringify({name:fileName,data:b64})
      }).then(r=>r.json()).then(res=>{
        if(res?.ok)this.log('Save sincronizado com Google Drive: '+fileName);
      }).catch(err=>{
        this.log('Aviso Sync Drive: '+(err?.message||err));
      });
    }catch(_){}
  }
  async loadCloudSaves(){
    if(typeof fetch!=='function')return;
    try{
      const base=(typeof location!=='undefined'&&location.origin)?location.origin:'http://localhost:8000';
      const res=await fetch(base+'/api/save?format=json');
      if(!res.ok)return;
      const json=await res.json();
      if(!json?.ok||!json.saves)return;
      for(const [file,b64] of Object.entries(json.saves)){
        const bin=Uint8Array.from(atob(b64),c=>c.charCodeAt(0));
        this.saved.set(file,bin);
        this.saved.set('/data/data/game.destiniaeng/files/'+file,bin);
        this.saved.set('./'+file,bin);
      }
      this.log('Saves carregados do Google Drive: '+Object.keys(json.saves).join(', '));
    }catch(err){
      this.log('Drive sync indisponivel: '+(err?.message||err));
    }
  }
  processNetwork(){
    const comm=this.exports.get('Comm');if(!comm)return;
    const pending=this.get32(comm+0x2f90);if(!pending)return;
    const bytes=this.read(comm+52,pending);
    this.put32(comm+0x2f90,0);
    if(pending<8)return;
    const v=new DataView(bytes.buffer,bytes.byteOffset,bytes.length);
    const cmd=v.getUint32(4,false);
    if(cmd===0){
      const p0=new Uint8Array([0,0,0,1,0,0,0,8,0x44,0x45,0x53,0x54,0x49,0x4e,0x49,0x41,0,0,0,2,0x4f,0x4b,0,0,0,0]);
      this.replyComm(0,p0);
      try{
        const g=this.exports.get('Game'),job=g?this.get16(g+0x4886):0;
        const name=job===0?'Guerreiro':job===1?'Atirador':'Ladrão';
        if(typeof fetch==='function'){
          const eps=['http://localhost:3000/api/onJoin','http://127.0.0.1:3000/api/onJoin','/api/onJoin'];
          (async()=>{for(const u of eps){try{const r=await fetch(u,{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({name,room:'sala1'})});if(r.ok)break;}catch(_){}}})();
        }
      }catch(_){}
    }else if(cmd===8){
      const p8=new Uint8Array([0,0,0,1,0,0]);
      this.replyComm(8,p8);
    }
  }
  setVirtualControls(visible){
    requireValue(typeof visible==='boolean','Preferência de controles inválida.');
    if(this.nativeTouchDown){const {x,y}=this.nativeTouchPoint;this.call(this.exports.get('Java_game_destiniaeng_GameThread_jniTouch'),[ENV,16,x,y,1]);this.nativeTouchDown=false;}
    this.virtualControls=visible;
  }
  installVirtualControls(){
    // Original APK code only: preserve menu input and transition/state epilogue.
    // Animation frames for hidden touch buttons are intentionally omitted too.
    const sites=[{at:0xff7c,bytes:[159,75,85,70,2,61],reg:10,end:0x1031f},{at:0xfa3e,bytes:[1,144,60,104,3,28],reg:0,end:0xfb0b}];
    for(const site of sites)requireValue(this.read(BASE+site.at,site.bytes.length).every((b,i)=>b===site.bytes[i]),'Biblioteca incompatível com controles opcionais.');
    for(const site of sites)this.e.hook_add(this.uc.HOOK_CODE,()=>{
      try{
        if(this.virtualControls)return;
        const state=this.r(site.reg);if(state!==2&&state!==3)return;
        if(site.reg===0)this.w(0,0);
        this.e.reg_write_i32(this.uc.ARM_REG_PC,BASE+site.end);
      }catch(error){this.error=error;this.e.emu_stop();}
    },null,BASE+site.at,BASE+site.at);
  }
  get16(at,signed=false){const bytes=this.read(at,2),v=new DataView(bytes.buffer,bytes.byteOffset,2);return signed?v.getInt16(0,true):v.getUint16(0,true);}
  installCodex(){
    const game=this.exports.get('Game');if(game!==BASE+0x77ee0)throw Error('Layout do Códex incompatível.');
    {const at=this.exports.get('Grp_drawWImage')&~1,menu=this.exports.get('GameUI_drawMainMenu')&~1;this.e.hook_add(this.uc.HOOK_CODE,()=>{
      const lr=this.e.reg_read_i32(this.uc.ARM_REG_LR)>>>0;
      if(this.r(3)===this.exports.get('MMain')+12&&this.r(2)<40&&(lr&~1)>=menu&&(lr&~1)<menu+8400)this.e.reg_write_i32(this.uc.ARM_REG_PC,lr);
    },null,at,at);}
    const symbols=['Mon_initMonster','drawMonsterOddEff','checkQuestMonster','Mon_createMonster','GameCanvas_LoadGame'];
    symbols.forEach((name,kind)=>{const at=this.exports.get(name)&~1;requireValue(at>=BASE,'Rotina do Códex ausente');this.e.hook_add(this.uc.HOOK_CODE,()=>{
      if(kind===4){this.codex.select(this.r(0));this.codex.newMap();return;}
      const slot=this.get16(game+0x4886);if(slot>9)return;this.codex.select(slot);const p=this.r(0);
      if(kind===3){this.codex.newMap();return;}if(kind===0){this.codex.respawn(p);return;}
      const id=this.get16(p+8),hp=this.get32(p+0x20)|0;
      // Normal procMonster notifies quests before it writes death state 2.
      if(kind===2){
        if(this.features0130?.arena.active){
          const arena=this.features0130.arena,backend=this.features0130.backend,c=backend.context?.();
          if(hp<=0&&c)this.features0130.kill(arena.runId,backend.instanceId?.(p),c.map,c.slot);
          this.e.reg_write_i32(this.uc.ARM_REG_PC,this.e.reg_read_i32(this.uc.ARM_REG_LR));
          return;
        }
        if(hp<=0){
          this.codex.observe(p,id,true,this.monsterFacts(p));
          this.bounties?.onMonsterKilled(slot,id);
          if(id>=100&&id<=109){
            this.economy.recordBossKill(slot,id);
            if(typeof localStorage!=='undefined'){
              try{localStorage.setItem('destinia:economy-v1:'+this.gameMode,this.economy.encode());}catch(_){}
            }
          }
        }
        return;
      }
      const x=this.get16(p+0x1a,true)-this.get16(game+0x4878,true),y=this.get16(p+0x1c,true)-this.get16(game+0x487a,true);
      if(x>=0&&x<400&&y>=0&&y<240)this.codex.observe(p,id,false,this.monsterFacts(p));
    },null,at,at);});
    for(const name of ['GameCanvas_mKeyPressed','MainMenu_mKeyPressed']){
     const target=this.exports.get(name),menu=target&~1;
     this.e.hook_add(this.uc.HOOK_CODE,()=>{try{
      if(this.codexNativeMenuBypass||this.codexNativeMenuDraining||!this.codexMenuVisible())return;
      const raw=this.r(0),key=raw>=13&&raw<=16?[3,9,5,7][raw-13]:raw,state=this.get32(this.exports.get('GameUI')+0x7c);
      if(!(this.codexOpen||this.codexTabSelected||key===5&&state===0||key===7&&(state===0x80000000||state===0x80000001)))return;
      if(this.codexNativeMenuPending<0){this.codexNativeMenuPending=raw;this.codexNativeMenuTarget=target;}
      this.e.reg_write_i32(this.uc.ARM_REG_PC,this.e.reg_read_i32(this.uc.ARM_REG_LR));
     }catch(error){this.error=error;this.e.emu_stop();}},null,menu,menu);
    }
  }
  codexDrainNativeMenu(){
    if(this.codexNativeMenuPending<0||this.codexNativeMenuDraining)return;
    const key=this.codexNativeMenuPending,target=this.codexNativeMenuTarget;this.codexNativeMenuPending=-1;this.codexNativeMenuDraining=true;
    try{if(!this.codexMenuKey(key,true)){this.codexNativeMenuBypass++;try{this.call(target,[key]);}finally{this.codexNativeMenuBypass--;}}}finally{this.codexNativeMenuDraining=false;}
  }
  call(address,args=[]){requireValue(address,'Função ARM ausente');let sp=(STACK+STACK_SIZE-256-Math.max(0,args.length-4)*4)&~7;for(let i=4;i<args.length;i++)this.put32(sp+(i-4)*4,args[i]);for(let i=0;i<4;i++)this.w(i,args[i]||0);this.e.reg_write_i32(this.uc.ARM_REG_SP,sp);this.e.reg_write_i32(this.uc.ARM_REG_LR,RETURN);this.error=null;try{this.e.emu_start(address,RETURN,0,0);}catch(error){const pc=this.e.reg_read_i32(this.uc.ARM_REG_PC)>>>0;throw Error(String(error)+" [ARM pc=0x"+(pc-BASE).toString(16)+", UI="+(this.uiRenderer?.kind||"none")+", entry="+(this.uiRenderer?.entry||0).toString(16)+"]");}if(this.error)throw this.error;requireValue((this.e.reg_read_i32(this.uc.ARM_REG_PC)&~1)===RETURN,'Execução ARM interrompida');const result=this.r(0);this.bossMage?.dispatch();this.codexDrainNativeMenu();return result;}
  init(){const device=encoder.encode('DESTINIA-WEB-TEST'),model=encoder.encode('Browser');const id=this.array(device),m=this.array(model);this.screen=this.array(new Uint8Array(this.screenBytes));this.call(this.exports.get('Java_game_destiniaeng_GameThread_initGame'),[ENV,16,id,device.length,m,model.length,this.width,this.height,0]);this.log('initGame concluído.');
    if(typeof localStorage!=='undefined'){
      try{
        const text=localStorage.getItem('destinia:economy-v1:'+this.gameMode);
        if(text)this.economy.decode(text);
      }catch(_){}
    }}
  step(){this.finishOfflinePurchase();this.processNetwork();if(this.features0130.open||this.features0130.backend.inFlight||this.bossRushUI?.open){this.features0130.tick(62,true);this.frame++;return 62;}if(this.codexOpen){this.features0130.tick(62,true);this.frame++;return 62;}if(this.inventory.open){this.features0130.tick(62,true);this.frame++;return 62;}this.lootQol.collect(this.frame);this.inventory.beginFrame();this.combatPower.beginFrame();const wait=this.call(this.exports.get('Java_game_destiniaeng_GameThread_jniRun'),[ENV,16,this.screen,this.frame++]);this.features0130.tick(Math.min(1000,Math.max(0,wait)),this.get16(this.exports.get('Game')+0x4854)!==0);this.finishOfflinePurchase();this.processNetwork();return wait>=1000?100:wait;}
  monsterFacts(p){const game=this.exports.get('Game'),file=`m${this.get16(game+0x4870)}_${this.get16(game+0x4872)}.mpd`;return {maxHP:this.get32(p+0x24),level:this.get16(p+0x10),elite:!!this.get32(p+0x4c),location:{file,name:this.monsterMetadata?.maps.get(file)||file}};}
  async prepareCodexPortraits(){this.portraits=await loadPortraits(this.assets,this.codex.names);try{this.monsterMetadata=await loadMonsterMetadata(this.assets);}catch(error){this.log(error.message);}}
  codexDetailData(){const row=this.codex.catalog(this.codexFilter)[this.codexSelection];return monsterDetailView(row,this.monsterMetadata,this.codex.facts(row?.id));}
  codexDetailsPage(direction){const pages=Math.max(1,this.codexDetailData()?.sections?.length||1),next=Math.max(0,Math.min(pages-1,this.codexDetailPage+direction));if(next!==this.codexDetailPage)this.codexDetailScroll=0;this.codexDetailPage=next;}
  codexScrollDetails(direction){const lists=detailListPages(this.codexDetailData()?.sections?.[this.codexDetailPage]).length;this.codexDetailScroll=Math.max(0,Math.min(lists-1,this.codexDetailScroll+direction));}

  framebuffer(){this.features0130.settle();this.features0130.backend.flush?.();const featureContext=this.features0130.backend.context?.();if(featureContext)this.features0130.observeContext(featureContext.slot,featureContext.map,featureContext.alive!==false);const pixels=this.arrays.get(this.screen).slice();if(!this.codexOpen)this.combatPower.paint(pixels);this.autoPotion?.update();this.bossMage.paint(pixels);this.bossRushUI.paint(pixels);this.lootQol.paint(pixels);this.inventory.paint(pixels);this.nativeCodex.paint(pixels);this.difficulty.paint(pixels);this.weaponsCatalog.paint(pixels);this.sharedStash?.paint(pixels);this.bounties?.paint(pixels);this.questTracker?.paint(pixels);this.autoPotion?.paintNotice(pixels,this.width,this.height);this.features0130.paint(pixels);return pixels;}
  codexMenuVisible(){const game=this.exports.get('Game'),ui=this.exports.get('GameUI');if(!game||!ui)return false;const state=this.get32(ui+0x7c);return this.get16(game+0x4854)===16&&this.get16(ui+0x76)>=2&&(state&0x0fffffff)<=1&&(state>>>28)<=8;}
  codexChangePage(direction){const count=this.codex.catalog(this.codexFilter).length,page=Math.max(0,Math.min(Math.max(0,Math.ceil(count/entriesPerPage)-1),this.codexPage+direction));this.codexSelection=Math.min(count-1,page*entriesPerPage+this.codexSelection%entriesPerPage);this.codexPage=page;}
  codexMoveSelection(delta){const count=this.codex.catalog(this.codexFilter).length;this.codexSelection=Math.max(0,Math.min(count-1,this.codexSelection+delta));this.codexPage=Math.floor(this.codexSelection/entriesPerPage);}
  codexSelectNativeTab(tab){this.codexNativeMenuBypass++;try{const u=this.exports.get('GameUI');if((this.get32(u+0x7c)&0x0fffffff)===1)this.call(this.exports.get('MainMenu_mKeyPressed'),[27]);if((this.get32(u+0x7c)&0x0fffffff)!==0)return false;for(let n=0;this.get32(u+0x7c)>>>28!==tab&&n<9;n++)this.call(this.exports.get('MainMenu_mKeyPressed'),[7]);return this.get32(u+0x7c)>>>28===tab;}finally{this.codexNativeMenuBypass--;}}
  codexOpenMenu(){if(this.nativeTouchDown){const {x,y}=this.nativeTouchPoint;this.call(this.exports.get('Java_game_destiniaeng_GameThread_jniTouch'),[ENV,16,x,y,1]);this.nativeTouchDown=false;}const slot=this.get16(this.exports.get('Game')+0x4886);if(slot<=9)this.codex.select(slot);this.codexOpen=true;this.codexDetailsOpen=false;this.codexDetailPage=0;this.codexDetailScroll=0;this.codexTabSelected=true;this.codexFilter=0;this.codex.setFilter(0);const first=this.codex.catalog(0).findIndex(row=>row.found);this.codexSelection=Math.max(0,first);this.codexPage=Math.floor(this.codexSelection/entriesPerPage);}
  codexMenuKey(key,down){
   if(!down)return false;
   if(key>=13&&key<=16)key=[3,9,5,7][key-13];
   if(this.codexOpen){
    if(!this.codexDetailsOpen&&(key===10016||key===4)){this.codexFilter=(this.codexFilter+1)%3;this.codex.setFilter(this.codexFilter);this.codexPage=0;this.codexSelection=0;return true;}
    if(this.codexDetailsOpen){if(key===10||key===27){this.codexDetailsOpen=false;}else if(key===5)this.codexDetailsPage(-1);else if(key===7||key===17||key===6)this.codexDetailsPage(1);else if(key===3)this.codexScrollDetails(-1);else if(key===9)this.codexScrollDetails(1);return true;}
    if(key===17||key===6){this.codexDetailsOpen=true;this.codexDetailPage=0;this.codexDetailScroll=0;return true;}
    if(key===10||key===27){this.codexOpen=false;this.codexTabSelected=true;}else if(key===5)this.codexMoveSelection(-1);else if(key===7)this.codexMoveSelection(1);else if(key===3)this.codexMoveSelection(-8);else if(key===9)this.codexMoveSelection(8);return true;}
   if(!this.codexMenuVisible()){this.codexTabSelected=false;return false;}
   const ui=this.exports.get('GameUI'),state=this.get32(ui+0x7c);
   if(this.codexTabSelected){
    if([17,6,9].includes(key)){this.codexOpenMenu();return true;}
    if(key===5||key===7){this.codexTabSelected=false;this.codexSelectNativeTab(key===5?8:0);return true;}
    if(key===10||key===27){this.codexTabSelected=false;return false;}return true;
   }
   if(key===5&&state===0||key===7&&(state===0x80000000||state===0x80000001)){if(state===0x80000001&&!this.codexSelectNativeTab(8))return false;this.codexOpenMenu();this.codexOpen=false;return true;}return false;
  }
  codexBarTouch(x,y){const p=nativeTransform(this.uiScale,this.width,this.height).inverse(x,y);return this.codexMenuVisible()&&nativeTabHit(p.x,p.y,this.width,this.height)>=0;}
  codexMenuTouch(x,y,phase){
    const point=nativeTransform(this.uiScale,this.width,this.height).inverse(x,y);x=point.x-Math.floor((this.width-400)/2);y=point.y-Math.floor((this.height-240)/2);
   if(this.codexTouchCaptured&&phase!==0){if(phase===1)this.codexTouchCaptured=false;return true;}
   if(this.codexOpen&&this.nativeTouchDown&&phase!==0)return false;
   if(phase===0&&(this.codexOpen||this.codexMenuVisible())){const tab=nativeTabHit(x,y);if(tab>=0){this.codexTouchCaptured=true;if(tab===9)this.codexOpenMenu();else{this.codexOpen=false;this.codexDetailsOpen=false;this.codexTabSelected=false;this.codexSelectNativeTab(tab);}return true;}}
   if(phase===0&&!this.codexOpen&&this.codexTabSelected&&this.codexMenuVisible()&&x>=84&&x<316&&y>=25&&y<240)this.codexOpen=true;
   if(this.codexOpen&&(x<84||x>=339||y<0||y>=240))return false;
   if(this.codexOpen){if(phase===0){this.codexTouchCaptured=true;const fHit=nativeFilterHit(x,y,this.width,this.height);if(!this.codexDetailsOpen&&fHit>=0){this.codexFilter=fHit;this.codex.setFilter(fHit);this.codexPage=0;this.codexSelection=0;return true;}if(contains(codexBack(),x,y)){if(this.codexDetailsOpen)this.codexDetailsOpen=false;else{this.codexOpen=false;this.codexTabSelected=true;}}else if(!this.codexDetailsOpen&&contains(nativeDetailsButton(),x,y)){this.codexDetailsOpen=true;this.codexDetailPage=0;this.codexDetailScroll=0;}else if(contains(codexPrevious(),x,y)){if(this.codexDetailsOpen)this.codexDetailsPage(-1);else this.codexChangePage(-1);}else if(contains(codexNext(),x,y)){if(this.codexDetailsOpen)this.codexDetailsPage(1);else this.codexChangePage(1);}else if(this.codexDetailsOpen&&contains(codexDetailUp(),x,y))this.codexScrollDetails(-1);else if(this.codexDetailsOpen&&contains(codexDetailDown(),x,y))this.codexScrollDetails(1);else if(!this.codexDetailsOpen){const index=codexHitTest(x,y,this.codex.catalog(this.codexFilter).length,this.codexPage);if(index>=0)this.codexSelection=index;}}return true;}

   if(phase===0&&!this.codexMenuVisible())this.codexTabSelected=false;return false;
  }
  touch(x,y,type){if(this.features0130.handleTouch(x,y,type))return;if(this.questTracker?.handleTouch(x,y,type))return;if(this.bounties?.open&&this.bounties.handleTouch(x,y,type))return;if(this.sharedStash?.open&&this.sharedStash.handleTouch(x,y,type))return;if(this.weaponsCatalog?.open&&this.weaponsCatalog.handleTouch(x,y,type))return;if(this.difficulty?.open&&this.difficulty.handleTouch(x,y,type))return;if(this.bossRushUI?.open&&this.bossRushUI.handleTouch(x,y,type))return;if(this.inventory.touch(x,y,type))return;if(this.combatPower.touch(x,y,type))return;if(this.codexMenuTouch(x,y,type))return;if(type===0)this.nativeTouchDown=true;if(type===1)this.nativeTouchDown=false;if(type===0||type===2)this.nativeTouchPoint={x,y};if(this.uiScale!==100&&type!==3){const point=this.uiRenderer.touchPoint(x,y,this.call(this.exports.get('_getTouchState')));x=point.x;y=point.y;}this.call(this.exports.get('Java_game_destiniaeng_GameThread_jniTouch'),[ENV,16,x,y,type]);this.processNetwork();}
  menuState(){return this.gMenu?this.get32(this.gMenu+156):0;}
  key(key,down){if(this.features0130.handleKey(key,down))return;if(key==='q'||key==='Q'){if(down)this.questTracker?.toggle();return;}if(key==='h'||key==='H'){if(down)this.autoPotion?.toggle();return;}if(this.bounties?.open&&this.bounties.handleKey(key,down))return;if(this.sharedStash?.open&&this.sharedStash.handleKey(key,down))return;if(this.weaponsCatalog?.open&&this.weaponsCatalog.handleKey(key,down))return;if(this.difficulty?.open&&this.difficulty.handleKey(key,down))return;if(key==='F4'||key==='w'||key==='W'){if(down)this.weaponsCatalog.toggle();return;}if(key==='F5'||key==='b'||key==='B'){if(down)this.sharedStash?.toggle();return;}if(key==='F8'||key==='c'||key==='C'){if(down)this.bounties?.toggle();return;}if(this.bossRushUI?.open&&this.bossRushUI.handleKey(key,down))return;if(this.inventory.key(key,down))return;if(this.combatPower.key(key,down))return;if(this.codexMenuKey(key,down))return;const state=this.menuState(),prefix=state<=0?'LogoCanvas':state===1?'MenuCanvas':'GameCanvas';if(state<=0&&!down)return;const fn=this.exports.get(prefix+(down?'_mKeyPressed':'_mKeyReleased'));if(fn)this.call(fn,[key]);this.processNetwork();}
  close(){this.e.close();this.saved.clear();this.files.clear();this.arrays.clear();}
}
