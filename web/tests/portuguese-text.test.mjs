import test from 'node:test';
import assert from 'node:assert/strict';
import {translateAsset} from '../dist/portuguese-text.mjs';
import {overlayGlyph} from '../dist/overlay-font.mjs';
const encode=new TextEncoder();
function table(text){const b=encode.encode(text);return Uint8Array.of(1,0,b.length&255,b.length>>>8,...b);}
test('rebuilds Portuguese TDT lengths without modifying source',()=>{
 const original=table('Inventory'),translated=translateAsset('gametxt_en.tdt',original);
 assert.equal(translated[2],10);assert.equal(new TextDecoder().decode(translated.slice(4)),'Inventario');
 assert.equal(new TextDecoder().decode(original.slice(4)),'Inventory');
 assert.equal(new TextDecoder().decode(translateAsset('mapname0_en.tdt',table('North Darkwoods - 1')).slice(4)),'Bosque Sombrio Norte - 1');
});
test('malformed tables and narrow GDT fields retain original bytes',()=>{
 const broken=Uint8Array.of(1,0,255,255,5);assert.deepEqual(translateAsset('gametxt_en.tdt',broken),broken);
 const narrow=Uint8Array.of(...encode.encode('Inventory'),0,0x81);assert.deepEqual(translateAsset('item_en.gdt',narrow),narrow);
 const padded=Uint8Array.of(...encode.encode('Inventory'),0,0,0,0,0,0x81),translated=translateAsset('item_en.gdt',padded);
 assert.equal(translated.length,padded.length);assert.equal(translated.at(-1),0x81);assert.equal(new TextDecoder().decode(translated.slice(0,10)),'Inventario');
});
test('accent glyphs have visible marks and distinct pixels',()=>{
 assert.notDeepEqual(overlayGlyph('Ã'),overlayGlyph('A'));assert.notDeepEqual(overlayGlyph('Ç'),overlayGlyph('C'));
 for(const ch of 'ÁÀÂÃÉÊÍÓÔÕÚÜÇáàâãéêíóôõúüç')assert.notDeepEqual(overlayGlyph(ch),overlayGlyph('?'));
});

import {preferredLanguage} from '../dist/language.mjs';
test('browser usa PT-BR sem preferencia e preserva escolha inglesa',()=>{
 for(const absent of [null,undefined,'','invalid','pt'])assert.equal(preferredLanguage(absent),'pt');
 assert.equal(preferredLanguage('en'),'en');
});
