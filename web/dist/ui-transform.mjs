// UI-only coordinates; framebuffer/world size and camera remain unchanged.
export class UiTransform {
 constructor(width=400,height=240,percent=100,anchor='Center',fitWidth=0,fitHeight=0,fitX=null,fitY=null){Object.assign(this,{width,height,percent,anchor,fitWidth,fitHeight,fitX,fitY});}
 scale(){let s=this.percent/100;if(this.fitWidth>0)s=Math.min(s,this.width/this.fitWidth);if(this.fitHeight>0)s=Math.min(s,this.height/this.fitHeight);return s;}
 offset(){const s=this.scale();let x=0,y=0;if(this.anchor==='Center'){x=Math.floor(this.width*(1-s)/2);y=Math.floor(this.height*(1-s)/2);}else{if(['TopRight','BottomRight'].includes(this.anchor))x=Math.floor(this.width*(1-s));if(['BottomLeft','BottomRight'].includes(this.anchor))y=Math.floor(this.height*(1-s));}
  if(this.fitWidth>0){const fx=this.fitX??(this.width-this.fitWidth)/2,left=Math.floor(fx*s+x),right=Math.floor((fx+this.fitWidth)*s+x);x+=left<0?-left:right>this.width?this.width-right:0;}
  if(this.fitHeight>0){const fy=this.fitY??(this.height-this.fitHeight)/2,top=Math.floor(fy*s+y),bottom=Math.floor((fy+this.fitHeight)*s+y);y+=top<0?-top:bottom>this.height?this.height-bottom:0;}
  return{x,y};
 }
 point(x,y){const o=this.offset(),s=this.scale();return{x:Math.floor(x*s+o.x),y:Math.floor(y*s+o.y)};}
 rect(x,y,w,h){const a=this.point(x,y),b=this.point(x+w,y+h);return{x:a.x,y:a.y,w:b.x-a.x,h:b.y-a.y};}
 inverse(x,y){const o=this.offset(),s=this.scale();return{x:Math.ceil((x+1-o.x)/s)-1,y:Math.ceil((y+1-o.y)/s)-1};}
}
