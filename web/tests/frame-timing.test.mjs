import test from 'node:test';import assert from 'node:assert/strict';import fs from 'node:fs';
test('meta de 16 FPS respeita intervalo original e desconta o processamento',async()=>{
 assert.ok(fs.existsSync(new URL('../dist/frame-timing.mjs',import.meta.url)),'Controle da meta de 16 FPS ainda ausente');
 const {frameDelay,targetFPS}=await import('../dist/frame-timing.mjs');assert.equal(targetFPS,16);
 assert.equal(frameDelay(60,20),42.5);assert.equal(frameDelay(100,20),80);assert.equal(frameDelay(60,90),0);assert.equal(frameDelay(40,0),62.5);
});
