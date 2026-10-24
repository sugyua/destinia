export function mapPoint(clientX,clientY,rect){return {x:Math.max(0,Math.min(399,Math.floor((clientX-rect.left)*400/rect.width))),y:Math.max(0,Math.min(239,Math.floor((clientY-rect.top)*240/rect.height)))};}
export class InputSources {
  constructor(emit){this.emit=emit;this.sources=new Map();this.counts=new Map();}
  press(source,key){if(this.sources.get(source)===key)return;this.release(source);this.sources.set(source,key);const count=this.counts.get(key)||0;this.counts.set(key,count+1);if(!count)this.emit(key,true);}
  release(source){if(!this.sources.has(source))return;const key=this.sources.get(source);this.sources.delete(source);const count=this.counts.get(key)-1;if(count)this.counts.set(key,count);else{this.counts.delete(key);this.emit(key,false);}}
  clear(){for(const key of this.counts.keys())this.emit(key,false);this.counts.clear();this.sources.clear();}
}
// A fast browser tap can arrive entirely between two ARM frames. Keep it down
// until one frame consumes it; pause/cancel can always release immediately.
export class FrameInputs {
 constructor(emit){this.emit=emit;this.held=new Map();this.pending=new Map();}
 handle(event,frame){
  const id=event.type==='key'?'key:'+event.key:'touch';
  if(event.type==='touch'&&event.phase===2){this.emit(event);return;}
  const down=event.type==='key'?event.down:event.phase===0;
  if(down){if(this.held.has(id)){this.pending.delete(id);return;}this.held.set(id,{frame,event});this.emit(event);}
  else if(this.held.has(id)){if(frame>this.held.get(id).frame){this.emit(event);this.held.delete(id);}else this.pending.set(id,event);}
 }
 afterFrame(frame){for(const [id,event] of this.pending)if(frame>this.held.get(id).frame){this.emit(event);this.held.delete(id);this.pending.delete(id);}}
 clearTouch(){const touch=this.held.get('touch');if(touch)this.emit({...touch.event,phase:1});this.held.delete('touch');this.pending.delete('touch');}
 clear(){for(const {event} of this.held.values())this.emit(event.type==='key'?{...event,down:false}:{...event,phase:1});this.held.clear();this.pending.clear();}
}
