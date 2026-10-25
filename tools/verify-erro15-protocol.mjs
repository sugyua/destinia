import fs from 'node:fs';
import Module from 'node:module';
import assert from 'node:assert/strict';
import {createHash} from 'node:crypto';
import {DestiniaWeb} from '../web/dist/engine.mjs';
const root=process.env.DESTINIA_GAME_ROOT;
assert.ok(root);
const vendor=new URL('../web/dist/vendor/unicorn_arm.js',import.meta.url);
const m=new Module(vendor.pathname);m.filename=vendor.pathname;m.paths=[];
m._compile(fs.readFileSync(vendor,'utf8'),vendor.pathname);
const uc=await m.exports();
const assets=new Map(fs.readdirSync(root+'/assets').map(n=>[n,new Uint8Array(fs.readFileSync(root+'/assets/'+n))]));
const elf=fs.readFileSync(root+'/lib/armeabi/libdestinia_jni.so');
const output={elfSha256:createHash('sha256').update(elf).digest('hex'),scenarios:[]};
for(const totalLength of [true,false]) {
 const r=new DestiniaWeb(uc,assets,{language:'pt'}),B=0x10000000;
 const call=(n,args=[])=>r.call(r.exports.get(n),args);
 const hex=(a,n)=>Buffer.from(r.read(B+a,n)).toString('hex');
 const log={lengthField:totalLength?'8+payload':'4+payload',replies:[],requests:[],reads:[],gameSaveCalls:0};
 try {
 r.load(elf);r.init();call('MenuCanvas_init',[r.exports.get('MenuCanvas')]);
 const menu=r.exports.get('Menu'),main=r.exports.get('MMain'),ui=r.exports.get('GameUI');
 r.put32(main,r.exports.get('MenuCanvas'));
 r.put32(menu+156,1);r.write(menu+204,new Uint8Array([1,0]));call('MenuCanvas_mKeyPressed',[17]);assert.equal(r.get32(menu+156),8);
 r.put32(menu+164,4);r.write(menu+184,new Uint8Array([10,0]));r.write(menu+190,new Uint8Array([0,0]));
 const load=r.exports.get('GameCanvas_LoadGame')&~1;
 r.e.hook_add(uc.HOOK_CODE,()=>{r.w(0,1);r.e.reg_write_i32(uc.ARM_REG_PC,r.e.reg_read_i32(uc.ARM_REG_LR));},null,load,load);
 const save=r.exports.get('GameCanvas_SaveGame')&~1;
 r.e.hook_add(uc.HOOK_CODE,()=>{log.gameSaveCalls++;throw Error('GameCanvas_SaveGame proibido nesta prova');},null,save,save);
 for(const name of ['Comm_readStr','Comm_read2Bytes','Comm_read4Bytes']){const a=r.exports.get(name)&~1;r.e.hook_add(uc.HOOK_CODE,()=>log.reads.push({name,pos:r.get32(B+0x6adc8)}),null,a,a);}
 call('MenuCanvas_mKeyPressed',[17]);assert.equal(r.get32(B+0x73b2c),111);
 log.requests.push(hex(0x70bd0,111));r.put32(B+0x73b2c,0);
 const reply=(cmd,payload)=>{const b=Buffer.alloc(payload.length+8);b.writeUInt32BE(payload.length+(totalLength?8:4));b.writeUInt32BE(cmd,4);payload.copy(b,8);log.replies.push(b.toString('hex'));call('Java_game_destiniaeng_GameThread_procCommMsg',[0x40000000,16,1,0]);call('Java_game_destiniaeng_GameThread_procCommRead',[0x40000000,16,r.array(b),b.length]);for(let i=0;i<4;i++)call('Comm_mRun');};
 reply(0,Buffer.from('000000010000000844455354494e4941000000024f4b00000000','hex'));
 log.afterLogin={menu:r.get32(menu+156),ui:r.get32(ui+124).toString(16),pending:r.get32(B+0x73b2c),socket:r.get32(B+0x45108),readPos:r.get32(B+0x6adc8),packetLen:r.get32(B+0x6adcc)};
 if(!totalLength){
 assert.equal(r.get32(menu+156),20);assert.equal(r.get32(ui+124),0xa0000000);
 call('MainMenu_mKeyPressed',[17]);log.afterConfirm={menu:r.get32(menu+156),ui:r.get32(ui+124).toString(16)};
 // cmd8 is an explicit probe, not an automatic request from login.
 call('Comm_init',[8]);call('Comm_flush');log.requests.push(hex(0x70bd0,r.get32(B+0x73b2c)));
 reply(8,Buffer.from('000000010000','hex'));log.afterCmd8={menu:r.get32(menu+156),ui:r.get32(ui+124).toString(16),socket:r.get32(B+0x45108)};
 }
 } catch(error){log.error=String(error);}
 finally{r.close();output.scenarios.push(log);}
}
fs.writeFileSync(new URL('../docs/ERRO15_VERIFICACAO.json',import.meta.url),JSON.stringify(output,null,2)+'\n');
console.log(JSON.stringify(output,null,2));
