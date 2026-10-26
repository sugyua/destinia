import {overlayGlyph} from './overlay-font.mjs';
import {UiTransform} from './ui-transform.mjs';
// Game framebuffer UI; deliberately independent from the surrounding web page.
export const menuButton=(width=400,height=240)=>({x:width/2+120,y:height/2-120,w:76,h:24});
export const codexPanel=(width=400,height=240)=>({x:(width-360)/2,y:(height-240)/2+25,w:360,h:215});
export const contains=(r,x,y)=>x>=r.x&&y>=r.y&&x<r.x+r.w&&y<r.y+r.h;
export const gridColumns=8,gridRows=4,entriesPerPage=gridColumns*gridRows;
export const codexGrid=(width=400,height=240)=>{const p=codexPanel(width,height);return {x:p.x+24,y:p.y+28,w:312,h:134};};
export const codexCell=(index,width=400,height=240)=>{const g=codexGrid(width,height);return {x:g.x+index%gridColumns*40,y:g.y+Math.floor(index/gridColumns)*34,w:32,h:32};};
export const codexDetails=(width=400,height=240)=>{const p=codexPanel(width,height);return {x:p.x+8,y:p.y+167,w:344,h:23};};
export const codexPrevious=(width=400,height=240)=>{const p=codexPanel(width,height);return {x:p.x+4,y:p.y+192,w:90,h:20};};
export const codexNext=(width=400,height=240)=>{const p=codexPanel(width,height);return {x:p.x+280,y:p.y+192,w:76,h:20};};
export const codexFilterTab=(filter=0,width=400,height=240)=>{
 const p=codexPanel(width,height);
 if(filter===0)return {x:p.x+136,y:p.y+6,w:44,h:14};
 if(filter===1)return {x:p.x+184,y:p.y+6,w:52,h:14};
 return {x:p.x+240,y:p.y+6,w:50,h:14};
};
export const codexFilterHit=(x,y,width=400,height=240)=>{
 for(let f=0;f<3;f++)if(contains(codexFilterTab(f,width,height),x,y))return f;
 return -1;
};
export const detailRowsPerPage=7;
// Each group starts a fresh subpage, even when the previous one has free rows.
export function detailListPages(section){
 const groups=section?.groups?.length?section.groups:[{title:'',rows:section?.lines||[]}],pages=[];
 for(const group of groups){const rows=group.rows||[];for(let i=0;i<Math.max(1,rows.length);i+=detailRowsPerPage)pages.push({title:group.title||'',rows:rows.slice(i,i+detailRowsPerPage)});}
 return pages;
}
export const codexDetailUp=(width=400,height=240)=>{const p=codexPanel(width,height);return {x:p.x+8,y:p.y+174,w:70,h:16};};
export const codexDetailDown=(width=400,height=240)=>{const p=codexPanel(width,height);return {x:p.x+282,y:p.y+174,w:70,h:16};};
export const codexBack=(width=400,height=240)=>{const p=codexPanel(width,height);return {x:p.x+298,y:p.y+4,w:58,h:20};};
export function codexHitTest(x,y,count,page=0,width=400,height=240){
 for(let i=0;i<entriesPerPage&&page*entriesPerPage+i<count;i++)if(contains(codexCell(i,width,height),x,y))return page*entriesPerPage+i;
 return -1;
}
export function codexTransform(width,height,percent,open){const p=codexPanel(width,height);return open?new UiTransform(width,height,percent,'Center',p.w,p.h,p.x,p.y):new UiTransform(width,height,percent,'Center',232,240);}
export function paintCodex(pixels,rows,{open=false,inMenu=false,selected=false,page=0,selectedIndex=0,detailsOpen=false,detailData=null,detailPage=0,detailScroll=0,portraits=new Map(),width=400,height=240,uiPercent=100,filter=0}={}){
 if(!open&&!inMenu)return;
 const dark=0x1946,gold=0xe5b2,white=0xe71c,muted=0x8c92,border=0x6b6b;
 const transform=codexTransform(width,height,uiPercent,open);
 const rawPixel=(x,y,c)=>{if(x>=0&&x<width&&y>=0&&y<height){const at=(y*width+x)*2;pixels[at]=c&255;pixels[at+1]=c>>>8;}};
 const pixel=(x,y,c)=>{const b=transform.rect(x,y,1,1);for(let yy=Math.max(0,b.y);yy<Math.min(height,b.y+b.h);yy++)for(let xx=Math.max(0,b.x);xx<Math.min(width,b.x+b.w);xx++)rawPixel(xx,yy,c);};
 const rect=(x,y,w,h,c)=>{for(let dy=0;dy<h;dy++)for(let dx=0;dx<w;dx++)pixel(x+dx,y+dy,c);};
 const text=(x,y,s,c,scale=1)=>{for(const ch of s){const bits=overlayGlyph(ch);for(let i=0;i<5;i++)for(let j=0;j<11;j++)if(bits[i]&(1<<j))rect(x+i*scale,y+(j-2)*scale,scale,scale,c);x+=6*scale;}};
 if(selected||open){const t=codexTransform(width,height,uiPercent,false);for(let y=height/2-120;y<height/2-95;y++)for(let x=width/2-116;x<width/2+116;x++){const b=t.rect(x,y,1,1);for(let yy=Math.max(0,b.y);yy<Math.min(height,b.y+b.h);yy++)for(let xx=Math.max(0,b.x);xx<Math.min(width,b.x+b.w);xx++){const at=(yy*width+xx)*2;rawPixel(xx,yy,((pixels[at]|pixels[at+1]<<8)>>>1)&0x7bef);}}}
 const b=menuButton(width,height);rect(b.x,b.y,b.w,b.h,selected||open?gold:border);rect(b.x+1,b.y+1,b.w-2,b.h-2,dark);rect(b.x+6,b.y+6,13,12,gold);rect(b.x+7,b.y+7,5,9,dark);rect(b.x+13,b.y+7,5,9,dark);text(b.x+24,b.y+9,'CODEX',selected||open?gold:white);
 if(!open)return;
 const {x,y,w,h}=codexPanel(width,height),ink=0,slot=0xbdd7,paper=0xe71c;
 rect(x,y,w,h,border);rect(x+2,y+2,w-4,h-4,paper);rect(x+5,y+5,w-10,17,0);text(x+12,y+10,'CODEX',gold);text(x+305,y+10,'VOLTAR',white);
 for(let f=0;f<3;f++){
  const tab=codexFilterTab(f,width,height),active=f===filter;
  rect(tab.x,tab.y,tab.w,tab.h,active?gold:border);rect(tab.x+1,tab.y+1,tab.w-2,tab.h-2,dark);
  text(tab.x+3,tab.y+4,['TODOS','COMUNS','CHEFES'][f],active?gold:muted);
 }
 page=Math.max(0,Math.min(Math.floor(page),Math.max(0,Math.ceil(rows.length/entriesPerPage)-1)));
 if(detailsOpen){
  const row=rows[selectedIndex],found=row?.found===true,info=found?detailData:null;
  rect(x+5,y+5,w-10,17,0);text(x+12,y+10,'CÓDEX - INFORMAÇÕES',gold);text(x+305,y+10,'VOLTAR',white);
  const art=found?portraits.get(row.id):null;
  rect(x+12,y+28,60,60,border);rect(x+13,y+29,58,58,0xbdd7);
  if(art?.width>0&&art?.height>0&&art.pixels?.length===art.width*art.height){
   const dw=Math.max(1,Math.floor(56*art.width/Math.max(art.width,art.height))),dh=Math.max(1,Math.floor(56*art.height/Math.max(art.width,art.height)));
   for(let yy=0;yy<dh;yy++)for(let xx=0;xx<dw;xx++){const c=art.pixels[Math.floor(yy*art.height/dh)*art.width+Math.floor(xx*art.width/dw)];if(c&0x10000)pixel(x+14+Math.floor((56-dw)/2)+xx,y+30+Math.floor((56-dh)/2)+yy,c);}
  }else text(x+32,y+46,'?',0x4208,2);
  const bossTag=row?.id>=100||row?.isBoss?'[CHEFE] ':'';
  text(x+82,y+30,found?bossTag+row.name:'??????',row?.id>=100||row?.isBoss?gold:0);text(x+82,y+44,'HP MAX: '+(info?.hp||'??????'),0);
  text(x+82,y+58,found?'MORTOS: '+row.kills:'MORTOS: 0',0);text(x+82,y+74,(info?.notes||'???????').slice(0,43),border);
  const sections=info?.sections||[{title:'INFORMACOES',lines:['MONSTRO AINDA NAO ENCONTRADO']}],pages=Math.max(1,sections.length);detailPage=Math.max(0,Math.min(detailPage,pages-1));
  const section=sections[detailPage],listPages=detailListPages(section),lists=listPages.length;detailScroll=Math.max(0,Math.min(detailScroll,lists-1));const list=listPages[detailScroll];
  text(x+12,y+92,((detailPage+1)+' - '+section.title+(list.title?' / '+list.title:'')).slice(0,55),0);rect(x+12,y+101,w-24,1,border);
  if(section.groups)rect(x+12,y+103,w-24,70,dark);
  list.rows.forEach((line,index)=>{const item=typeof line==='object'&&line!==null;text(x+12,y+105+index*10,String(item?line.text:line).slice(0,55),item?line.color??white:section.groups?white:0);});
  if(lists>1){
   const up=codexDetailUp(width,height),down=codexDetailDown(width,height);
   for(const button of [up,down]){rect(button.x,button.y,button.w,button.h,border);rect(button.x+1,button.y+1,button.w-2,button.h-2,paper);}
   text(up.x+19,up.y+5,'SUBIR',detailScroll?0:muted);text(down.x+17,down.y+5,'DESCER',detailScroll+1<lists?0:muted);text(x+145,y+179,(section.groups?'DROP ':'LISTA ')+(detailScroll+1)+'/'+lists,border);
  }
  text(x+12,y+198,'ANTERIOR',detailPage?0:muted);text(x+145,y+198,'PAG '+(detailPage+1)+'/'+pages,border);text(x+298,y+198,'PROXIMA',detailPage+1<pages?0:muted);return;
 }
 const start=page*entriesPerPage,end=Math.min(start+entriesPerPage,rows.length);
 selectedIndex=Math.max(start,Math.min(Math.floor(selectedIndex),end-1));
 for(let i=start;i<end;i++){
  const row=rows[i],cell=codexCell(i-start,width,height),active=i===selectedIndex,isBoss=row.isBoss||row.id>=100;
  rect(cell.x,cell.y,cell.w,cell.h,active?gold:isBoss?0xf800:border);rect(cell.x+1,cell.y+1,30,30,slot);
  const art=row.found===true?portraits.get(row.id):null;
  if(art&&art.width>0&&art.height>0&&art.pixels?.length===art.width*art.height){
   const dw=Math.max(1,Math.floor(30*art.width/Math.max(art.width,art.height))),dh=Math.max(1,Math.floor(30*art.height/Math.max(art.width,art.height)));
   for(let dy=0;dy<dh;dy++)for(let dx=0;dx<dw;dx++){const c=art.pixels[Math.floor(dy*art.height/dh)*art.width+Math.floor(dx*art.width/dw)];if(c&0x10000)pixel(cell.x+1+Math.floor((30-dw)/2)+dx,cell.y+1+Math.floor((30-dh)/2)+dy,c);}
  }else text(cell.x+11,cell.y+9,'?',0x4208,2);
  if(isBoss)text(cell.x+2,cell.y+22,'B',gold);
 }
 const detail=codexDetails(width,height),row=rows[selectedIndex],found=row?.found===true,isBoss=row?.isBoss||row?.id>=100;
 rect(detail.x,detail.y,detail.w,detail.h,0xffff);rect(x+156,detail.y,1,detail.h,border);rect(x+254,detail.y,1,detail.h,border);
 text(x+12,y+169,isBoss?'CHEFE':'MONSTRO',isBoss?gold:border);text(x+164,y+169,'ESTADO',border);text(x+270,y+169,'MORTOS',border);
 text(x+12,y+180,found?String(row.name||'').slice(0,23):'??????',isBoss?0x8000:ink);text(x+164,y+180,found?'ENCONTRADO':'???????',ink);text(x+270,y+180,String(found?row.kills:0),ink);
 text(x+12,y+198,'ANTERIOR',page?ink:muted);text(x+145,y+198,'PAG '+(page+1)+'/'+Math.max(1,Math.ceil(rows.length/entriesPerPage)),border);text(x+298,y+198,'PROXIMA',end<rows.length?ink:muted);
}
