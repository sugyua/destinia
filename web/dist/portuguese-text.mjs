import {portugueseRules} from './portuguese-rules.mjs';
const tables=new Set(['help_touch_en.tdt','char_name_en.tdt','mapname0_en.tdt','mapname1_en.tdt','mapname2_en.tdt','nick_en.tdt','gametxt_en.tdt','gametxt_f_en.tdt','equip_name_en.tdt','equip_head_en.tdt','qitem_name_en.tdt','skill0_desc_en.tdt','skill1_desc_en.tdt','skill2_desc_en.tdt','pet_skill_en.tdt','attr0_desc_en.tdt','attr1_desc_en.tdt','attr2_desc_en.tdt']);
const records=new Set(['item_en.gdt','quest_en.gdt','quest_hard_en.gdt','mon_data_en.gdt']);
const encoder=new TextEncoder(),decoder=new TextDecoder();
export function translateAsset(name,source){
 if(tables.has(name)){
  if(source.length<2)return source;
  const count=source[0]|source[1]<<8,output=[source[0],source[1]];let at=2,changed=false;
  for(let i=0;i<count;i++){
   if(at+2>source.length)return source;
   const size=source[at]|source[at+1]<<8;at+=2;if(at+size>source.length)return source;
   const bytes=source.subarray(at,at+size),text=decoder.decode(bytes),replacement=portugueseRules[text];
   const translated=replacement===undefined?bytes:encoder.encode(replacement);
   if(translated.length>65535)return source;
   changed ||= replacement!==undefined;output.push(translated.length&255,translated.length>>>8,...translated);at+=size;
  }
  if(at!==source.length || !changed)return source;
  return Uint8Array.from(output);
 }
 if(!records.has(name))return source;
 const output=source.slice();let changed=false;
 for(let at=0;at<source.length;){
  while(at<source.length && source[at]===0)at++;
  const start=at;while(at<source.length && source[at]!==0)at++;
  if(at===start || at===source.length || at-start>512)continue;
  const bytes=source.subarray(start,at);if(bytes.some(b=>b<32 || b>126))continue;
  const replacement=portugueseRules[decoder.decode(bytes)];if(replacement===undefined)continue;
  let end=at;while(end<source.length && source[end]===0)end++;
  const translated=encoder.encode(replacement);if(translated.length+1>end-start)continue;
  output.fill(0,start,end);output.set(translated,start);changed=true;
 }
 return changed?output:source;
}
export function translatedAssets(assets){return new Map(Array.from(assets,([name,bytes])=>[name,translateAsset(name,bytes)]));}
