import test from 'node:test';import assert from 'node:assert/strict';import fs from 'node:fs';
const path=new URL('../dist/codex.mjs',import.meta.url);
test('Códex registra espécies, mortes únicas e respawn sem misturar slots',async()=>{
 assert.ok(fs.existsSync(path),'Códex ainda não implementado');
 const {Codex}=await import(path);const c=new Codex(new Map([[1,'Fungus'],[4,'Scorpion']]));
 c.select(0);c.observe(100,1,false);assert.equal(c.rows()[0].kills,0);
 c.observe(100,1,true);c.observe(100,1,true);assert.equal(c.rows()[0].kills,1);
 c.observe(200,1,true);assert.equal(c.rows()[0].kills,2);c.respawn(100);c.observe(100,1,true);assert.equal(c.rows()[0].kills,3);
 c.observe(300,999,true);assert.equal(c.rows().length,1);
 c.select(1);assert.equal(c.rows().length,0);c.observe(100,4,true);assert.equal(c.rows()[0].name,'Scorpion');
 c.select(0);assert.equal(c.rows()[0].kills,3);c.erase(0);assert.equal(c.rows().length,0);
});
test('catalogo mostra todas as especies sem revelar desconhecidos nem alterar descobertas',async()=>{
 const {Codex}=await import('../dist/codex.mjs');const c=new Codex(new Map([[61,'Imperial Soldier'],[1,'Fungus'],[100,'Guardian']]));c.select(0);
 assert.equal(typeof c.catalog,'function','Catalogo de especies bloqueadas ainda ausente');
 assert.deepEqual(c.catalog(),[{id:1,name:'??????',found:false,kills:0},{id:61,name:'??????',found:false,kills:0},{id:100,name:'??????',found:false,kills:0}]);
 c.observe(90,61,false);c.observe(90,61,true);c.observe(90,61,true);const before=c.rows();
 assert.deepEqual(c.catalog()[1],{id:61,name:'Imperial Soldier',found:true,kills:1});assert.equal(c.catalog()[0].found,false);assert.deepEqual(c.rows(),before);
 c.select(1);assert.ok(c.catalog().every(r=>!r.found&&r.kills===0));c.select(0);assert.equal(c.catalog()[1].kills,1);
});

test('HP maximo nivel e locais observados ficam separados por personagem',async()=>{
 const {Codex}=await import('../dist/codex.mjs');const c=new Codex(new Map([[61,'Imperial Soldier']]));c.select(0);
 c.observe(100,61,false,{maxHP:170,level:2,elite:false,location:{file:'m2_49.mpd',name:'Dungeon Prison - 4'}});
 assert.equal(typeof c.facts,'function','Fatos do encontro ainda ausentes');assert.equal(c.facts(61).maxHP,170);assert.equal(c.facts(61).level,2);
 c.observe(101,61,false,{maxHP:3460,level:49,elite:false,location:{file:'m2_33.mpd',name:'Lagrant Castle Dungeon - 14'}});assert.equal(c.facts(61).locations.length,2);assert.equal(c.facts(61).maxHP,3460);
 c.observe(102,61,false,{maxHP:0,level:0});assert.equal(c.facts(61).maxHP,3460,'Dados vazios nao apagam HP verificado');
 c.select(1);assert.equal(c.facts(61),null);c.select(0);assert.equal(c.facts(61).locations.length,2);c.erase(0);assert.equal(c.facts(61),null);
});
