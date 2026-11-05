// A later acknowledgement cannot erase an earlier unconfirmed write in this session.
export class SaveStatus {
 constructor(){this.reset();}
 reset(){this.failed=false;this.message='Progresso temporário nesta aba.';return this.message;}
 accept(event){
  if(typeof event?.ok!=='boolean'||typeof event.message!=='string')return this.message;
  if(!event.ok)this.failed=true;
  if(!this.failed||!event.ok)this.message=event.message;
  return this.message;
 }
}
