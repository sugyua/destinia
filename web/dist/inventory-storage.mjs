import {InventoryProtection} from './inventory.mjs';
// Window owns durable localStorage; worker commits only after its acknowledgement.
export function writeInventoryProtection(storage,mode,request){
 let text=null;try{const key='destinia:inventory-protect-v1:'+mode;if(request.key!==key)throw Error('CONTEXTO DE SAVE MUDOU');text=storage.getItem(key);if(text!==request.expected)throw Error('PROTECOES MUDARAM - REABRA');if(text!==null)new InventoryProtection().decode(text);new InventoryProtection().decode(request.text);storage.setItem(key,request.text);return {type:'inventoryProtectionAck',id:request.id,ok:true,text:request.text};}
 catch(error){return {type:'inventoryProtectionAck',id:request.id,ok:false,text,error:error.message};}
}
export class InventoryStorageBridge {
 constructor(snapshot,send){this.text=snapshot?.text??null;this.error=snapshot?.error||'';this.send=send;this.next=0;this.pending=null;}
 refresh(snapshot){this.text=snapshot?.text??null;this.error=snapshot?.error||'';}
 getItem(){if(this.error)throw Error(this.error);return this.text;}
 commit(key,text,done){if(this.pending)throw Error('PROTECAO SENDO SALVA');const id=++this.next;const timer=setTimeout(()=>{if(this.pending?.id===id){this.pending=null;this.error='SEM CONFIRMACAO DE GRAVACAO - RECARREGUE';done(Error(this.error));}},10000);this.pending={id,timer,done};try{this.send({type:'inventoryProtectionWrite',id,key,expected:this.text,text});}catch(error){clearTimeout(timer);this.pending=null;this.error=error.message;throw error;}}
 acknowledge(message){if(!this.pending||message.id!==this.pending.id)return;const {timer,done}=this.pending;clearTimeout(timer);this.pending=null;this.text=message.text??null;this.error='';done(message.ok?null:Error(message.error||'FALHA SALVANDO PROTECAO'));}
}
