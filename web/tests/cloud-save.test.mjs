import test from 'node:test';
import assert from 'node:assert/strict';
import http from 'node:http';
import {DestiniaWeb} from '../dist/engine.mjs';

// Real HTTP on a kernel-selected loopback port; only synthetic bytes in memory.
async function fixture(handler,run){
 const requests=[];
 const server=http.createServer(async(req,res)=>{
  let body='';for await(const chunk of req)body+=chunk;
  requests.push({method:req.method,url:req.url,body});
  const reply=handler(req,body);res.writeHead(reply.status??200,{'Content-Type':'application/json'});
  res.end(typeof reply.body==='string'?reply.body:JSON.stringify(reply.body));
 });
 await new Promise(resolve=>server.listen(0,'127.0.0.1',resolve));
 const previous=Object.getOwnPropertyDescriptor(globalThis,'location');
 Object.defineProperty(globalThis,'location',{value:{origin:'http://127.0.0.1:'+server.address().port},configurable:true});
 const engine=Object.create(DestiniaWeb.prototype),logs=[],statuses=[];
 // Write-only cases start with an already validated server snapshot.
 engine.cloudLoadOk=true;engine.saved=new Map();engine.log=message=>logs.push(message);engine.onSaveStatus=value=>statuses.push(value);
 try{await run({engine,logs,statuses,requests});}
 finally{if(previous)Object.defineProperty(globalThis,'location',previous);else delete globalThis.location;server.closeAllConnections();await new Promise(resolve=>server.close(resolve));}
}
const bytes=new Uint8Array([0,17,255,42]),encoded=Buffer.from(bytes).toString('base64');

test('cloud load stages synthetic files and preserves protected sidecars',async()=>{
 await fixture(()=>({body:{ok:true,saves:{'save0.dat':encoded,'protected.txt':encoded}}}),async({engine,statuses,requests})=>{
  engine.preservedFeatureSaveNames=new Set(['protected.txt']);
  assert.equal(await engine.loadCloudSaves(),true);
  for(const name of ['save0.dat','./save0.dat','/data/data/game.destiniaeng/files/save0.dat'])assert.deepEqual(engine.saved.get(name),bytes);
  assert.equal(engine.saved.has('protected.txt'),false);assert.equal(statuses.at(-1).ok,true);
  assert.equal(requests[0].url,'/api/save?format=json');
 });
});
test('cloud load HTTP failure is explicit and keeps in-memory progress',async()=>{
 await fixture(()=>({status:404,body:{ok:true,saves:{}}}),async({engine,statuses})=>{
  engine.saved.set('save0.dat',bytes);assert.equal(await engine.loadCloudSaves(),false);
  assert.deepEqual(engine.saved.get('save0.dat'),bytes);assert.equal(statuses.at(-1).ok,false);
 });
});
test('cloud load rejects malformed later file without partially applying earlier files',async()=>{
 await fixture(()=>({body:{ok:true,saves:{'save0.dat':encoded,'save1.dat':'!!!'}}}),async({engine,statuses})=>{
  engine.saved.set('prior',bytes);assert.equal(await engine.loadCloudSaves(),false);
  assert.deepEqual([...engine.saved.keys()],['prior']);assert.equal(statuses.at(-1).ok,false);
 });
});
test('cloud write awaits actual HTTP and JSON acknowledgement with unchanged payload',async()=>{
 await fixture(()=>({body:{ok:true}}),async({engine,statuses,requests,logs})=>{
  engine.saved.set('/virtual/save0.dat',bytes);assert.equal(await engine.persistCloudSave('/virtual/save0.dat'),true);
  assert.deepEqual(JSON.parse(requests[0].body),{name:'save0.dat',data:encoded});
  assert.equal(statuses.at(-1).ok,true);assert.ok(!logs.some(s=>s.includes('Google Drive')));
 });
});
for(const [label,response] of [['HTTP500',{status:500,body:{ok:true}}],['negative acknowledgement',{body:{ok:false}}],['malformed JSON',{body:'not json'}]]){
 test('cloud write does not claim success on '+label,async()=>{
  await fixture(()=>response,async({engine,statuses})=>{
   engine.saved.set('save0.dat',bytes);assert.equal(await engine.persistCloudSave('save0.dat'),false);
   assert.equal(statuses.at(-1).ok,false);assert.deepEqual(engine.saved.get('save0.dat'),bytes);
  });
 });
}
test('cloud write skips unrelated files without network traffic',async()=>{
 await fixture(()=>({body:{ok:true}}),async({engine,requests})=>{
  engine.saved.set('settings.ini',bytes);assert.equal(await engine.persistCloudSave('settings.ini'),false);assert.equal(requests.length,0);
 });
});
test('save notification failure cannot turn a confirmed write into failure',async()=>{
 await fixture(()=>({body:{ok:true}}),async({engine})=>{
  engine.onSaveStatus=()=>{throw Error('notification unavailable');};engine.saved.set('save0.dat',bytes);
  assert.equal(await engine.persistCloudSave('save0.dat'),true);
 });
});

import {SaveStatus} from '../dist/save-status.mjs';
test('persistent save warning survives later acknowledgements until explicit restart',()=>{
 const status=new SaveStatus();
 assert.equal(status.accept({ok:false,message:'not confirmed'}),'not confirmed');
 assert.equal(status.accept({ok:true,message:'confirmed'}),'not confirmed');
 status.reset();assert.equal(status.accept({ok:true,message:'confirmed'}),'confirmed');
});

test('failed cloud load disables later writes without touching server data',async()=>{
 await fixture(req=>req.method==='GET'?{status:503,body:{ok:false}}:{body:{ok:true}},async({engine,requests,statuses})=>{
  assert.equal(await engine.loadCloudSaves(),false);engine.saved.set('save0.dat',bytes);
  assert.equal(await engine.persistCloudSave('save0.dat'),false);
  assert.equal(requests.length,1,'failed initial download must not permit POST');
  assert.equal(statuses.at(-1).ok,false);
 });
});
