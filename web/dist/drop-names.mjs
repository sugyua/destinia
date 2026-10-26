const BASE=0x10000000,RETURN=0x60900000;
export class DropNames {
 constructor(r){this.r=r;this.enabled=false;this.drawing=false;}
 install(){const r=this.r;for(const [offset,bytes] of [[0x166e0,[240,181,95,70]],[0x12444,[16,181,18,75]]]){const actual=r.read(BASE+offset,bytes.length);if(!bytes.every((b,i)=>actual[i]===b))throw Error('Biblioteca incompatível com nomes dos drops.');}
 r.e.mem_map(RETURN,4096,r.uc.PROT_READ|r.uc.PROT_EXEC);
 for(const pc of [BASE+0x166e0,BASE+0x12444,RETURN])r.e.hook_add(r.uc.HOOK_CODE,()=>{try{this.at(pc);}catch(error){r.error=error;r.e.emu_stop();}},null,pc,pc);}
 at(pc){const r=this.r,setpc=value=>r.e.reg_write_i32(r.uc.ARM_REG_PC,value),lr=()=>r.e.reg_read_i32(r.uc.ARM_REG_LR)>>>0;
 if(pc===RETURN){this.drawing=false;r.e.reg_write_i32(r.uc.ARM_REG_LR,this.caller);setpc(this.caller);return;}
 if(pc===BASE+0x166e0){if(this.enabled&&!this.drawing){this.drawing=true;this.caller=lr();r.e.reg_write_i32(r.uc.ARM_REG_LR,RETURN|1);}return;}
 if(!this.drawing)return;const game=r.exports.get('Game'),index=r.r(0);let visible=false;
 if(index<20&&r.read(game+0x3e76+index,1)[0]>0&&r.read(game+0x3e76+index,1)[0]<128){const entry=game+0x3e90+index*60,x=r.get16(entry+40,true)-r.get16(game+0x4878,true),y=r.get16(entry+42,true)-r.get16(game+0x487a,true);visible=x>=0&&x<(this.r.width||800)&&y>=0&&y<(this.r.height||480);}
 r.w(0,visible?1:0);setpc(lr());}
}
