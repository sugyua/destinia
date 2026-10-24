// Browser recognition, not USB access: only standard-mapped pads are actionable.
const buttons=new Map([[0,17],[1,10],[2,12],[3,27],[4,2],[5,11],[6,4],[7,8],[8,1],[12,3],[13,9],[14,5],[15,7]]);
const down=b=>!!b&&(b.pressed||b.value>.5);
function direction(x=0,y=0,skill=false){if(Math.max(Math.abs(x),Math.abs(y))<.4)return null;return Math.abs(x)>Math.abs(y)?(x<0?(skill?2:5):(skill?11:7)):(y<0?(skill?4:3):(skill?8:9));}
export class WebGamepad {
 constructor({sources,read=()=>globalThis.navigator?.getGamepads?.()??null,onPause=()=>{},onStatus=()=>{},onDisconnect=()=>{},onUse=()=>{}}){Object.assign(this,{sources,read,onPause,onStatus,onDisconnect,onUse});this.held=new Map();this.identity=null;this.neutral=true;this.status='';}
 report(value){if(value!==this.status){this.status=value;this.onStatus(value);}}
 suspend(){for(const id of this.held.keys())this.sources.release(id);this.held.clear();this.neutral=true;}
 unavailable(message){const had=this.identity!==null;this.suspend();this.identity=null;this.report(message);if(had)this.onDisconnect();}
 tick({ready,focused,paused,blocked=false}){
  let pads;try{pads=this.read();}catch{this.unavailable('Controle: API bloqueada ou indisponível neste navegador.');return;}
  if(pads===null){this.unavailable('Controle: Gamepad API indisponível neste navegador.');return;}
  const connected=Array.from(pads).filter(p=>p&&p.connected);
  const pad=connected.find(p=>`${p.index}:${p.id}`===this.identity&&p.mapping==='standard')??connected.find(p=>p.mapping==='standard');
  if(!pad){const had=this.identity!==null;this.suspend();this.identity=null;this.report(connected.length?'Controle detectado, mas sem mapeamento padrão; use touch/teclado.':'Nenhum controle reconhecido. Conecte e pressione um botão.');if(had)this.onDisconnect();return;}
  const id=`${pad.index}:${pad.id}`;
  if(this.identity!==id){const had=this.identity!==null;this.suspend();this.identity=id;if(had)this.onDisconnect();}
  this.report(`Controle reconhecido: ${pad.id} · padrão`);
  if(!ready||!focused||blocked){this.suspend();return;}
  if(this.neutral){if(pad.buttons.some(down)||pad.axes.slice(0,4).some(a=>Math.abs(a)>=.25))return;this.neutral=false;}
  if(pad.buttons.some(down)||pad.axes.slice(0,4).some(a=>Math.abs(a)>=.4))this.onUse();
  // Start toggles pause once, with all buttons/axes neutral before rearming.
  if(down(pad.buttons[9])){this.suspend();this.onPause();return;}
  if(paused){for(const id of this.held.keys())this.sources.release(id);this.held.clear();return;}
  const desired=new Map();
  for(const [button,key] of buttons)if(down(pad.buttons[button]))desired.set(`pad:b${button}`,key);
  for(const [axis,key] of [[0,direction(pad.axes[0],pad.axes[1])],[1,direction(pad.axes[2],pad.axes[3],true)]])if(key!==null)desired.set(`pad:a${axis}`,key);
  for(const id of this.held.keys())if(!desired.has(id))this.sources.release(id);
  for(const [id,key] of desired)this.sources.press(id,key);
  this.held=desired;
 }
}
