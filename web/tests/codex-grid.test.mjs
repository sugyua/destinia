import test from 'node:test';import assert from 'node:assert/strict';import * as ui from '../dist/codex-ui.mjs';
const frame=()=>new Uint8Array(400*240*2),catalog=()=>Array.from({length:73},(_,i)=>({id:i<63?i+1:100+i-63,name:'Monster '+i,found:false,kills:0}));
const color=(p,x,y)=>p[(y*400+x)*2]|p[(y*400+x)*2+1]<<8;
test('grade contém 8 por 4 espaços e acertos respeitam catálogo, página e rodapé',()=>{
 assert.equal(ui.entriesPerPage,32);assert.equal(ui.gridColumns,8);assert.equal(ui.gridRows,4);
 const first=ui.codexCell(0),last=ui.codexCell(31),panel=ui.codexPanel();assert.equal(first.w,32);assert.equal(first.h,32);assert.ok(last.x>first.x&&last.y>first.y);assert.ok(last.y+last.h<ui.codexDetails().y);
 for(const size of [[400,240],[800,480]])for(let i=0;i<32;i++){const c=ui.codexCell(i,...size);assert.equal(ui.codexHitTest(c.x+16,c.y+16,73,0,...size),i);assert.ok(ui.contains(ui.codexPanel(...size),c.x,c.y));}
 const ninth=ui.codexCell(8),tenth=ui.codexCell(9);assert.equal(ui.codexHitTest(ninth.x+1,ninth.y+1,73,2),72);assert.equal(ui.codexHitTest(tenth.x+1,tenth.y+1,73,2),-1);
 const next=ui.codexNext();assert.equal(ui.codexHitTest(next.x+2,next.y+2,73),-1);assert.ok(next.y>=ui.codexDetails().y+ui.codexDetails().h);assert.equal(panel.w,360);
});
test('desconhecidos nunca revelam nome, imagem ou mortes e a última página tem nove espaços',()=>{
 const rows=catalog(),a=frame(),b=frame(),art=new Map([[1,{width:1,height:1,pixels:new Uint32Array([0x1f81f])}]]);
 ui.paintCodex(a,rows,{open:true});rows[0].name='SECRET NAME';rows[0].kills=98765;ui.paintCodex(b,rows,{open:true,portraits:art});assert.ok(Buffer.from(a).equals(Buffer.from(b)),'Unknown entry must mask all private fields');
 const c=frame();ui.paintCodex(c,rows,{open:true,page:2,selectedIndex:64});const nine=ui.codexCell(8),ten=ui.codexCell(9);assert.notEqual(color(c,nine.x,nine.y),color(c,ten.x,ten.y));
});
test('miniatura completa preserva preto e seleção usa mortes reais na barra abaixo',()=>{
 const rows=catalog();rows[0]={id:1,name:'Fungus',found:true,kills:17};const art=new Map([[1,{width:2,height:1,pixels:new Uint32Array([0x10000,0x1f81f])}]]),a=frame(),b=frame();
 ui.paintCodex(a,rows,{open:true,portraits:art,selectedIndex:0});const cell=ui.codexCell(0);assert.equal(color(a,cell.x+3,cell.y+10),0);assert.equal(color(a,cell.x+24,cell.y+10),0xf81f);
 rows[0].kills=18;ui.paintCodex(b,rows,{open:true,portraits:art,selectedIndex:0});const detail=ui.codexDetails();let changed=0;for(let y=0;y<240;y++)for(let x=0;x<400;x++)if(color(a,x,y)!==color(b,x,y)){changed++;assert.ok(ui.contains(detail,x,y));}assert.ok(changed>0);
 const other=frame();ui.paintCodex(other,rows,{open:true,portraits:art,selectedIndex:1});const second=ui.codexCell(1);assert.equal(color(a,cell.x,cell.y),color(other,second.x,second.y));assert.notEqual(color(a,cell.x,cell.y),color(other,cell.x,cell.y));
 assert.equal(rows[0].kills,18);
});

test('janela de detalhes exibe dados do selecionado e oculta dados desconhecidos',()=>{
 const rows=catalog(),info={hp:'100 (NV 1)',notes:'HP varia por nivel',sections:[{id:'drops',title:'DROPS',lines:['Gold','HP Potion']},{id:'spawns',title:'LOCAIS DE SPAWN',lines:['Lagrant Castle Dungeon']},{id:'variants',title:'VARIANTES',lines:['Elite']}]};
 const unknown=frame(),hidden=frame();ui.paintCodex(unknown,rows,{open:true,detailsOpen:true,detailData:info});
 ui.paintCodex(hidden,rows,{open:true,detailsOpen:true,detailData:{hp:'SECRET HP',notes:'SECRET MAP',sections:[{id:'drops',title:'SECRET',lines:['SECRET ITEM']}]}});
 assert.deepEqual(unknown,hidden,'Desconhecido nao deve revelar dados recebidos');
 rows[0]={id:1,name:'Fungus',found:true,kills:2};const known=frame(),grid=frame();ui.paintCodex(known,rows,{open:true,detailsOpen:true,detailData:info});ui.paintCodex(grid,rows,{open:true});
 assert.notDeepEqual(known,grid,'Acao deve mostrar uma janela diferente da grade');
 const changed=frame();ui.paintCodex(changed,rows,{open:true,detailsOpen:true,detailData:{...info,hp:'200 (NV 1)'}});assert.notDeepEqual(known,changed,'HP do selecionado deve ser desenhado');
});


test('uma pagina de categoria nao desenha conteudo de outra e lista longa permanece na categoria',()=>{
 const rows=catalog();rows[0]={id:1,name:'Fungus',found:true,kills:2};
 const detailData={hp:'100',notes:'ULTIMO ENCONTRO',sections:[{id:'drops',title:'DROPS',lines:Array.from({length:20},(_,i)=>'Item '+i)},{id:'spawns',title:'LOCAIS DE SPAWN',lines:['Dungeon Prison']},{id:'variants',title:'VARIANTES',lines:['Elite']}]};
 const a=frame(),b=frame();ui.paintCodex(a,rows,{open:true,detailsOpen:true,detailData,detailPage:0});
 const changed={...detailData,sections:detailData.sections.map((s,i)=>i===1?{...s,lines:['SECRET MAP']}:s)};ui.paintCodex(b,rows,{open:true,detailsOpen:true,detailData:changed,detailPage:0});assert.deepEqual(a,b,'Pagina Drops nao deve pintar os locais');
 const scrolled=frame();ui.paintCodex(scrolled,rows,{open:true,detailsOpen:true,detailData,detailPage:0,detailScroll:1});assert.notDeepEqual(a,scrolled,'Lista longa precisa de navegacao interna');
 const spawns=frame();ui.paintCodex(spawns,rows,{open:true,detailsOpen:true,detailData,detailPage:1});assert.notDeepEqual(a,spawns);
});

test('subpaginas de drops preservam categorias e cores de cada item',()=>{
 const section={id:'drops',title:'DROPS',groups:[{title:'EQUIPAMENTOS',rows:Array.from({length:8},(_,i)=>({text:'Espada '+i,color:0x07e0}))},{title:'OVOS',rows:[{text:'Rare Monster Egg',color:0x001f}]}]};
 assert.equal(typeof ui.detailListPages,'function','Paginacao por categoria de drop ainda ausente');
 const pages=ui.detailListPages(section);assert.deepEqual(pages.map(p=>p.title),['EQUIPAMENTOS','EQUIPAMENTOS','OVOS']);assert.deepEqual(pages.map(p=>p.rows.length),[7,1,1]);
 const rows=catalog();rows[0]={id:1,name:'Fungus',found:true,kills:1};const a=frame(),b=frame(),detailData={hp:'100',sections:[section]};
 ui.paintCodex(a,rows,{open:true,detailsOpen:true,detailData,detailScroll:0});ui.paintCodex(b,rows,{open:true,detailsOpen:true,detailData,detailScroll:2});
 const colors=p=>{const values=new Set();for(let y=130;y<199;y++)for(let x=32;x<368;x++)values.add(color(p,x,y));return values;};
 assert.ok(colors(a).has(0x07e0));assert.ok(!colors(a).has(0x001f));assert.ok(colors(b).has(0x001f));assert.ok(!colors(b).has(0x07e0));
 rows[0].found=false;const hidden=frame(),masked=frame();ui.paintCodex(hidden,rows,{open:true,detailsOpen:true,detailData});ui.paintCodex(masked,rows,{open:true,detailsOpen:true});assert.deepEqual(hidden,masked);
});
