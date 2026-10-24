// Read-only metadata from the original APK 1.0.6, verified against Map_loadData,
// Mon_createMonster, procScript, GameCanvas_dropItem and GameUI_getNItemIdx.
import {unpackResource} from './codex-portraits.mjs';
const requireData=(ok,message)=>{if(!ok)throw Error('Detalhes: '+message);};
const u16=(b,p)=>b[p]|b[p+1]<<8,s16=(b,p)=>u16(b,p)<<16>>16;
const decode=b=>new TextDecoder().decode(b).replace(/\0.*$/s,'');
export function readTextTable(bytes){
 requireData(bytes?.length>=2,'tabela de textos ausente');const count=u16(bytes,0),rows=[];let at=2;requireData(count<=4096,'tabela de textos excessiva');
 for(let i=0;i<count;i++){requireData(at+2<=bytes.length,'texto truncado');const n=u16(bytes,at);at+=2;requireData(n<=4096&&at+n<=bytes.length,'texto fora dos limites');rows.push(decode(bytes.subarray(at,at+n)));at+=n;}
 requireData(at===bytes.length,'tabela de textos incompatível');return rows;
}
export function readMonsterMap(bytes){
 requireData(bytes?.length>=4,'MPD mapa truncado');let at=4;const skip=n=>{requireData(Number.isSafeInteger(n)&&n>=0&&at+n<=bytes.length,'MPD mapa fora dos limites');at+=n;},byte=()=>{requireData(at<bytes.length,'MPD mapa truncado');return bytes[at++];};
 const [w,h,p,q]=bytes;requireData(w>0&&h>0,'MPD mapa sem dimensoes');skip(p+q+4*w*h);skip(byte()*3);skip(2);skip(byte()*7);skip(byte()*5);skip(byte()*6);
 const species=[];for(let i=0;i<4;i++){const id=byte(),level=byte();byte();species.push({id,baseLevel:level,count:0});}
 const count=byte();for(let i=0;i<count;i++){byte();byte();const slot=byte();byte();requireData(slot<4,'MPD slot de monstro inválido');species[slot].count++;}
 requireData(at===bytes.length,'MPD mapa com dados excedentes');return species;
}
function readBossStages(bytes){
 requireData(bytes?.length>=1,'script ausente');const count=bytes[0],lengths=[5,6,1,2,2,4,5,2,2,4,3,4,3,0,3,1,1,1,1,1,2,4,3,4,3,5,2],stages=[];
 requireData(count<=128&&1+count*4<=bytes.length,'cabecalho de script inválido');
 for(let i=0;i<count;i++){
  const begin=u16(bytes,1+i*4),end=begin+u16(bytes,3+i*4);requireData(begin>=1+count*4&&end<=bytes.length,'script fora dos limites');let at=begin,area=null,map=null;
  while(at<end){const op=bytes[at];requireData(op<lengths.length,'opcode de script incompatível');if(op===13)requireData(at+3<=end,'texto de script truncado');const n=op===13?4+u16(bytes,at+1):lengths[op];requireData(n>0&&at+n<=end,'instrucao de script truncada');
   if(op===0){area=bytes[at+1];map=bytes[at+2];}
   if(op===25){requireData(area!==null&&bytes[at+1]>=100,'estagio de chefe inválido');stages.push({id:bytes[at+1],baseLevel:bytes[at+2],file:`m${area}_${map}.mpd`,script:i});}at+=n;
  }
 }
 return stages;
}
export async function loadMonsterMetadata(assets){
 const table=assets.get('mon_data_en.gdt');requireData(table?.length===120*32,'tabela de monstros incompatível');const monsters=new Map();
 for(let id=0;id<120;id++){const p=id*32,name=decode(table.subarray(p+12,p+32));if(name)monsters.set(id,{id,name,baseHP:u16(table,p),questId:s16(table,p+6),locations:[],dropMode:id>=100?'equipment':'normal'});}
 const names=[0,1,2].map(area=>readTextTable(assets.get(`mapname${area}_en.tdt`))),maps=new Map();
 for(const [file,packed]of assets){const match=file.match(/^m([0-2])_(\d+)\.mpd$/);if(!match)continue;const bytes=await unpackResource(packed),species=readMonsterMap(bytes),name=names[Number(match[1])][Number(match[2])]||file;maps.set(file,name);
  for(const m of species)if(m.count&&monsters.has(m.id))monsters.get(m.id).locations.push({file,name,baseLevel:m.baseLevel});
 }
 for(const stage of readBossStages(assets.get('script_en.sdt')))if(monsters.has(stage.id)){requireData(maps.has(stage.file),'mapa de chefe ausente');monsters.get(stage.id).locations.push({...stage,name:maps.get(stage.file)});}
 const items=assets.get('item_en.gdt');requireData(items?.length===158*166,'tabela de itens incompatível');const itemRows=[];
 for(let p=0;p<items.length;p+=166)itemRows.push({id:u16(items,p),type:u16(items,p+4),tier:u16(items,p+6),name:decode(items.subarray(p+14,p+46))});
 for(const m of monsters.values())m.locations.sort((a,b)=>a.file.localeCompare(b.file));
 const equipmentNames=readTextTable(assets.get('equip_name_en.tdt'));requireData(equipmentNames.length===248,'nomes de equipamentos incompatíveis');
 return {monsters,maps,items:itemRows,equipmentNames,mapCount:maps.size};
}
const typeLabels=new Map([[0,'POCOES HP'],[1,'POCOES MP'],[2,'FRASCOS HP'],[3,'FRASCOS MP'],[5,'ORBES DE FORCA'],[6,'ORBES DE DESTREZA'],[7,'ORBES DE VITALIDADE'],[8,'ORBES DE INTELIGENCIA'],[9,'ORBES DE PRECISAO'],[10,'ORBES DE EVASAO'],[11,'ORBES CRITICOS'],[12,'ORBES VAMPIRICOS'],[13,'MOCHILAS'],[14,'METAIS'],[15,'COUROS'],[16,'TECIDOS'],[17,'PLANTAS'],[22,'PEDRAS DE FORJA'],[23,'PERGAMINHOS DE IDENTIFICACAO'],[24,'OVOS DE MONSTROS']]);
function wrap(lines){const result=[];for(const line of lines){const words=String(line).split(/\s+/);let row='';for(const word of words){if(row&&row.length+word.length+1>55){result.push(row);row='';}row+=(row?' ':'')+word;}result.push(row);}return result;}
// Translations of gametxt_en[122..126]; colors match GameUI_getEquipColor.
const rarities=[['COMUM',0xffff],['MAGICO',0x07e0],['RARO',0x041f],['EPICO',0xbb9f],['LENDARIO',0xfc08]];
const equipmentTypes=['ESPADA GRANDE','ESPADA PEQUENA','ARMA DE FOGO','ELMO','ARMADURA','LUVAS','BOTAS','ANEL'];
function dropGroups(m,data,facts,known){
 const onlyEquipment=m.dropMode==='equipment'||facts?.elite===true,level=known?facts.level:null,max=level===null?4:level+2<40?2:level+2<80?3:4,groups=[];
 const line=(text,color=0xffff,extra={})=>({text,color,...extra});
 const rules=['DROPS POSSIVEIS / NAO GARANTIDOS',level===null?'RARIDADES VARIAM COM O NIVEL':'RARIDADES DO ULTIMO ENCONTRO / NV '+level,'EQUIPAMENTOS: NOMES BASE / PREFIXOS VARIAM'];
 if(onlyEquipment)rules.push('ELITE OU CHEFE: APENAS EQUIPAMENTOS');
 else rules.push('ITENS EXEMPLO DO NIVEL OBSERVADO','OUTROS NIVEIS MUDAM OS ITENS DISPONIVEIS');
 const minimum=onlyEquipment?2:0;
 for(let grade=minimum;grade<=max;grade++)rules.push(rarities[grade][0]);
 // Rarity legend shares the same colors as the corresponding item names.
 groups.push({id:'rules',title:'REGRAS E RARIDADES',rows:rules.map(text=>{const rarity=rarities.find(r=>r[0]===text);return line(text,rarity?.[1]??0xffff);})});
 for(let grade=minimum;grade<=max;grade++){
  const [label,color]=rarities[grade],rows=[];
  for(let type=0;type<8;type++){
   const names=new Set();
   if(level===null)names.add(equipmentTypes[type]);
   else for(let effective=level;effective<=level+2;effective++){
    const cap=effective<40?2:effective<80?3:4;if(grade>cap)continue;
    const sub=grade===4?Math.floor((effective-80)/10)+28:grade===3?Math.floor((effective-40)/10)+21:Math.floor(effective/5);
    const name=data.equipmentNames[type*31+sub];if(name)names.add(name);
   }
   for(const name of names)rows.push(line(name+' / '+label,color,{type,rarity:grade}));
  }
  groups.push({id:'equipment-'+grade,title:'EQUIPAMENTOS / '+label,rows});
 }
 if(!onlyEquipment){
  // Native kind 3 creates a forging recipe for types 0..6, not a ring.
  for(const equipment of groups.filter(g=>g.id.startsWith('equipment-')&&Number(g.id.slice(10))>=2))groups.push({id:'recipe-'+equipment.id.slice(10),title:'RECEITAS / '+rarities[Number(equipment.id.slice(10))][0],rows:equipment.rows.filter(row=>row.type<7).map(row=>({...row,text:'Recipe: '+row.text}))});
  groups.push({id:'gold',title:'OURO',rows:[line('OURO')]});
  for(const [type,label]of typeLabels){
   const candidates=data.items.filter(item=>item.type===type);let items;
   if(type===24)items=candidates;
   else if(known){const tier=Math.floor(level/20);items=[candidates.find(item=>item.tier===tier)||candidates.at(-1)].filter(Boolean);}
   else items=candidates;
   groups.push({id:type===24?'eggs':'items-'+type,title:label,rows:[...new Set(items.map(item=>item.name))].map(name=>line(name))});
  }
  if(m.questId>=0)groups.push({id:'quest',title:'ITEM DE MISSAO',rows:[line('SO COM MISSAO ATIVA COMPATIVEL')]});
 }
 return groups;
}
export function monsterDetailView(row,data,facts=null){
 if(!row?.found)return null;const m=data?.monsters.get(row.id);if(!m)return {hp:'NAO VERIFICADO',notes:'DADOS ORIGINAIS INDISPONIVEIS',sections:[{id:'drops',title:'DROPS',lines:['Dados de drops indisponiveis.']},{id:'spawns',title:'LOCAIS DE SPAWN',lines:['Locais indisponiveis.']},{id:'variants',title:'VARIANTES',lines:['Variantes indisponiveis.']}]};
 const known=facts?.maxHP>0&&facts?.level>0,hp=known?`${facts.maxHP} / NV ${facts.level}`:'VARIA COM O NIVEL',notes=known?'ULTIMO ENCONTRO'+(facts.elite?' / ELITE':''):'HP EXATO DISPONIVEL APOS NOVO ENCONTRO';
 const locations=new Map(m.locations.map(x=>[x.file,x.name]));for(const location of facts?.locations||[])locations.set(location.file,data.maps.get(location.file)||location.name||location.file);
 const spawns=[...locations.values()];if(!locations.size)spawns.push('Local especial ainda nao mapeado.');spawns.push('Locais de campanha e encontros registrados.','Eventos podem usar outros locais.');
 const variants=['HP muda com nivel e dificuldade.'];
 if(m.dropMode==='normal')variants.push('Variante normal: HP conforme nivel.','Variante elite: HP normal multiplicado por 5.','Elites usam o ramo de equipamentos.');else variants.push('Chefe: HP e nivel conforme o encontro.','Variantes especiais so sao mostradas quando verificadas.');
 const groups=dropGroups(m,data,facts,known),lines=groups.flatMap(group=>[group.title,...group.rows.map(row=>row.text)]);
 return {hp,notes,sections:[{id:'drops',title:'DROPS',groups,lines},{id:'spawns',title:'LOCAIS DE SPAWN',lines:wrap(spawns)},{id:'variants',title:'VARIANTES',lines:wrap(variants)}]};
}
