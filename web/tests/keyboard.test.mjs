import test from 'node:test';
import assert from 'node:assert/strict';
import {InputSources} from '../dist/input.mjs';
import {installKeyboard, panelShortcuts} from '../dist/keyboard.mjs';

function fixture(){
 const target=new EventTarget(),sent=[],sources=new InputSources((key,down)=>sent.push({key,down}));
 const state={ready:true,paused:false};installKeyboard(target,sources,()=>state);
 const fire=(type,key,extra={})=>{const event=new Event(type,{cancelable:true});Object.assign(event,{key,code:key,repeat:false,...extra});target.dispatchEvent(event);return event;};
 return {fire,sent,state,sources};
}
for(const key of ['F4','F5','F8'])test(`${key}: press/release, repeticao e reabertura`,()=>{
 const f=fixture();assert.ok(f.fire('keydown',key).defaultPrevented);
 assert.ok(f.fire('keydown',key,{repeat:true}).defaultPrevented);
 f.fire('keyup',key);f.fire('keydown',key);f.fire('keyup',key);
 assert.deepEqual(f.sent,[{key,down:true},{key,down:false},{key,down:true},{key,down:false}]);
});
test('pausa bloqueia comandos, mas F5 nao recarrega uma partida carregada',()=>{
 const f=fixture();f.state.paused=true;assert.ok(f.fire('keydown','F5').defaultPrevented);assert.deepEqual(f.sent,[]);
 f.state.ready=false;assert.equal(f.fire('keydown','F5').defaultPrevented,false);
});
test('soltar durante pausa libera a tecla; Tab e atalhos do navegador preservados',()=>{
 const f=fixture();f.fire('keydown','F4');f.state.paused=true;f.fire('keyup','F4');assert.equal(f.sent.at(-1).down,false);
 f.state.paused=false;for(const key of ['Tab','Unidentified'])assert.equal(f.fire('keydown',key).defaultPrevented,false);
 assert.equal(f.fire('keydown','F5',{ctrlKey:true}).defaultPrevented,false);
});
test('mapa preserva comandos anteriores e define os rotulos dos paineis',()=>{
 const f=fixture();f.fire('keydown','w');f.fire('keyup','w');assert.deepEqual(f.sent,[{key:3,down:true},{key:3,down:false}]);
 assert.deepEqual(panelShortcuts.map(s=>s.key),['F4','F5','F8']);
});
