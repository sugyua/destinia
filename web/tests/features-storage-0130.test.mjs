import test from 'node:test';
import assert from 'node:assert/strict';
import {FeatureStorageBridge,writeFeatureStorage} from '../dist/features-storage.mjs';
test('worker persistence waits for main-thread localStorage confirmation and detects stale tabs',async()=>{
 const values=new Map(),storage={getItem:k=>values.get(k)??null,setItem:(k,v)=>values.set(k,v)};
 let request;
 const bridge=new FeatureStorageBridge('classico',{text:null},r=>{request=r;});
 const waiting=bridge.setItem(bridge.key,'new',null);
 assert.equal(bridge.getItem(bridge.key),null);
 bridge.acknowledge(writeFeatureStorage(storage,'classico',request));
 await waiting;assert.equal(bridge.getItem(bridge.key),'new');
 values.set(bridge.key,'other tab');
 const stale=bridge.setItem(bridge.key,'lost update','new');
 bridge.acknowledge(writeFeatureStorage(storage,'classico',request));
 await assert.rejects(stale,/outra aba/);assert.equal(storage.getItem(bridge.key),'other tab');
});
test('wrong mode, oversized values and storage failure cannot report a successful save',()=>{
 const storage={getItem:()=>null,setItem:()=>{throw Error('quota');}};
 assert.equal(writeFeatureStorage(storage,'classico',{id:1,key:'destinia:features0130:expandido',text:'x',expected:null}).ok,false);
 assert.equal(writeFeatureStorage(storage,'classico',{id:1,key:'destinia:features0130:classico',text:'x'.repeat(4000001),expected:null}).ok,false);
 assert.equal(writeFeatureStorage(storage,'classico',{id:1,key:'destinia:features0130:classico',text:'x',expected:null}).ok,false);
});
