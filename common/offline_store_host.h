// Include after the host's ARM helpers/export table. Never grant cash locally.
static bool finishOfflinePurchase(){
 if(!pendingBuyCash)return true;
 pendingBuyCash=false;pendingCashAmount=0;
 auto find=[](const char* name){auto it=exportsMap.find(name);return it==exportsMap.end()?0u:it->second;};
 const auto getCash=find("getMyCash"),comm=find("Comm"),menuCallback=find("Menu_netProc"),result=find("Java_game_destiniaeng_GameThread_procCommMsg"),net=find("MMain_setNetState"),main=find("MMain");
 if(!getCash||!comm||!net||!main){hostReport("[host] Loja offline: contrato nativo ausente.\n");return false;}
 u32 before=0,after=0;if(!callFn(getCash,{},&before))return false;
 // Comm_resultCB dereferences Comm.callback. Calling it while NULL crashed
 // older experimental cancellation tests. Only the verified menu callback
 // receives the native cancellation packet; otherwise no callback is invoked.
 if(result && menuCallback && rd32(comm)==menuCallback && rd32(comm+28)==30){
  if(!callFn(result,{ENV_OBJ,0x10,1,static_cast<u32>(-10)}))return false;
 }
 if(!callFn(net,{0}) || !callFn(getCash,{},&after))return false;
 if(after!=before || rd16(main+14624)!=0){hostReport("[host] ERRO: cancelamento offline nao preservou saldo/estado.\n");return false;}
 hostReport("[host] Loja offline: compra cancelada; saldo preservado %u; rede liberada.\n",after);
 return true;
}
