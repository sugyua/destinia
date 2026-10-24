import test from 'node:test';
import assert from 'node:assert/strict';
import {blitIndexed} from '../dist/graphics-fast.mjs';
test('blit rejeita enderecos e passos invalidos antes de acessar a memoria',()=>{
 const r={read(){assert.fail('Entrada invalida nao deve ler memoria');},write(){assert.fail('Entrada invalida nao deve escrever memoria');}};
 const valid={dst:0x1000,src:0x2000,palette:0x3000,width:2,height:2,sourceStride:2,destinationStep:2,destinationStride:4};
 for(const patch of [{destinationStep:1.5},{sourceStride:NaN},{src:2**32},{palette:2**32-100},{dst:Infinity},{destinationStride:3.5}])assert.equal(blitIndexed(r,{...valid,...patch}),false);
});
