const keyFor=mode=>{
 if(!['classico','expandido'].includes(mode))throw Error('Modo invalido');
 return 'destinia:features0130:'+mode;
};
export function writeFeatureStorage(storage,mode,request){
 const reply={type:'featuresStorageAck',id:request?.id,ok:false};
 try{
  const key=keyFor(mode);
  if(request?.key!==key||typeof request.text!=='string'||request.text.length>4000000||!(request.expected===null||typeof request.expected==='string'))throw Error('Pedido de save invalido');
  if(storage.getItem(key)!==request.expected)throw Error('Save alterado em outra aba');
  storage.setItem(key,request.text);reply.ok=true;
 }catch(error){reply.error=String(error.message||error);}
 return reply;
}
export class FeatureStorageBridge {
 constructor(mode,snapshot={},send){this.key=keyFor(mode);this.text=snapshot.text??null;this.error=snapshot.error;this.send=send;this.sequence=0;this.pending=null;}
 getItem(key){if(key!==this.key)throw Error('Chave de save invalida');if(this.error)throw Error(this.error);return this.text;}
 setItem(key,text,expected=this.text){
  if(key!==this.key||this.pending)return Promise.reject(Error('Save ocupado ou chave invalida'));
  return new Promise((resolve,reject)=>{
   const id=++this.sequence;
   const timer=setTimeout(()=>{this.pending=null;reject(Error('Sem confirmacao do armazenamento'));},15000);
   this.pending={id,text,resolve,reject,timer};
   try{this.send({type:'featuresStorageWrite',id,key,text,expected});}catch(error){clearTimeout(timer);this.pending=null;reject(error);}
  });
 }
 acknowledge(reply){
  const pending=this.pending;if(!pending||pending.id!==reply.id)return;
  clearTimeout(pending.timer);this.pending=null;
  if(reply.ok===true){this.text=pending.text;pending.resolve();}else pending.reject(Error(reply.error||'Save nao confirmado'));
 }
}
