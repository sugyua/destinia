import test from 'node:test';import assert from 'node:assert/strict';
import {centeredCamera,mapViewportBounds,clearMapMargins,tileInsideMap} from '../dist/map-viewport.mjs';
test('small axes center independently and larger axes retain native camera',()=>{
 assert.deepEqual(centeredCamera(30,30,800,480,123,51),{x:-160,y:51});
 assert.deepEqual(centeredCamera(20,10,800,480,123,51),{x:-240,y:-160});
 assert.deepEqual(centeredCamera(60,60,800,480,123,51),{x:123,y:51});
 assert.deepEqual(centeredCamera(30,30,400,240,123,51),{x:123,y:51});
});
test('grey clears margins only and off-map tiles cannot wrap into the frame',()=>{
 const bounds=mapViewportBounds(30,30,800,480,-160,0);assert.deepEqual(bounds,{x0:160,x1:640,y0:0,y1:480});
 const pixels=new Uint8Array(800*480*2);for(let i=0;i<pixels.length;i+=2){pixels[i]=31;pixels[i+1]=248;}const before=pixels.slice();
 clearMapMargins(pixels,800,480,bounds);
 for(let y=0;y<480;y++)for(let x=0;x<800;x++){const at=(y*800+x)*2;assert.equal(pixels[at]|pixels[at+1]<<8,x>=160&&x<640?0xf81f:0x8410);}
 assert(tileInsideMap(160,0,30,30,-160,0));assert(!tileInsideMap(144,0,30,30,-160,0));assert(!tileInsideMap(640,0,30,30,-160,0));
 const full=before.slice();clearMapMargins(full,800,480,{x0:0,x1:800,y0:0,y1:480});assert.deepEqual(full,before);
});
