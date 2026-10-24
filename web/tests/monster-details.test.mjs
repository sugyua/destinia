import test from 'node:test';import assert from 'node:assert/strict';import fs from 'node:fs';
const root=process.env.DESTINIA_GAME_ROOT||'/workspace/destinia-06/game',assets=new Map(fs.readdirSync(root+'/assets').map(n=>[n,new Uint8Array(fs.readFileSync(root+'/assets/'+n))]));
test('dados originais mostram locais reais e HP variavel do Imperial Soldier',async()=>{
 const {loadMonsterMetadata,monsterDetailView}=await import('../dist/monster-details.mjs');const data=await loadMonsterMetadata(assets),m=data.monsters.get(61);
 assert.equal(data.monsters.size,73);assert.equal(data.mapCount,172);assert.equal(m.baseHP,100);assert.equal(m.locations.length,8);assert.ok(!m.locations.some(x=>x.file==='m2_51.mpd'));
 assert.ok(m.locations.some(x=>x.file==='m2_49.mpd'&&x.name==='Dungeon Prison - 4'));
 const view=monsterDetailView({id:61,found:true,name:'Imperial Soldier',kills:2},data,{maxHP:170,level:2,elite:false,locations:[]});
 assert.match(view.hp,/170/);assert.match(view.hp,/2/);assert.ok(view.sections.flatMap(s=>s.lines).some(x=>x.includes('Dungeon Prison')));assert.ok(view.sections.flatMap(s=>s.lines).some(x=>x.includes('Lagrant Castle')));assert.ok(view.sections.flatMap(s=>s.lines).some(x=>x.includes('EQUIPAMENTOS')));assert.ok(view.sections.flatMap(s=>s.lines).some(x=>x.includes('HP Potion')));assert.ok(!view.sections.flatMap(s=>s.lines).some(x=>x.includes('Essence')));
 assert.ok(!view.sections.flatMap(s=>s.lines).some(x=>x.includes('MISSAO ATIVA')),'Imperial Soldier nao tem questdrop na tabela');
 const hidden=monsterDetailView({id:61,found:false,name:'SECRET'},data,{maxHP:170,level:2});assert.equal(hidden,null);
});
test('leitores de detalhes rejeitam tabelas truncadas e nao anunciam locais falsos',async()=>{
 const {loadMonsterMetadata}=await import('../dist/monster-details.mjs');const bad=new Map(assets);bad.set('m2_49.mpd',new Uint8Array([30,30,0]));await assert.rejects(()=>loadMonsterMetadata(bad),/mapa|MPD|recurso/i);
});

test('paginas de detalhes separam Drops, Locais de spawn e Variantes nessa ordem',async()=>{
 const {loadMonsterMetadata,monsterDetailView}=await import('../dist/monster-details.mjs'),data=await loadMonsterMetadata(assets),view=monsterDetailView({id:61,found:true},data,{maxHP:170,level:2,elite:false});
 assert.ok(Array.isArray(view.sections),'Categorias separadas ainda ausentes');assert.deepEqual(view.sections.map(s=>s.id),['drops','spawns','variants']);
 const [drops,spawns,variants]=view.sections;
 assert.ok(drops.lines.some(s=>s.includes('HP Potion')));assert.ok(!drops.lines.some(s=>s.includes('Dungeon Prison')));
 assert.ok(spawns.lines.some(s=>s.includes('Dungeon Prison')));assert.ok(!spawns.lines.some(s=>s.includes('HP Potion')));
 assert.ok(variants.lines.some(s=>s.includes('elite')));assert.ok(variants.lines.some(s=>s.includes('multiplicado por 5')));assert.ok(!variants.lines.some(s=>s.includes('Dungeon Prison')));
});

test('drops usam subpaginas por tipo, um unico item por linha e ovos separados',async()=>{
 const {loadMonsterMetadata,monsterDetailView}=await import('../dist/monster-details.mjs'),data=await loadMonsterMetadata(assets),view=monsterDetailView({id:61,found:true},data,{maxHP:170,level:2,elite:false});
 const groups=view.sections[0].groups;assert.ok(groups?.length>4,'Drops ainda nao separados por categoria');
 const eggs=groups.find(g=>g.id==='eggs');assert.equal(eggs.rows.length,4);assert.deepEqual(eggs.rows.map(r=>r.text.split(' / ')[0]),['Normal Monster Egg','Magic Monster Egg','Rare Monster Egg','Epic Monster Egg']);
 for(const group of groups)for(const row of group.rows){assert.equal(typeof row.text,'string');assert.ok(!row.text.includes(',')&&!row.text.includes('\n'),'Linha mistura itens');assert.ok(row.text.length<=55,'Nome cortado na janela');assert.ok(Number.isInteger(row.color));}
 const equipment=groups.filter(g=>g.id.startsWith('equipment-'));assert.ok(equipment.length>=3);for(const group of equipment){assert.equal(new Set(group.rows.map(r=>r.type)).size,8);assert.equal(new Set(group.rows.map(r=>r.rarity)).size,1);}
 const elite=monsterDetailView({id:61,found:true},data,{maxHP:850,level:2,elite:true});assert.ok(elite.sections[0].groups.every(g=>g.id==='rules'||g.id.startsWith('equipment-')),'Elite nao recebe pool de consumiveis');
});

test('raridades respeitam nivel efetivo e as cores originais sem colorir ovos como equipamentos',async()=>{
 const {loadMonsterMetadata,monsterDetailView}=await import('../dist/monster-details.mjs'),data=await loadMonsterMetadata(assets),colors=[0xffff,0x07e0,0x041f,0xbb9f,0xfc08];
 for(const [level,max]of [[2,2],[37,2],[38,3],[39,3],[40,3],[77,3],[78,4],[79,4],[80,4],[99,4]])for(const elite of [false,true]){
  const view=monsterDetailView({id:61,found:true},data,{maxHP:100,level,elite}),groups=view.sections[0].groups;assert.ok(groups,'Raridades de drops ausentes');
  const equipment=groups.filter(g=>g.id.startsWith('equipment-'));assert.deepEqual(equipment.map(g=>g.rows[0].rarity),Array.from({length:max-(elite?2:0)+1},(_,i)=>i+(elite?2:0)));
  for(const g of equipment)for(const row of g.rows)assert.equal(row.color,colors[row.rarity]);
  if(!elite)assert.ok(groups.find(g=>g.id==='eggs').rows.every(r=>r.color===0xffff));
 }
});

test('receitas sao drops separados de equipamentos e nao incluem aneis nem ramo elite',async()=>{
 const {loadMonsterMetadata,monsterDetailView}=await import('../dist/monster-details.mjs'),data=await loadMonsterMetadata(assets);
 for(const level of [2,38,78]){
  const groups=monsterDetailView({id:61,found:true},data,{level,maxHP:100}).sections[0].groups,recipes=groups.filter(g=>g.id.startsWith('recipe-'));
  assert.equal(recipes.length,level<38?1:level<78?2:3,'Receitas originais ainda nao listadas');
  for(const group of recipes){assert.equal(new Set(group.rows.map(r=>r.type)).size,7);assert.ok(group.rows.every(r=>r.type<7&&r.text.startsWith('Recipe: ')&&r.text.length<=55));}
 }
});
