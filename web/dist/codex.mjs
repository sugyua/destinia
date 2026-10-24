// Discovery data only. This web prototype keeps its Codex in session memory.
export class Codex {
 constructor(names=new Map()){this.names=names;this.characters=new Map();this.counted=new Set();this.slot=-1;}
 select(slot){if(!Number.isInteger(slot)||slot<0||slot>9)return;if(slot!==this.slot){this.slot=slot;this.counted.clear();}}
 respawn(instance){this.counted.delete(instance);}
 newMap(){this.counted.clear();}
 erase(slot){this.characters.delete(slot);this.counted.clear();}
 observe(instance,id,killed,facts=null){
  if(this.slot<0||!this.names.has(id))return;
  if(!this.characters.has(this.slot))this.characters.set(this.slot,new Map());
  const records=this.characters.get(this.slot);if(!records.has(id))records.set(id,{id,name:this.names.get(id),kills:0});
  if(facts&&Number.isInteger(facts.maxHP)&&facts.maxHP>0&&facts.maxHP<=2147483647&&Number.isInteger(facts.level)&&facts.level>0&&facts.level<=65535){
   const record=records.get(id),locations=record.facts?.locations||[];
   if(facts.location&&/^m[0-2]_\d+\.mpd$/.test(facts.location.file)&&!locations.some(x=>x.file===facts.location.file))locations.push({file:facts.location.file,name:facts.location.name});
   record.facts={maxHP:facts.maxHP,level:facts.level,elite:!!facts.elite,locations};
  }
  if(killed&&!this.counted.has(instance)){this.counted.add(instance);const r=records.get(id);r.kills=Math.min(4294967295,r.kills+1);}
 }
 facts(id){const f=this.characters.get(this.slot)?.get(id)?.facts;return f?{...f,locations:f.locations.map(x=>({...x}))}:null;}
 catalog(){const records=this.characters.get(this.slot);return [...this.names].sort(([a],[b])=>a-b).map(([id,name])=>{const record=records?.get(id);return {id,name:record?name:'??????',found:!!record,kills:record?.kills||0};});}
 rows(){return [...(this.characters.get(this.slot)?.values()||[])].sort((a,b)=>a.id-b.id).map(r=>({...r}));}
}
export function monsterNames(bytes){
 if(!bytes||bytes.length!==3840)throw Error('Tabela de monstros incompatível.');const names=new Map();
 for(let i=0;i<bytes.length/32;i++){let name='';for(let j=12;j<32&&bytes[i*32+j];j++){const c=bytes[i*32+j];if(c<32||c>126){name='';break;}name+=String.fromCharCode(c);}if(name)names.set(i,name);}
 return names;
}
