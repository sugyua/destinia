import http from 'node:http';
import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import {createRequire} from 'node:module';
import {UiTransform} from '../dist/ui-transform.mjs';
import {DestiniaWeb} from '../dist/engine.mjs';
const require=createRequire(import.meta.url);
const vendorTemp=fs.mkdtempSync(path.join(os.tmpdir(),'destinia-web-wasm-'));
fs.copyFileSync(new URL('../dist/vendor/unicorn_arm.js',import.meta.url),path.join(vendorTemp,'unicorn.cjs'));
const factory=require(path.join(vendorTemp,'unicorn.cjs'));
process.on('exit',()=>fs.rmSync(vendorTemp,{recursive:true,force:true}));
const root=process.env.DESTINIA_GAME_ROOT||(fs.existsSync('game/assets') ? 'game' : '../game');
const assets=new Map(fs.readdirSync(root+'/assets').map(n=>[n,new Uint8Array(fs.readFileSync(root+'/assets/'+n))]));
const uc=await factory();
function make(options){return new DestiniaWeb(uc,assets,{width:400,height:240,language:'en',...options});}
function invoke(r,name,args){args.forEach((n,i)=>r.w(i,n));r.handlers.get(r.imports.get(name))();return r.r(0);}
test('recusa ELF que nao seja ARM original antes de executar',()=>{const r=make();try{assert.throws(()=>r.load(new Uint8Array(52)),/ELF/);}finally{r.close();}});
test('inicializa a biblioteca original e produz framebuffer real com toque',()=>{const r=make();try{r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();for(let i=0;i<10;i++)r.step();const b=r.framebuffer();assert.equal(b.length,192000);assert.ok(b.some(n=>n));r.touch(200,190,0);r.touch(200,190,1);r.step();assert.equal(r.frame,11);}finally{r.close();}});
test('arquivos de save ficam somente na instancia e tamanho zero nao le memoria',()=>{const r=make();try{const name=r.malloc(64),mode=r.malloc(16),buffer=r.malloc(16);r.text(name,'/data/data/game.destiniaeng/files/test.dat');r.text(mode,'wb+');const handle=invoke(r,'fopen',[name,mode]);assert.ok(handle);r.write(buffer,new Uint8Array([7,9,13]));assert.equal(invoke(r,'fwrite',[buffer,1,3,handle]),3);assert.equal(invoke(r,'fread',[0xffffffff,0,5,handle]),0);assert.equal(invoke(r,'fwrite',[0xffffffff,1,0,handle]),0);invoke(r,'fseek',[handle,0,0]);r.write(buffer,new Uint8Array(3));assert.equal(invoke(r,'fread',[buffer,1,3,handle]),3);assert.deepEqual(Array.from(r.read(buffer,3)),[7,9,13]);assert.equal(r.saved.size,1);}finally{r.close();}assert.equal(r.saved.size,0);});
test('feature save restored locally is not overwritten by an older cloud copy',async()=>{
 const prior=globalThis.fetch,engine=make();
 globalThis.fetch=async()=>({ok:true,json:async()=>({ok:true,saves:{'save0.dat':btoa(String.fromCharCode(1,2)),'save1.dat':btoa(String.fromCharCode(3))}})});
 engine.saved.set('save0.dat',Uint8Array.of(9,8));engine.preservedFeatureSaveNames=new Set(['save0.dat']);
 try{
  assert.equal(await engine.loadCloudSaves(),true);
  assert.deepEqual([...engine.saved.get('save0.dat')],[9,8]);
  assert.deepEqual([...engine.saved.get('save1.dat')],[3]);
 }finally{globalThis.fetch=prior;engine.close();}
});

test('Códex recebe callbacks ARM originais de desenho e derrota',()=>{
 const r=make();try {
  r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();
  assert.ok(r.codex,'Ponte do Códex ainda não implementada');
  const game=r.exports.get('Game'),p=r.malloc(136);r.write(p,new Uint8Array(136));r.put32(game+0x4886,0);r.put32(game+0x4878,0);
  const short=(at,n)=>r.write(p+at,new Uint8Array([n&255,n>>8&255]));
  short(8,1);short(0x12,1);short(0x1a,50);short(0x1c,50);r.put32(p+0x20,10);
  r.call(r.exports.get('drawMonsterOddEff'),[p]);assert.equal(r.codex.rows()[0].name,'Fungus');assert.equal(r.codex.rows()[0].kills,0);
  short(0x12,2);r.put32(p+0x20,0);r.call(r.exports.get('checkQuestMonster'),[p]);r.call(r.exports.get('checkQuestMonster'),[p]);assert.equal(r.codex.rows()[0].kills,1);
  short(8,4);short(0x12,1);short(0x1a,65000);r.put32(p+0x20,10);r.call(r.exports.get('drawMonsterOddEff'),[p]);assert.equal(r.codex.rows().length,1);
  r.call(r.exports.get('GameCanvas_LoadGame'),[1,0]);assert.equal(r.codex.slot,1);assert.equal(r.codex.rows().length,0);
 } finally {r.close();}
});
test('Códex abre pelo menu original, congela o jogo e volta sem alterar Craft/Pets',()=>{
 const r=make();try {
  r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();r.call(r.exports.get("GameCanvas_init"),[r.exports.get("GameCanvas")]);
  for(let i=0;i<10;i++)r.step();
  // Open the actual ARM menu; only its completed animation is controlled here.
  r.call(r.exports.get('openMainmenu'),[0]);const ui=r.exports.get('GameUI'),game=r.exports.get('Game');r.write(ui+0x76,new Uint8Array([2,0]));
  // Entering by keyboard while a native screen gesture is held must release it.
  r.touch(200,100,0);assert.notEqual(r.get16(r.exports.get('MMain')+0x390c),0);
  r.key(5,true);r.key(17,true);assert.equal(r.codexOpen,true);
  assert.equal(r.get16(r.exports.get('MMain')+0x390c),0,'Soltar o toque nativo ao abrir o Códex');
  r.touch(200,100,1);r.key(10,true);r.key(7,true);
  const state=r.get32(ui+0x7c);r.touch(303,12,0);r.touch(303,12,1);
  assert.equal(r.codexOpen,true,'O livro no menu original deve abrir o Códex');
  const before=r.read(game,32056),menuBefore=r.read(ui,12088);for(let i=0;i<3;i++)r.step();
  assert.deepEqual(r.read(game,32056),before,'Combate e progresso não podem avançar durante a consulta');
  assert.deepEqual(r.read(ui,12088),menuBefore);assert.ok(r.framebuffer().some(n=>n));
  r.key(10,true);r.key(10,false);assert.equal(r.codexOpen,false);assert.equal(r.get32(ui+0x7c),state);
  r.key(7,true);r.key(7,false);assert.equal(r.codexTabSelected,false);assert.equal(r.get32(ui+0x7c),0);
  r.key(5,true);r.key(5,false);assert.equal(r.codexTabSelected,true);r.key(17,true);r.key(17,false);assert.equal(r.codexOpen,true);
  r.key(10,true);r.key(10,false);r.key(5,true);r.key(5,false);assert.equal(r.codexTabSelected,false);assert.equal(r.get32(ui+0x7c),0x80000000);
  r.key(7,true);r.key(7,false);assert.equal(r.codexTabSelected,true);assert.equal(r.get32(ui+0x7c),0x80000000);
  // Capture the whole gesture, including the release after closing the panel.
  r.key(17,true);r.touch(290,35,0);assert.equal(r.codexOpen,false);assert.equal(r.codexTouchCaptured,true);r.touch(290,35,1);assert.equal(r.codexTouchCaptured,false);

  // The new entry must not hijack other tabs, submenus, title screens or gameplay.
  r.put32(ui+0x7c,0x50000002);r.touch(303,12,0);r.touch(303,12,1);assert.equal(r.codexOpen,false);
  r.write(game+0x4854,new Uint8Array(2));r.touch(303,12,0);r.touch(303,12,1);assert.equal(r.codexOpen,false);
 } finally {r.close();}
});
test('miniaturas reais e paginas do Codex nao alteram memoria ARM nem contadores',async()=>{
 const r=make();try{
  r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();r.call(r.exports.get("GameCanvas_init"),[r.exports.get("GameCanvas")]);await r.prepareCodexPortraits();
  assert.equal(r.portraits.size,73);assert.ok(r.portraits.get(1).pixels.length);
  r.call(r.exports.get('openMainmenu'),[0]);r.write(r.exports.get('GameUI')+0x76,new Uint8Array([2,0]));r.codexOpenMenu();
  for(let id=1;id<=5;id++)r.codex.observe(10000+id,id,true);
  const game=r.read(r.exports.get('Game'),32056),rows=r.codex.rows(),first=r.framebuffer();
  r.key(7,true);assert.equal(r.codexSelection,1);assert.equal(r.codexPage,0);assert.notDeepEqual(r.framebuffer(),first);
  r.key(5,true);assert.equal(r.codexSelection,0);assert.deepEqual(r.framebuffer(),first);
  r.key(9,true);assert.equal(r.codexSelection,8);r.key(3,true);assert.equal(r.codexSelection,0);
  r.touch(275,225,0);r.touch(275,225,1);assert.equal(r.codexPage,1);assert.equal(r.codexSelection,32);
  r.touch(100,56,0);r.touch(100,56,1);assert.equal(r.codexSelection,32);assert.equal(r.codex.catalog()[32].found,false);
  r.codexChangePage(1);assert.equal(r.codexPage,2);r.codexChangePage(1);assert.equal(r.codexPage,2);
  r.key(7,true);assert.equal(r.codexSelection,65);r.codexChangePage(-1);r.codexChangePage(-1);
  r.key(5,true);assert.equal(r.codexSelection,0);assert.deepEqual(r.framebuffer(),first);
  assert.deepEqual(r.read(r.exports.get('Game'),32056),game);assert.deepEqual(r.codex.rows(),rows);
 }finally{r.close();}
});

test('poses do Códex coincidem pixel a pixel com drawAGDImageT do APK',async()=>{
 const r=make({fastGraphics:false});try {
  r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();
  await r.prepareCodexPortraits();
  const call=(n,args)=>r.call(r.exports.get(n),args),fb=r.malloc(400*240*2),name=r.malloc(128);
  call('MC_grpSetScreenBuffer',[1,fb]);
  for(const id of [1,4,61,100,102,109]) {
   const [wmb,agdName]=(await import('../dist/codex-portraits.mjs')).portraitAssets(id);
   // Obtain the original frame count without using the portrait decoder.
   const {inflateSync}=await import('node:zlib');const packed=assets.get(wmb),w=packed[0]===90?inflateSync(packed.subarray(12)):packed;
   r.text(name,wmb);const bodyImages=call('WImg_loadNewWMBs',[name,w[4]|w[5]<<8]);
   assert.ok(bodyImages);r.text(name,agdName);const agd=r.malloc(8);r.write(agd,new Uint8Array(8));call('loadAGD',[agd,name]);
   const action=id<100?1:0,frame=r.get32(r.get32(agd+4)+action*8+4),parts=call('MC_GETDPTR',[r.get32(frame+12)]),body=[];
   for(let i=0;i<r.read(frame+8,1)[0];i++){const p=parts+i*20;if((r.read(p+12,1)[0]&63)===0)body.push(r.read(p,20));}
   body.forEach((part,i)=>r.write(parts+i*20,part));r.write(frame+8,new Uint8Array([body.length]));
   const groups=r.malloc(12);r.write(groups,new Uint8Array(12));r.put32(groups,bodyImages);
   const draw=background=>{const bytes=new Uint8Array(400*240*2);for(let i=0;i<bytes.length;i+=2){bytes[i]=background&255;bytes[i+1]=background>>8;}r.write(fb,bytes);call('drawAGDImageT',[1,200,120,groups,agd,action,0,0,0]);return r.read(fb,bytes.length);};
   // Two backgrounds distinguish transparency even when a sprite uses that color.
   const a=draw(0x1122),b=draw(0x3344),canvas=new Uint32Array(400*240),bounds=[400,240,-1,-1];
   for(let y=0;y<240;y++)for(let x=0;x<400;x++){const p=(y*400+x)*2;if(a[p]===b[p]&&a[p+1]===b[p+1]){canvas[y*400+x]=0x10000|a[p]|a[p+1]<<8;bounds[0]=Math.min(bounds[0],x);bounds[1]=Math.min(bounds[1],y);bounds[2]=Math.max(bounds[2],x);bounds[3]=Math.max(bounds[3],y);}}
   const expected=[];for(let y=bounds[1];y<=bounds[3];y++)for(let x=bounds[0];x<=bounds[2];x++)expected.push(canvas[y*400+x]);
   const actual=r.portraits.get(id);assert.deepEqual([actual.width,actual.height],[bounds[2]-bounds[0]+1,bounds[3]-bounds[1]+1],String(id));assert.deepEqual(Array.from(actual.pixels),expected,`Original ARM pose ${id}`);
  }
 }finally{r.close();}
});

// These compare the replacement pixel loops against the unmodified ARM routines.
test('desenho acelerado preserva clipping espelhamento rotacao e efeitos originais',async()=>{
 const original=make({fastGraphics:false}),fast=make();
 try {
  const {inflateSync}=await import('node:zlib');
  const setup=r=>{r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();const fb=r.malloc(192000);r.call(r.exports.get('MC_grpSetScreenBuffer'),[1,fb]);const path='mon21_0_head.wmb',name=r.malloc(64),packed=assets.get(path),raw=packed[0]===90?inflateSync(packed.subarray(12)):packed;r.text(name,path);return {fb,images:r.call(r.exports.get('WImg_loadNewWMBs'),[name,raw[4]|raw[5]<<8])};};
  const a=setup(original),b=setup(fast),background=new Uint8Array(192000);for(let i=0;i<background.length;i+=2){background[i]=0x22;background[i+1]=0x11;}
  for(const center of [false,true])for(const routine of ['Grp_drawClipWImageP','Grp_drawSimpleClipWImageP'])for(const mode of [0,1,2,3,4,5,6,7,8,9,10])for(const flags of (routine.includes('Simple')?[0]:[0,1,2,3,4,5,6,7])) {
   for(const [r,state] of [[original,a],[fast,b]]){r.write(state.fb,background);for(const n of ['gProcVarB','gProcVarS','gProcVarR'])r.put32(r.exports.get(n),0x12345678);r.call(r.exports.get('Grp_setDrawOpVal'),[mode,128]);const image=state.images,w=r.get16(image+8),h=r.get16(image+10),args=[1,center?140:flags<4?-3:399,center?100:flags<4?-4:239,image,0,0,w,h];if(routine.includes('Simple'))args.push(0);else args.push(flags,0);r.call(r.exports.get(routine),args);}
   assert.deepEqual(fast.read(b.fb,192000),original.read(a.fb,192000),routine+' op'+mode+' flip'+flags);
   for(const n of ['gProcVarB','gProcVarS','gProcVarR'])assert.equal(fast.get32(fast.exports.get(n)),original.get32(original.exports.get(n)),n);
  }
  assert.ok(fast.graphicsStats?.draws>0,'Aceleracao das copias de pixels ainda nao esta ativa');
  assert.ok(fast.graphicsStats?.alpha>0,'Sprites alpha ainda nao acelerados');
  assert.ok(fast.graphicsStats?.multiply>0,'Sombras ainda nao aceleradas');
  assert.ok(fast.graphicsStats?.effects>0,'Efeitos de luz ainda nao acelerados');
  assert.equal(original.graphicsStats?.draws||0,0);
  assert.throws(()=>fast.read(0xdead0000,16),/read|mem|Unicorn/i);
  assert.equal(fast.read(0xdead0000,0).length,0);
 }finally{original.close();fast.close();}
});

test('preenchimento acelerado preserva recorte alpha e variaveis auxiliares ARM',()=>{
 const original=make({fastGraphics:false}),fast=make();
 try{
  const setup=r=>{r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();const fb=r.malloc(192000);r.call(r.exports.get('MC_grpSetScreenBuffer'),[1,fb]);return fb;};
  const a=setup(original),b=setup(fast),background=new Uint8Array(192000);for(let i=0;i<background.length;i++)background[i]=(i*37)&255;
  for(const mode of [0,1,2])for(const alpha of [0,1,8,15,16])for(const [x,y,w,h]of [[-3,-4,16,19],[100,70,40,30],[399,239,20,20],[410,250,10,10]]){
   for(const [r,fb]of [[original,a],[fast,b]]){r.write(fb,background);for(const n of ['gProcVarB','gProcVarS','gProcVarR'])r.put32(r.exports.get(n),0x12345678);r.call(r.exports.get('Grp_setDrawOpVal'),[mode,alpha*16]);r.call(r.exports.get('Grp_setColorRGB'),[123,160,220]);r.call(r.exports.get('Grp_fillRect'),[1,x,y,w,h]);}
   assert.deepEqual(fast.read(b,192000),original.read(a,192000),`mode${mode} alpha${alpha} (${x},${y})`);
   for(const n of ['gProcVarB','gProcVarS','gProcVarR'])assert.equal(fast.get32(fast.exports.get(n)),original.get32(original.exports.get(n)),n);
  }
  assert.ok(fast.graphicsStats?.fills>0,'Preenchimento alpha ainda nao acelerado');
 }finally{original.close();fast.close();}
});

test('duas mortes comuns de Imperial Soldier contam pelo fluxo ARM real antes do estado morto',()=>{
 const r=make();try{
  r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();for(let i=0;i<10;i++)r.step();
  // Complete the same original data/UI initialization as entering gameplay.
  r.call(r.exports.get('GameCanvas_init'),[r.exports.get('GameCanvas')]);
  r.call(r.exports.get('Map_loadData'),[2,49,1]);r.call(r.exports.get('Mon_createMonster'));
  const g=r.exports.get('Game'),count=r.get32(g+0x1f9c),targets=[];
  for(let i=0;i<count;i++){const p=g+0x14fc+i*0x88;if(r.get16(p+8)===61)targets.push(p);}
  assert.equal(targets.length,3,'Mapa original Dungeon Prison -4 tem tres Imperial Soldiers');
  r.write(g+0x4854,new Uint8Array(2)); // Normal gameplay, not the story/menu state.
  for(const p of targets.slice(0,2)){r.put32(p+0x20,0);r.write(p+0x12,new Uint8Array([1,0,15,0]));}
  r.call(r.exports.get('procMonster'));
  assert.ok(targets.slice(0,2).every(p=>r.get16(p+0x12)===2),'A rotina original deve concluir a morte');
  assert.equal(r.codex.rows().find(m=>m.id===61)?.kills||0,2,'Duas derrotas devem contar mesmo com estado 1 na chamada de checkQuestMonster');
  assert.equal(r.codex.facts(61).maxHP,r.get32(targets[1]+0x24));assert.equal(r.codex.facts(61).level,r.get16(targets[1]+0x10));
  r.call(r.exports.get('procMonster'));assert.equal(r.codex.rows().find(m=>m.id===61).kills,2,'Cadaver nao deve contar de novo');
  assert.ok(r.get32(targets[2]+0x20)>0,'Terceiro monstro permanece vivo');
 }finally{r.close();}
});

test('Acao abre detalhes no jogo e Voltar retorna a selecao sem avancar combate',async()=>{
 const r=make();try{
  r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();r.call(r.exports.get("GameCanvas_init"),[r.exports.get("GameCanvas")]);await r.prepareCodexPortraits();
  r.call(r.exports.get('openMainmenu'),[0]);const ui=r.exports.get('GameUI'),g=r.exports.get('Game');r.write(ui+0x76,new Uint8Array([2,0]));r.codexOpenMenu();
  r.codex.observe(100,61,false,{maxHP:170,level:2,elite:false,location:{file:'m2_49.mpd',name:'Dungeon Prison - 4'}});r.codexSelection=60;r.codexPage=1;
  const rows=r.codex.rows(),before=r.read(g,32056),grid=r.framebuffer();r.key(17,true);assert.equal(r.codexDetailsOpen,true);assert.notDeepEqual(r.framebuffer(),grid);assert.match(r.codexDetailData().hp,/170/);
  for(let i=0;i<3;i++)r.step();assert.deepEqual(r.read(g,32056),before,'Consulta nao deve executar combate');
  assert.equal(r.codexDetailData().sections[r.codexDetailPage].id,'drops');
  r.touch(275,202,0);r.touch(275,202,1);assert.equal(r.codexDetailScroll,1);assert.equal(r.codexDetailPage,0);
  r.key(3,true);assert.equal(r.codexDetailScroll,0);r.key(9,true);assert.equal(r.codexDetailScroll,1);
  r.touch(275,225,0);r.touch(275,225,1);assert.equal(r.codexDetailPage,1);assert.equal(r.codexDetailScroll,0);assert.equal(r.codexDetailData().sections[1].id,'spawns');
  r.key(7,true);assert.equal(r.codexDetailPage,2);assert.equal(r.codexDetailData().sections[2].id,'variants');r.key(7,true);assert.equal(r.codexDetailPage,2);
  r.key(5,true);r.key(5,true);assert.equal(r.codexDetailPage,0);
  r.key(10,true);assert.equal(r.codexDetailsOpen,false);assert.equal(r.codexOpen,true);assert.equal(r.codexSelection,60);assert.equal(r.codexPage,1);
  r.key(6,true);assert.equal(r.codexDetailsOpen,true);r.touch(290,35,0);r.touch(290,35,1);assert.equal(r.codexDetailsOpen,false);assert.equal(r.codexOpen,true);
  r.codexMoveSelection(-1);r.key(17,true);assert.equal(r.codexDetailsOpen,true);assert.equal(r.codexDetailData(),null,'Nao revelar monstro desconhecido');assert.deepEqual(r.codex.rows(),rows);
 }finally{r.close();}
});

test('raridades e nomes base dos drops coincidem com as rotinas ARM originais',async()=>{
 const {monsterDetailView}=await import('../dist/monster-details.mjs'),r=make();try{
  r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();await r.prepareCodexPortraits();
  for(const level of [2,37,38,39,40,77,78,79,80,99])for(const elite of [false,true]){
   const rows=monsterDetailView({id:61,found:true},r.monsterMetadata,{level,maxHP:100,elite}).sections[0].groups.filter(g=>g.id.startsWith('equipment-')).flatMap(g=>g.rows),expected=new Set();
   for(let type=0;type<8;type++)for(let requested=elite?2:0;requested<=4;requested++)for(let effective=level;effective<=level+2;effective++){
    const idx=r.call(r.exports.get('GameUI_getEquipIdx'),[type,requested,effective]),grade=idx>>8&7,sub=idx&31,offset=grade===4?28:grade===3?21:0;
    const name=r.monsterMetadata.equipmentNames[type*31+sub+offset];expected.add(type+'|'+grade+'|'+name);
   }
   assert.deepEqual(new Set(rows.map(row=>row.type+'|'+row.rarity+'|'+row.text.split(' / ')[0])),expected);
   for(const row of rows)assert.equal(row.color,r.call(r.exports.get('GameUI_getEquipColor'),[row.rarity]));
  }
 }finally{r.close();}
});
test('controles virtuais opcionais bloqueiam desenho e hitboxes sem bloquear menus ou comandos físicos',()=>{
 const r=make();try{
  r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();r.step();
  assert.equal(r.virtualControls,true);
  let drawCalls=0;
  for(const name of ['Grp_drawWPNImage','Grp_drawWPNImage2']){const at=r.exports.get(name)&~1;r.e.hook_add(uc.HOOK_CODE,()=>{drawCalls++;},null,at,at);}
  const base=0x10000000,main=r.exports.get('MMain'),game=r.exports.get('Game'),ui=r.exports.get('GameUI'),point=r.malloc(8);
  r.put32(r.get32(base+0x44240),r.exports.get('GameCanvas'));
  for(let index=0;index<14;index++){const b=new Uint8Array(8),v=new DataView(b.buffer);[10+index*20,10,16,16].forEach((n,i)=>v.setInt16(i*2,n,true));r.write(main+0xea+index*8,b);}
  for(const state of [0,-1,16,17]){
   r.write(game+0x4854,new Uint8Array([state<0?255:state===17?16:state,state<0?255:0]));r.put32(ui+0x7c,state===17?0x20000001:0);
   const gameplay=state===0||state===-1,touchState=r.call(r.exports.get('_getTouchState'));
   assert.equal(touchState,state===0?2:state===-1?3:state===16?4:5);
   r.put32(point,state===17?254:94);r.put32(point+4,14);
   r.setVirtualControls(true);const key=r.call(r.exports.get('getKeyPadIdx'),[point]);assert.equal(key,state===17?13:5);
   const fb=r.get32(base+0x4ef5c),pixels=r.get32(fb+8),width=r.get32(fb),height=r.get32(fb+4),background=new Uint8Array(width*height*2).fill(0x47);
   drawCalls=0;r.write(pixels,background);r.call(r.exports.get('MMain_drawTouchButton'),[0]);const visible=r.read(pixels,background.length),visibleCalls=drawCalls;assert.ok(visibleCalls>0);
   r.setVirtualControls(false);assert.equal(r.call(r.exports.get('getKeyPadIdx'),[point]),gameplay?0:key);
   drawCalls=0;r.write(pixels,background);r.call(r.exports.get('MMain_drawTouchButton'),[0]);const hidden=r.read(pixels,background.length);
   assert.equal(r.get16(main+0xd0),touchState);
   if(gameplay){assert.deepEqual(hidden,background);assert.equal(drawCalls,0);}else {assert.deepEqual(hidden,visible);assert.equal(drawCalls,visibleCalls);}
   r.setVirtualControls(true);assert.equal(r.call(r.exports.get('getKeyPadIdx'),[point]),key);
  }
  // A screen gesture is released before changing modes; external controls use
  // GameCanvas callbacks and remain independent of hidden native hitboxes.
  r.write(game+0x4854,new Uint8Array(2));r.put32(ui+0x7c,0);
  let pressed=0,released=0;
  r.e.hook_add(uc.HOOK_CODE,()=>{if(r.r(0)===17)pressed++;},null,r.exports.get('GameCanvas_mKeyPressed')&~1,r.exports.get('GameCanvas_mKeyPressed')&~1);
  r.e.hook_add(uc.HOOK_CODE,()=>{if(r.r(0)===17)released++;},null,r.exports.get('GameCanvas_mKeyReleased')&~1,r.exports.get('GameCanvas_mKeyReleased')&~1);
  r.put32(r.gMenu+156,2);r.setVirtualControls(false);r.key(17,true);r.key(17,false);assert.equal(pressed,1);assert.equal(released,1);
 }finally{r.close();}
});
test('loja offline preserva saldo e libera rede com callback original ou vazio',()=>{
 const r=make();try{
  r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();r.step();
  for(const callback of [0,r.exports.get('Menu_netProc'),0x1234])for(const product of [0,1,2,3,4,5,999]){
   r.call(r.exports.get('setMyCash'),[12345]);r.call(r.exports.get('Comm_SetProcessCB'),[callback]);
   r.call(r.exports.get('Comm_buyCash'),[product]);assert.equal(r.get16(r.exports.get('MMain')+14624),1);
   r.step();assert.equal(r.call(r.exports.get('getMyCash')),12345);assert.equal(r.get16(r.exports.get('MMain')+14624),0);
  }
  r.call(r.exports.get('MMain_SaveData'));assert.ok(r.saved.size);r.call(r.exports.get('setMyCash'),[1]);r.call(r.exports.get('MMain_LoadData'));assert.equal(r.call(r.exports.get('getMyCash')),12345);
 }finally{r.close();}
});

test('modos classico e expandido isolam progresso temporario sem conversao',()=>{const classic=make(),expanded=make({gameMode:'expandido'});try{assert.equal(classic.gameMode,'classico');assert.equal(expanded.gameMode,'expandido');classic.saved.set('save0.dat',new Uint8Array([1,2]));assert.equal(expanded.saved.size,0);expanded.saved.set('save0.dat',new Uint8Array([3]));assert.deepEqual([...classic.saved.get('save0.dat')],[1,2]);}finally{classic.close();expanded.close();}});

test('UI escala pixels antes do clipping/alpha e percorre controles ARM reais',()=>{
 for(const percent of [75,125]){const r=make({uiScale:percent});try{
  r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();r.step();
  const ui=r.uiRenderer,grp=r.exports.get('Grp'),handle=r.get32(grp+4),pixels=r.call(r.exports.get('MC_GRP_GET_FRAME_BUFFER_POINTER'),[handle]),background=new Uint16Array(400*240);for(let i=0;i<background.length;i++)background[i]=(i*31337)&65535;
  for(const alpha of [0,1])for(const x of [40,-8,395]){
   ui.kind=null;r.call(r.exports.get('Grp_setColorPixel'),[0x5c39]);r.call(r.exports.get('Grp_setDrawOpVal'),[alpha,7]);const t=new UiTransform(400,240,percent,'Center',232,240),b=t.rect(x,36,16,48);const right=Math.min(400,b.x+b.w),bottom=Math.min(240,b.y+b.h);b.x=Math.max(0,b.x);b.y=Math.max(0,b.y);b.w=Math.max(0,right-b.x);b.h=Math.max(0,bottom-b.y);
   r.write(pixels,new Uint8Array(background.buffer));r.call(r.exports.get('Grp_fillRect'),[handle,b.x,b.y,b.w,b.h]);const expected=r.read(pixels,192000);
   r.write(pixels,new Uint8Array(background.buffer));ui.kind='Panel';r.call(r.exports.get('Grp_fillRect'),[handle,x,36,16,48]);const actual=r.read(pixels,192000);let mismatch=-1;for(let i=0;i<actual.length;i++)if(actual[i]!==expected[i]){mismatch=i;break;}assert.equal(mismatch,-1,`UIpixel mismatch percent=${percent} alpha=${alpha} x=${x} at=${mismatch}`);
  }
  ui.kind=null;r.call(r.exports.get('GameCanvas_init'),[r.exports.get('GameCanvas')]);r.call(r.exports.get('Map_loadData'),[2,49,1]);r.call(r.exports.get('Mon_createMonster'));
  r.put32(r.get32(0x10044240),r.exports.get('GameCanvas'));r.write(r.exports.get('Game')+0x4854,new Uint8Array(2));r.put32(r.exports.get('GameUI')+0x7c,0);const tick=performance.now();r.step();console.error('UI_FRAME_MS',percent,(performance.now()-tick).toFixed(1));assert.ok(ui.draws>6);assert.equal(ui.frames.length,0);assert.equal(ui.primitive,false);
 }finally{r.close();}}
});

test('UI preserva composicao multiplicativa com alpha usada na selecao de classe',()=>{
 const r=make({uiScale:75});try{
  r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();r.step();const ui=r.uiRenderer,handle=r.get32(r.exports.get('Grp')+4),dst=r.call(r.exports.get('MC_GRP_GET_FRAME_BUFFER_POINTER'),[handle]),src=r.malloc(2),pal=r.malloc(512),bg=new Uint8Array(192000);for(let i=0;i<bg.length;i++)bg[i]=i*73&255;
  r.write(src,new Uint8Array([0,1]));const palette=new Uint16Array(256);palette[0]=0x5c39;palette[1]=0x7bef;r.write(pal,new Uint8Array(palette.buffer));r.call(r.exports.get('Grp_setDrawOpVal'),[1,7]);
  r.write(dst,bg);r.call(r.exports.get('_Grp_drawAlpMultiply16'),[dst+2*(36*400+40),src,pal,2,1,0,1,398]);const expected=r.read(dst,192000);
  r.write(dst,bg);ui.realBase=dst;ui.matrix=new UiTransform(400,240,100,'TopLeft');ui.indexed(0x60000000+2*((512+36)*2048+512+40),src,pal,2,1,2,2,800,4,0x10000000+0x9468);
  const actual=r.read(dst,192000);assert.ok(actual.every((b,i)=>b===expected[i]),'AlphaMultiply deve misturar no pixel real de fundo');
 }finally{r.close();}
});

test('PT-BR e o padrao sem preferencia; ingles explicito preserva os assets originais',()=>{
 const standard=new DestiniaWeb(uc,assets),english=new DestiniaWeb(uc,assets,{language:'en'});
 try{assert.equal(standard.language,'pt');assert.notDeepEqual(standard.assets.get('gametxt_en.tdt'),assets.get('gametxt_en.tdt'));assert.equal(english.language,'en');assert.equal(english.assets,assets);}finally{standard.e.close();english.e.close();}
});

test('nomes de drops ampliam somente visibilidade; distancia de coleta e Game ficam intactos',()=>{
 const r=make();try{
  r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();r.step();r.call(r.exports.get('GameCanvas_init'),[r.exports.get('GameCanvas')]);
  r.put32(r.exports.get('MMain')+4,r.get32(r.exports.get('Grp')+4));
  const game=r.exports.get('Game'),main=r.exports.get('MMain'),pixels=r.call(r.exports.get('MC_GRP_GET_FRAME_BUFFER_POINTER'),[r.get32(main+4)]),bg=new Uint8Array(192000).fill(0x34);
  const shorts=(at,...values)=>{const b=new Uint8Array(values.length*2),d=new DataView(b.buffer);values.forEach((n,i)=>d.setUint16(i*2,n,true));r.write(at,b);};
  const flags=new Uint8Array(20);flags[0]=1;r.write(game+0x3e76,flags);r.write(game+0x3e90,new TextEncoder().encode('TEST DROP\0'));shorts(game+0x3eb8,160,100);shorts(game+0x3ec0,1,0);shorts(game+0x4878,0,0);shorts(game+0x4890,100,100);
  r.write(pixels,bg);r.call(r.exports.get('drawDropItemName'));assert.deepEqual(r.read(pixels,bg.length),bg);
  shorts(game+0x4890,160,100);assert.equal(r.call(r.exports.get('_checkItemNearby'),[0]),1);r.write(pixels,bg);r.call(r.exports.get('drawDropItemName'));const expected=r.read(pixels,bg.length);assert.notDeepEqual(expected,bg);
  shorts(game+0x4890,100,100);const before=r.read(game,0x4900);r.dropNames.enabled=true;r.write(pixels,bg);r.call(r.exports.get('drawDropItemName'));assert.deepEqual(r.read(pixels,bg.length),expected);assert.deepEqual(r.read(game,0x4900),before);assert.equal(r.call(r.exports.get('_checkItemNearby'),[0]),0);
 }finally{r.e.close();}
});

test('FC compara atributos reais com buffs iguais sem alterar Game ou saves',()=>{
 const r=make();try{
  r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();r.step();r.call(r.exports.get('GameCanvas_init'),[r.exports.get('GameCanvas')]);const game=r.exports.get('Game');
  const shorts=(at,...v)=>{const b=new Uint8Array(v.length*2),d=new DataView(b.buffer);v.forEach((n,i)=>d.setUint16(i*2,n,true));r.write(at,b);};
  for(let i=0;i<6;i++)r.put32(game+0x4584+4*i,0);shorts(game+0x48ba,20,15,10,10);shorts(game+0x48fa,20);shorts(game+0x7574,65535);
  const current=r.call(r.exports.get('GameUI_getNewEquipByIdx'),[1]),selected=r.call(r.exports.get('GameUI_getNewEquipByIdx'),[2]),candidate=game+0x2ed6+20*selected;r.put32(game+0x4584,game+0x2ed6+20*current);r.call(r.exports.get('GameUI_calcHeroStat'));
  r.call(r.exports.get('setMyGold'),[20000]);const before=r.read(game,32056),files=[...r.saved];const info=r.combatPower.calculate(candidate,false);assert.equal(info.current,488);assert.equal(r.call(r.exports.get('getMyGold')),20000);assert.equal(info.projected,608);assert.equal(info.after[7]-info.before[7],30);assert.deepEqual(r.read(game,32056),before);assert.deepEqual([...r.saved],files);
  shorts(game+0x48f4,1);assert.equal(r.combatPower.calculate(candidate,false).current,488);shorts(game+0x4972,30);r.call(r.exports.get('GameUI_calcHeroStat'));const buffed=r.combatPower.calculate(candidate,false);assert.equal(buffed.current,500);assert.equal(buffed.projected,624);
  shorts(game+0x4972,0);r.call(r.exports.get('GameUI_calcHeroStat'));assert.equal(r.combatPower.calculate(candidate,false).current,488);
  for(const [job,ownType] of [[0,0],[1,2],[2,1]]){shorts(game+0x4886,job);const a=r.call(r.exports.get('GameUI_getNewEquipByIdx'),[(ownType<<5)|1]);r.put32(game+0x4584,game+0x2ed6+20*a);r.call(r.exports.get('GameUI_calcHeroStat'));for(const type of [0,1,2]){const b=r.call(r.exports.get('GameUI_getNewEquipByIdx'),[(type<<5)|2]),state=r.read(game,32056),cross=r.combatPower.calculate(game+0x2ed6+20*b,false);if(type===ownType){assert.equal(cross.valid,true);assert.ok(cross.projected>cross.current);}else{assert.equal(cross.valid,false);assert.equal(cross.reason,'OUTRA CLASSE');}assert.deepEqual(r.read(game,32056),state);}}
 }finally{r.close();}
});
test('filtros internos preservam ARM, toque escalado e alvos reais de equipar/vender',()=>{
 const records=new Map(),inventoryStorage={getItem:k=>records.get(k)??null,setItem:(k,v)=>records.set(k,v)};const r=make({inventoryStorage});try{
  r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();
  const g=r.exports.get('Game'),ui=r.exports.get('GameUI'),call=(name,args=[])=>r.call(r.exports.get(name),args),short=(at,...v)=>{const b=new Uint8Array(v.length*2),d=new DataView(b.buffer);v.forEach((x,i)=>d.setUint16(i*2,x,true));r.write(at,b);};
  call('GameCanvas_init',[r.exports.get('GameCanvas')]);for(let i=0;i<6;i++)r.put32(g+0x4584+4*i,0);short(g+0x48ba,20,15,10,10);short(g+0x48fa,20);short(g+0x7574,65535);call('GameUI_calcHeroStat');call('setMyGold',[20000]);
  const a=call('GameUI_getNewEquipByIdx',[2]),b=call('GameUI_getNewEquipByIdx',[2]),c=call('GameUI_getNewEquipByIdx',[33]);r.write(g+0x2ed6+20*b,r.read(g+0x2ed6+20*a,20));
  for(let bag=0;bag<4;bag++)short(g+0x4338+bag*0x92,4,0,bag?0:3);[a,b,c].forEach((id,slot)=>short(g+0x433e+6*slot,0,id,1));
  const open=(state,keys)=>{call('openMainmenu',[state]);short(ui+0x76,16);r.put32(r.get32(0x10044240),r.exports.get('GameCanvas'));for(const k of keys)call('MainMenu_mKeyPressed',[k]);};
  open(0,[7,7,9,6,9]);r.touch(1,1,0);assert.equal(r.nativeTouchDown,true);const original=r.read(g,32056);r.key(10016,true);assert.equal(r.nativeTouchDown,false);assert.ok(r.inventory.open);assert.equal(r.inventory.panel.items.length,3);assert.ok(r.inventory.panel.names[0].length);r.step();r.framebuffer();assert.deepEqual(r.read(g,32056),original);r.inventory.panel.query.minPrice=999999;r.inventory.panel.rebuild();assert.equal(r.inventory.panel.view.length,0);assert.equal(r.inventory.choose(),false);r.key(10,true);assert.equal(r.inventory.open,false);
  for(const scale of [75,100,125]){r.uiScale=scale;const t=new UiTransform(400,240,scale,'Center',392,240),point=t.point(350,60);r.touch(point.x,point.y,0);r.touch(point.x,point.y,1);assert.equal(r.inventory.open,true);r.key(10,true);}r.uiScale=100;
  open(0,[7,9,6]);assert.ok(r.inventory.enter());r.inventory.panel.reset();r.inventory.panel.selectedPool=b;assert.ok(r.inventory.choose());const pointer=g+0x2ed6+20*b;assert.equal(r.get32(g+0x4584),pointer);assert.equal(r.get32(ui+0x2148),0);call('MainMenu_mKeyPressed',[6]);assert.equal(r.get32(ui+0x2148),pointer);call('MainMenu_mKeyPressed',[27]);assert.equal(r.get32(g+0x4584),pointer);
  open(0x91000001,[9,6,9]);assert.ok(r.inventory.enter());assert.equal(r.inventory.panel.items[0].price,30);r.inventory.panel.selectedPool=a;
  const protectionBefore=r.read(g,32056),saveBefore=[...r.saved];r.key(10014,true);assert.equal(r.inventory.panel.isProtected(),true);assert.equal(r.inventory.choose(),false);assert.deepEqual(r.read(g,32056),protectionBefore);
  const reopened=make({inventoryStorage});try{reopened.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));reopened.inventory.protection.select(r.get16(g+0x4886));assert.equal(reopened.inventory.protection.has(a,r.read(g+0x2ed6+20*a,20)),true);}finally{reopened.close();}
  const sidecar=inventoryStorage.getItem(r.inventory.storageKey),write=inventoryStorage.setItem;inventoryStorage.setItem=()=>{throw Error('QUOTA EXCEDIDA');};assert.equal(r.inventory.toggleProtection(),false);assert.equal(r.inventory.panel.isProtected(),true);assert.equal(inventoryStorage.getItem(r.inventory.storageKey),sidecar);inventoryStorage.setItem=write;
  r.key(10,true);short(ui+0x5a,0);const protectedUI=r.read(ui,12088);
  for(const key of [6,17]){call('MainMenu_mKeyPressed',[key]);assert.equal(r.get32(ui+0x7c),0x91040004);}
  assert.deepEqual(r.read(g,32056),protectionBefore);assert.deepEqual(r.read(ui,12088),protectedUI);assert.deepEqual([...r.saved],saveBefore);
  assert.ok(r.inventory.enter());r.inventory.panel.selectedPool=b;assert.equal(r.inventory.panel.isProtected(),false);assert.ok(r.inventory.choose());
  assert.ok(r.inventory.enter());r.inventory.panel.selectedPool=a;assert.ok(r.inventory.toggleProtection());assert.equal(r.inventory.panel.isProtected(),false);
  const workingStorage=r.inventory.storage;r.inventory.storage=null;r.inventory.persistenceError='PERSISTENCIA INDISPONIVEL';short(ui+0x5a,0);call('MainMenu_mKeyPressed',[6]);assert.equal(r.get32(ui+0x7c),0x91040004);r.inventory.storage=workingStorage;r.inventory.syncProtection();
  const clean=inventoryStorage.getItem(r.inventory.storageKey);inventoryStorage.setItem(r.inventory.storageKey,'CORROMPIDO');assert.equal(r.inventory.toggleProtection(),false);assert.equal(inventoryStorage.getItem(r.inventory.storageKey),'CORROMPIDO');r.inventory.open=false;short(ui+0x5a,0);call('MainMenu_mKeyPressed',[6]);assert.equal(r.get32(ui+0x7c),0x91040004);assert.deepEqual(r.read(g,32056),protectionBefore);inventoryStorage.setItem(r.inventory.storageKey,clean);assert.ok(r.inventory.enter());r.inventory.panel.selectedPool=a;
  assert.ok(r.inventory.choose());call('MainMenu_mKeyPressed',[6]);assert.equal(r.get32(ui+0x2e0),g+0x433e);call('MainMenu_mKeyPressed',[27]);assert.equal(call('getMyGold'),20000);call('MainMenu_mKeyPressed',[6]);call('MainMenu_mKeyPressed',[6]);assert.equal(call('getMyGold'),20030);assert.equal(r.read(g+0x2e0c+a,1)[0],0);assert.equal(r.get32(g+0x4584),pointer);call('MainMenu_mKeyPressed',[6]);assert.ok(r.inventory.enter());assert.ok(!r.inventory.panel.items.some(i=>i.pool===a));assert.equal(r.inventory.panel.selectedPool,-1);r.inventory.open=false;
 }finally{r.close();}
});
test('dez abas usam navegacao nativa sem substituir Craft ou Pets',()=>{const r=make();try{r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();r.call(r.exports.get('GameCanvas_init'),[r.exports.get('GameCanvas')]);r.call(r.exports.get('openMainmenu'),[0]);const u=r.exports.get('GameUI');r.write(u+0x76,new Uint8Array([16,0]));for(let i=0;i<9;i++){r.codexMenuTouch(96+23*i,12,0);r.codexMenuTouch(96+23*i,12,1);assert.equal(r.get32(u+0x7c)>>>28,i,'Aba '+i);}const at=(x,y)=>(y*400+x)*2;const original=new Uint8Array(400*240*2).fill(0x35);r.nativeCodex.paint(original);assert.notEqual(original[at(96,12)],0x35,'Barra inteira deve usar a geometria centralizada');r.key(7,true);assert.equal(r.codexTabSelected,true);const preview=new Uint8Array(400*240*2).fill(0x35);r.nativeCodex.paint(preview);assert.notEqual(preview[at(91,50)],0x35,'Previa deve aparecer antes da confirmacao');assert.ok(r.nativeCodex.hand.pixels.some(c=>c&0x10000),'Sprite original da mao precisa estar disponivel');r.codexMenuTouch(303,12,0);r.codexMenuTouch(303,12,1);assert.equal(r.codexOpen,true);}finally{r.close();}});
test('moldura e fonte originais do Codex preservam Game UI graficos e descobertas',async()=>{const r=make({uiScale:75});try{r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();r.call(r.exports.get('GameCanvas_init'),[r.exports.get('GameCanvas')]);await r.prepareCodexPortraits();r.call(r.exports.get('openMainmenu'),[0]);r.write(r.exports.get('GameUI')+0x76,new Uint8Array([16,0]));r.codexOpenMenu();r.codex.observe(10,61,true);r.codexSelection=60;r.codexPage=1;const addresses=[['Game',32056],['GameUI',12088],['MMain',14632],['Grp',124],['frameBuffer',20]],before=addresses.map(([n,size])=>r.read(r.exports.get(n),size)),rows=r.codex.rows();const first=r.framebuffer(),heap=r.heap;assert.ok(first.some(v=>v));assert.deepEqual(r.framebuffer(),first);assert.equal(r.heap,heap,'Cache evita trabalho ARM e novas alocacoes por quadro');r.key(6,true);r.framebuffer();for(let i=0;i<addresses.length;i++)assert.deepEqual(r.read(r.exports.get(addresses[i][0]),addresses[i][1]),before[i],addresses[i][0]);assert.deepEqual(r.codex.rows(),rows);assert.equal(r.uiRenderer.percent,75);}finally{r.close();}});

test('Codex abre pela aba dentro do inventario e preserva contexto ao voltar',()=>{const r=make();try{r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();r.call(r.exports.get('GameCanvas_init'),[r.exports.get('GameCanvas')]);r.call(r.exports.get('openMainmenu'),[0]);const u=r.exports.get('GameUI');r.write(u+0x76,new Uint8Array([16,0]));for(const state of [0x10000001,0x20000001]){r.codexOpen=r.codexTabSelected=false;r.put32(u+0x7c,state);const before=r.read(r.exports.get('Game'),32056);r.touch(303,12,0);r.touch(303,12,1);assert.equal(r.codexOpen,true);r.framebuffer();assert.deepEqual(r.read(r.exports.get('Game'),32056),before);r.key(10,true);assert.equal(r.get32(u+0x7c),state);assert.equal(r.codexSelectNativeTab(0),true);assert.equal(r.get32(u+0x7c),0);}}finally{r.close();}});

test('toque na previa do Codex ativa INFO sem atravessar para a aba nativa',()=>{const r=make();try{r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();r.call(r.exports.get('GameCanvas_init'),[r.exports.get('GameCanvas')]);r.call(r.exports.get('openMainmenu'),[0]);r.write(r.exports.get('GameUI')+0x76,new Uint8Array([16,0]));r.key(5,true);assert.equal(r.codexTabSelected,true);assert.equal(r.codexOpen,false);r.touch(270,199,0);r.touch(270,199,1);assert.equal(r.codexOpen,true);assert.equal(r.codexDetailsOpen,true);}finally{r.close();}});


test('Seta direita no corpo de Sistema seleciona Codex sem abrir submenu original',()=>{
 const r=make();try{
  r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();r.call(r.exports.get('GameCanvas_init'),[r.exports.get('GameCanvas')]);
  const ui=r.exports.get('GameUI'),game=r.exports.get('Game');r.call(r.exports.get('openMainmenu'),[0]);r.write(ui+0x76,new Uint8Array([16,0]));
  r.codexSelectNativeTab(8);r.call(r.exports.get('MainMenu_mKeyPressed'),[17]);
  assert.equal(r.get32(ui+0x7c),0x80000001,'Sistema aberto pelo comando original');
  const before=r.read(game,32056);r.key(7,true);r.key(7,false);
  assert.equal(r.codexTabSelected,true,'Direita deve selecionar a decima aba');assert.equal(r.codexOpen,false);
  assert.equal(r.get32(ui+0x7c),0x80000000,'Corpo deve ser cancelado pelo fluxo original');
  r.key(17,true);r.key(17,false);assert.equal(r.codexOpen,true);
  assert.deepEqual(r.read(game,32056),before,'Navegacao nao altera personagem ou itens');
  r.key(10,true);r.key(10,false);r.key(5,true);r.key(5,false);
  assert.equal(r.codexTabSelected,false);assert.equal(r.get32(ui+0x7c),0x80000000);
 }finally{r.close();}
});

test('Mago sintetico usa ARM original sem tocar pools e chama UI fora da emulacao',async()=>{
 for(const width of [400,800]){const r=make({width,height:width===400?240:480});try{
  r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();await r.bossMage.prepared;
  r.call(r.exports.get('GameCanvas_init'),[r.exports.get('GameCanvas')]);r.call(r.exports.get('Map_loadData'),[0,65,1]);const g=r.exports.get('Game'),sh=(p,n)=>r.write(p,new Uint8Array([n&255,n>>>8&255]));
  r.put32(r.get32(0x10044240),r.exports.get('GameCanvas'));sh(g+0x4854,0);sh(g+0x4866,0);const site=r.bossMage.visibleSite();assert.deepEqual(site,{area:0,map:65,x:328,y:344});
  sh(g+0x4890,site.x);sh(g+0x4892,site.y+16);sh(g+0x4898,2);sh(g+0x4878,site.x-width/2);sh(g+0x487a,site.y-r.height/2);r.write(g+0x3e76,new Uint8Array(20));
  let opens=0;r.bossMage.menu.opener=()=>{opens++;r.call(r.exports.get('getMyGold'));};const before=r.read(g,32056),saved=[...r.saved];
  r.call(r.exports.get('GameCanvas_mKeyPressed'),[17]);assert.equal(opens,1);r.call(r.exports.get('GameCanvas_mKeyPressed'),[17]);assert.equal(opens,1);r.call(r.exports.get('GameCanvas_mKeyReleased'),[17]);assert.deepEqual(r.read(g,32056),before);
  const pixels=new Uint8Array(width*r.height*2+32).fill(0x35);r.bossMage.paint(pixels.subarray(16,-16));assert(pixels.subarray(16,-16).some(c=>c!==0x35));assert(pixels.subarray(0,16).every(c=>c===0x35)&&pixels.subarray(-16).every(c=>c===0x35));assert.deepEqual(r.read(g,32056),before);assert.deepEqual([...r.saved],saved);
  r.lootQol.autoTarget=0;r.call(r.exports.get('GameCanvas_mKeyPressed'),[6]);r.lootQol.autoTarget=-1;assert.equal(opens,1);r.write(g,before);
  sh(g+0x1fac,site.x);sh(g+0x1fae,site.y);assert.equal(r.bossMage.visibleSite(),null);r.write(g,before);
  r.bossMage.menu.opener=null;r.call(r.exports.get('GameCanvas_mKeyPressed'),[17]);assert(r.bossMage.menu.pending);r.call(r.exports.get('Map_loadData'),[0,1,1]);assert.equal(r.bossMage.visibleSite(),null);assert.equal(r.bossMage.menu.pending,false);
  r.call(r.exports.get('openMainmenu'),[0]);const ui=r.exports.get('GameUI');sh(ui+0x76,16);
  for(const scale of [75,100,125]){r.uiScale=scale;const {nativeTransform,nativeTabRect}=await import('../dist/codex-native.mjs');const t=nativeTransform(scale,width,r.height);
   for(let i=0;i<10;i++){const b=nativeTabRect(i,width,r.height),p=t.point(b.x+11,b.y+12);r.touch(p.x,p.y,0);r.touch(p.x,p.y,1);if(i===9){assert(r.codexOpen);r.framebuffer();r.key(10,true);}else assert.equal(r.get32(ui+0x7c)>>>28,i);}
  }
 }finally{r.close();}}
});

test('Barra inclui livro inteiro e uma unica moldura nativa antes de selecionar Codex',()=>{
 const r=make();try{
  r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();r.call(r.exports.get('GameCanvas_init'),[r.exports.get('GameCanvas')]);r.call(r.exports.get('openMainmenu'),[0]);r.write(r.exports.get('GameUI')+0x76,new Uint8Array([16,0]));
  const handle=r.get32(r.exports.get('Grp')+4);r.call(r.exports.get('GameUI_drawMainMenu'),[handle]);const ptr=r.call(r.exports.get('MC_GRP_GET_FRAME_BUFFER_POINTER'),[handle]),before=r.read(ptr,192000),out=before.slice();r.nativeCodex.paint(out);
  const raster=r.nativeCodex.cache.values().next().value,at=(x,y)=>(y*400+x)*2;
  for(let y=2;y<19;y++)for(let x=294;x<312;x++)assert.deepEqual(out.slice(at(x,y),at(x,y)+2),raster.slice(at(x,y),at(x,y)+2),'Livro nao pode ser recortado pela borda antiga');
  for(let y=0;y<25;y++)for(let x=321;x<328;x++)assert.deepEqual(out.slice(at(x,y),at(x,y)+2),r.nativeCodex.base.slice(at(x-12,y),at(x-12,y)+2),'Fechamento deve reutilizar a moldura original deslocada');
  for(let y=40;y<240;y++)for(let x=84;x<316;x++)assert.deepEqual(out.slice(at(x,y),at(x,y)+2),before.slice(at(x,y),at(x,y)+2),'Corpo nativo preservado ao centralizar a barra');
  for(let x=294;x<312;x++){r.codexOpen=false;r.codexTabSelected=false;r.touch(x,12,0);r.touch(x,12,1);assert.equal(r.codexOpen,true,'Todo o livro visivel deve ser clicavel');}
 }finally{r.close();}
});

test('Entradas na rotina ARM original incluem Codex no ciclo de abas',()=>{
 const r=make();try{
  r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();r.call(r.exports.get('GameCanvas_init'),[r.exports.get('GameCanvas')]);r.call(r.exports.get('openMainmenu'),[0]);const ui=r.exports.get('GameUI'),g=r.exports.get('Game');r.write(ui+0x76,new Uint8Array([16,0]));
  r.codexSelectNativeTab(8);const before=r.read(g,32056);
  r.call(r.exports.get('MainMenu_mKeyPressed'),[16]);assert.equal(r.codexTabSelected,true,'Direita nativa nao deve pular o Codex');assert.equal(r.get32(ui+0x7c),0x80000000);assert.equal(r.codexOpen,false);
  r.call(r.exports.get('MainMenu_mKeyPressed'),[17]);assert.equal(r.codexOpen,true);assert.deepEqual(r.read(g,32056),before);
  r.codexSelection=16;for(const [key,expected] of [[13,8],[14,16],[15,15],[16,16]]){r.call(r.exports.get('GameCanvas_mKeyPressed'),[key]);assert.equal(r.codexSelection,expected);}assert.deepEqual(r.read(g,32056),before);
  r.key(10,true);r.call(r.exports.get('MainMenu_mKeyPressed'),[7]);assert.equal(r.codexTabSelected,false);assert.equal(r.get32(ui+0x7c),0,'Navegacao interna deve chegar ao Status sem recaptura');
  r.call(r.exports.get('MainMenu_mKeyPressed'),[5]);assert.equal(r.codexTabSelected,true);
 }finally{r.close();}
});

test('FC Status usa fonte ARM sem faixa azul e comparador fica na descricao nativa',()=>{
 const r=make();try{
  r.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));r.init();r.call(r.exports.get('GameCanvas_init'),[r.exports.get('GameCanvas')]);const g=r.exports.get('Game'),u=r.exports.get('GameUI'),s=(at,v)=>r.write(at,new Uint8Array([v&255,v>>>8]));
  for(let i=0;i<6;i++)r.put32(g+0x4584+4*i,0);[20,15,10,10].forEach((v,i)=>s(g+0x48ba+2*i,v));s(g+0x48fa,20);s(g+0x7574,65535);
  const a=r.call(r.exports.get('GameUI_getNewEquipByIdx'),[1]),b=r.call(r.exports.get('GameUI_getNewEquipByIdx'),[2]),candidate=g+0x2ed6+20*b;r.put32(g+0x4584,g+0x2ed6+20*a);r.call(r.exports.get('GameUI_calcHeroStat'));r.call(r.exports.get('_backupHeroStat'));s(g+0x4854,16);s(u+0x76,16);
  const handle=r.get32(r.exports.get('Grp')+4),ptr=r.call(r.exports.get('MC_GRP_GET_FRAME_BUFFER_POINTER'),[handle]);let fonts=0;const at=r.exports.get('Font_drawStringChar')&~1;r.e.hook_add(uc.HOOK_CODE,()=>fonts++,null,at,at);
  r.combatPower.beginFrame();r.call(r.exports.get('_drawMenuStatus'),[handle,84,25,232]);const before=r.read(ptr,192000),after=before.slice(),fontBefore=fonts,game=r.read(g,32056);r.combatPower.paint(after);
  assert.ok(fonts>fontBefore,'FC deve usar a fonte original');assert.deepEqual(after.slice((47*400+182)*2,(47*400+182)*2+2),before.slice((47*400+182)*2,(47*400+182)*2+2),'Sem faixa azul');assert.deepEqual(r.read(g,32056),game);
  r.put32(u+0x7c,0x20000002);r.combatPower.beginFrame();r.call(r.exports.get('_drawEquipDesc'),[handle,91,157,candidate,0]);const item=r.read(ptr,192000),itemPage=item.slice();r.combatPower.paint(itemPage);
  for(let y=0;y<225;y++)assert.deepEqual(itemPage.slice(y*800,(y+1)*800),item.slice(y*800,(y+1)*800),'Pagina Item preserva texto e grade originais');
  const ui=r.read(u,12088);r.combatPower.key(11,true);const comparison=item.slice();r.combatPower.paint(comparison);assert.equal(r.combatPower.comparison,true);assert.notDeepEqual(comparison,itemPage);
  for(let y=0;y<240;y++)for(let x=0;x<400;x++)if(x<91||x>=309||y<157||y>=239){const i=(y*400+x)*2;assert.equal(comparison[i],item[i]);assert.equal(comparison[i+1],item[i+1]);}
  assert.deepEqual(r.read(g,32056),game);assert.deepEqual(r.read(u,12088),ui);r.combatPower.key(2,true);assert.equal(r.combatPower.comparison,false);
  const grp=r.exports.get('Grp'),font=r.exports.get('Font'),graphics=r.read(grp,124),color=r.get32(font),slots=r.read(r.exports.get('frameBuffer'),20),nativeCall=r.call.bind(r);r.combatPower.nativeCache.clear();
  r.call=(address,args)=>{if(address===r.exports.get('Font_drawStringChar'))throw Error('FC_RENDER_FAILURE');return nativeCall(address,args);};
  assert.throws(()=>r.combatPower.paint(item.slice()),/FC_RENDER_FAILURE/);assert.deepEqual(r.read(grp,124),graphics);assert.equal(r.get32(font),color);assert.deepEqual(r.read(r.exports.get('frameBuffer'),20),slots);assert.deepEqual(r.read(g,32056),game);
  r.call=nativeCall;const retry=item.slice();r.combatPower.paint(retry);assert.deepEqual(retry,itemPage);
 }finally{r.close();}
});

// Original ARM produces the fixture; no personal save or Drive path is used.
test('ARM save and load roundtrip through an ephemeral cloud server',async()=>{
 const remote=new Map(),previous=Object.getOwnPropertyDescriptor(globalThis,'location');
 let uploadedResolve;const uploaded=new Promise(resolve=>{uploadedResolve=resolve;});
 const server=http.createServer(async(req,res)=>{
  let body='';for await(const chunk of req)body+=chunk;
  if(req.method==='POST'){
   const value=JSON.parse(body);remote.set(value.name,value.data);
   res.writeHead(200,{'Content-Type':'application/json'});res.end(JSON.stringify({ok:true}));
   if(value.name==='save0.dat')uploadedResolve();
  }else{res.writeHead(200,{'Content-Type':'application/json'});res.end(JSON.stringify({ok:true,saves:Object.fromEntries(remote)}));}
 });
 await new Promise(resolve=>server.listen(0,'127.0.0.1',resolve));
 Object.defineProperty(globalThis,'location',{value:{origin:'http://127.0.0.1:'+server.address().port},configurable:true});
 let engine,timer;
 try{
  engine=make({featureStorage:{getItem:()=>null,setItem:()=>{}}});
  engine.load(new Uint8Array(fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so')));
  assert.equal(await engine.loadCloudSaves(),true);engine.init();
  // Synthetic level1 at the existing documented hero-level field (combat-power.mjs).
  engine.write(engine.exports.get('Game')+0x48fa,Uint8Array.of(1,0));
  engine.call(engine.exports.get('GameCanvas_SaveGame'),[0]);
  await Promise.race([uploaded,new Promise((_,reject)=>{timer=setTimeout(()=>reject(Error('ARM upload not received')),5000);})]);
  clearTimeout(timer);
  const expected=new Uint8Array(engine.saved.get('/data/data/game.destiniaeng/files/save0.dat'));
  assert.ok(expected.length>0);assert.deepEqual(Uint8Array.from(Buffer.from(remote.get('save0.dat'),'base64')),expected);
  engine.saved.clear();assert.equal(await engine.loadCloudSaves(),true);
  assert.deepEqual(engine.saved.get('/data/data/game.destiniaeng/files/save0.dat'),expected);
  assert.equal(engine.call(engine.exports.get('GameCanvas_LoadGame'),[0]),1);
  assert.equal(engine.get16(engine.exports.get('Game')+0x48fa),1);
 }finally{
  clearTimeout(timer);engine?.close();
  if(previous)Object.defineProperty(globalThis,'location',previous);else delete globalThis.location;
  server.closeAllConnections();await new Promise(resolve=>server.close(resolve));
 }
});
