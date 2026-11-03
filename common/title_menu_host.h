#pragma once
#include "title_menu_model.h"
#include "title_menu_view.h"

static destinia::TitleMenuModel titleMenu;
static int titleMenuPending=-1;
static bool titleMenuDraining=false;
static bool titleLoadPending=false;
static destinia::TitleId titleEquipRequest=destinia::TitleId::Nenhum;
static std::string titleMenuMessage;
static bool titleMenuTouchCaptured=false;
static bool titleNativeFrameDrawing=false;
static bool titleDrawRedirected=false;
static u32 titleDrawSlot=0,titleDrawPrevious=0,titleDrawReturn=0,titleDrawDescriptor=0,titleDrawTrampoline=0;
static void titleRestoreDrawTarget(){
 if(!titleDrawRedirected)return;
 if(!wr(titleDrawSlot,&titleDrawPrevious,sizeof titleDrawPrevious)){fatal=true;executionFailed=true;}
 titleDrawRedirected=false;
}
struct TitleNativeFrameScope {
 TitleNativeFrameScope(){titleNativeFrameDrawing=true;}
 ~TitleNativeFrameScope(){titleRestoreDrawTarget();titleNativeFrameDrawing=false;}
};
static bool titleStoreAvailable=true;
static u32 titleStatusBuffer=0;

static bool titleRefreshStore(){
 if(saveDir.empty())return false;
 auto& current=bossRushUI.titlesStore;destinia::TitlesStore fresh;
 const bool loaded=fresh.load(fs::path(saveDir));
 if(!loaded && (!fresh.canSave() || !current.equippedTitles.empty() || !current.unlockedTitles.empty() || !current.exclusiveTitles.empty()))return false;
 current=std::move(fresh);return true;
}
static void titleEnterExtra(){
 titleStoreAvailable=titleRefreshStore();
 titleMenuMessage=titleStoreAvailable?"":"ERRO AO LER TITULOS";
 if(titleStoreAvailable&&codexAvailable){
  // A title must not outlive the Codex progress that earned it after a crash.
  // Persist pending observations first; on failure, leave unlocks unchanged.
  if(codex.dirty)codexFlush();
  if(codex.dirty || !codexWritable){titleMenuMessage="ERRO AO SALVAR CODEX";return;}
  auto next=bossRushUI.titlesStore;bool changed=false;
  for(const auto id:{destinia::TitleId::Cacador,destinia::TitleId::Complecionista,destinia::TitleId::CacadorDeFungos}){
   if(!next.isUnlocked(titleMenu.slot,id)&&destinia::titleUnlockedByCodex(id,codex,titleMenu.slot)){next.unlock(titleMenu.slot,id);changed=true;}
  }
  if(changed){if(next.save(fs::path(saveDir)))bossRushUI.titlesStore=std::move(next);else {titleStoreAvailable=false;titleMenuMessage="ERRO AO SALVAR TITULO";}}
 }
}

// Replace only the title string drawn by the original status renderer. Its
// original font, clipping, highlight and coordinates remain in use.
static void titleStatusTextHook(uc_engine* engine,uint64_t,uint32_t,void*){
 if((regLR()&~1u)!=SO_BASE+0x31c40 || !titleStatusBuffer)return;
 const auto slot=rd16(exportsMap.at("Game")+0x4886);if(slot>=10)return;
 if(!titleStoreAvailable||!bossRushUI.titlesStore.canSave())return;
 const auto id=bossRushUI.titlesStore.getEquipped(slot);
 if(id==destinia::TitleId::Nenhum||!bossRushUI.titlesStore.isUnlocked(slot,id))return;
 const auto* def=destinia::findTitle(id);if(!def)return;
 std::string name=def->nome;size_t at=0;
 while((at=name.find(u8"ç",at))!=std::string::npos){name.replace(at,2,"c");++at;}
 if(name.size()>120)name.resize(120);
 if(uc_mem_write(engine,titleStatusBuffer,name.c_str(),name.size()+1)!=UC_ERR_OK ||
    uc_reg_write(engine,UC_ARM_REG_R1,&titleStatusBuffer)!=UC_ERR_OK){fatal=true;uc_emu_stop(engine);}
}
static bool titleSelectExtra(destinia::TitleId id){
 const auto g=exportsMap.at("Game"),ui=exportsMap.at("GameUI");const auto slot=rd16(g+0x4886);
 auto& store=bossRushUI.titlesStore;
 if(slot>=10||!titleStoreAvailable||!survivalHost.ready||!destinia::findTitle(id)||!store.isUnlocked(slot,id)||!store.canSave()||saveDir.empty())return false;
 std::vector<uint8_t> gameBefore(32056),uiBefore(12088);
 if(!rd(g,gameBefore.data(),gameBefore.size())||!rd(ui,uiBefore.data(),uiBefore.size()))return false;
 auto previous=store;auto next=store;next.setEquipped(slot,id);next.setExclusive(slot,id!=destinia::TitleId::Nenhum);
 const auto rollback=[&](){store=previous;const bool okay=wr(g,gameBefore.data(),gameBefore.size())&&wr(ui,uiBefore.data(),uiBefore.size());if(!okay){fatal=true;executionFailed=true;}return false;};
 try{
  store=next;const uint16_t none=0;
  if(!wr(g+0x48f0,&none,sizeof none)||!callFn(exportsMap.at("GameUI_calcHeroStat"),{}))return rollback();
  const u32 state=rd32(ui+0x7c);if(state==2){const u32 status=1;if(!wr(ui+0x7c,&status,sizeof status))return rollback();}
  if(!next.save(fs::path(saveDir)))return rollback();
  store=std::move(next);titleMenu.open=false;titleMenuMessage.clear();return true;
 }catch(...){return rollback();}
}

// Let the native routine perform the original selection, but commit the
// sidecar change only after it succeeds. Restore both in-memory objects if a
// concurrent update or write failure prevents saving the extra-title state.
static bool titleSelectOriginal(int raw){
 const auto g=exportsMap.at("Game"),ui=exportsMap.at("GameUI");const auto slot=rd16(g+0x4886);
 auto& store=bossRushUI.titlesStore;if(slot>=10||!store.canSave()||saveDir.empty())return false;
 std::vector<uint8_t> gameBefore(32056),uiBefore(12088);
 if(!rd(g,gameBefore.data(),gameBefore.size())||!rd(ui,uiBefore.data(),uiBefore.size()))return false;
 auto previous=store;auto next=store;next.setEquipped(slot,destinia::TitleId::Nenhum);next.setExclusive(slot,false);
 const auto rollback=[&](){store=previous;const bool okay=wr(g,gameBefore.data(),gameBefore.size())&&wr(ui,uiBefore.data(),uiBefore.size());if(!okay){fatal=true;executionFailed=true;}return false;};
 try{
  store=next;
  if(!callFn(exportsMap.at("MainMenu_mKeyPressed"),{u32(raw)}))return rollback();
  if(!next.save(fs::path(saveDir)))return rollback();
  store=std::move(next);titleMenuMessage.clear();return true;
 }catch(...){return rollback();}
}

static bool titleMenuTouch(int x,int y,int phase,int width,int height,int scale){
 if(titleMenuTouchCaptured && phase!=0){if(phase==1)titleMenuTouchCaptured=false;return true;}
 const auto g=exportsMap.at("Game"),ui=exportsMap.at("GameUI");
 const int mode=int16_t(rd16(g+0x4854)),slot=rd16(g+0x4886),index=int16_t(rd16(ui+0x56));
 const auto state=rd32(ui+0x7c);titleMenu.validate(mode,state,slot);
 const auto p=destinia::nativeCodexTransform(width,height,scale).inverse(x,y);
 if(!titleMenu.open){
  if(phase==0 && destinia::titleExtraNextOriginal(width,height).contains(p.first,p.second) &&
     titleMenu.enter(mode,state,index,slot,7)){titleEnterExtra();titleMenuTouchCaptured=true;return true;}
  return false;
 }
 if(phase==0 && p.second>=(height-240)/2 && p.second<(height-240)/2+25){titleMenu.open=false;return false;}
 if(phase==0){
  titleMenuTouchCaptured=true;const auto hit=destinia::titleExtraHit(x,y,width,height,scale);
  if(hit.kind==destinia::TitleHitKind::Previous){if(titleMenu.selected>=8)titleMenu.selected=(titleMenu.selected/8-1)*8;else{titleMenu.open=false;wr16(ui+0x56,49);}}
  else if(hit.kind==destinia::TitleHitKind::Next){const auto next=(titleMenu.selected/8+1)*8;if(next<titleMenu.count())titleMenu.selected=next;}
  else if(hit.kind==destinia::TitleHitKind::Row && hit.row>=0 && titleMenu.selected/8*8+size_t(hit.row)<titleMenu.count()){
   if(titleMenu.selected==titleMenu.selected/8*8+size_t(hit.row)){
    const auto action=titleMenu.key(17,bossRushUI.titlesStore);
    if(action==destinia::TitleMenuAction::Equip){if(!titleSelectExtra(titleMenu.selectedTitle()))titleMenuMessage="ERRO AO SALVAR TITULO";}
    else if(action==destinia::TitleMenuAction::Locked)titleMenuMessage="TITULO BLOQUEADO";
   }else {titleMenu.selected=titleMenu.selected/8*8+size_t(hit.row);titleMenuMessage.clear();}
  }
 }
 return true;
}

// The native page painter receives its total as the first stack argument.
// Extend that displayed count without changing the original title indices.
static void titleMenuPageCountHook(uc_engine* engine,uint64_t,uint32_t,void*) {
 u32 sp=0;
 const u32 total=7u+u32((titleMenu.count()+7)/8);
 if(uc_reg_read(engine,UC_ARM_REG_SP,&sp)!=UC_ERR_OK ||
    sp<STACK_BASE || sp>STACK_BASE+STACK_SIZE-sizeof(total) ||
    uc_mem_write(engine,sp,&total,sizeof(total))!=UC_ERR_OK){fatal=true;uc_emu_stop(engine);}
}
static void titleMenuLoadHook(uc_engine*,uint64_t,uint32_t,void*){titleLoadPending=true;}

// Draw the native menu into a scratch framebuffer while the added page is
// open. The ARM routine must still run: it updates cursor animation state.
static void titleMenuNativePaintHook(uc_engine* engine,uint64_t,uint32_t,void*){
 if(!titleNativeFrameDrawing||!titleMenu.open)return;
 const auto g=exportsMap.at("Game"),ui=exportsMap.at("GameUI");
 titleMenu.validate(int16_t(rd16(g+0x4854)),rd32(ui+0x7c),rd16(g+0x4886));
 if(!titleMenu.open)return;
 const auto handle=R(0);
 if(titleDrawRedirected||handle>4){fatal=true;uc_emu_stop(engine);return;}
 titleDrawSlot=exportsMap.at("frameBuffer")+handle*4;
 if(uc_reg_read(engine,UC_ARM_REG_LR,&titleDrawReturn)!=UC_ERR_OK||
    !rd(titleDrawSlot,&titleDrawPrevious,sizeof titleDrawPrevious)||
    !titleDrawPrevious||
    !wr(titleDrawSlot,&titleDrawDescriptor,sizeof titleDrawDescriptor)){fatal=true;uc_emu_stop(engine);return;}
 titleDrawRedirected=true;const u32 target=titleDrawTrampoline|1u;
 if(uc_reg_write(engine,UC_ARM_REG_LR,&target)!=UC_ERR_OK){titleRestoreDrawTarget();fatal=true;uc_emu_stop(engine);}
}
static void titleMenuDrawReturnHook(uc_engine* engine,uint64_t,uint32_t,void*){
 if(!titleDrawRedirected){fatal=true;uc_emu_stop(engine);return;}
 const u32 target=titleDrawReturn;titleRestoreDrawTarget();
 if(fatal||uc_reg_write(engine,UC_ARM_REG_LR,&target)!=UC_ERR_OK||uc_reg_write(engine,UC_ARM_REG_PC,&target)!=UC_ERR_OK){fatal=true;uc_emu_stop(engine);}
}

// Defer actions until the ARM frame completes; never re-enter Unicorn in a hook.
static void titleMenuNativeHook(uc_engine* engine,uint64_t,uint32_t,void*) {
 if(titleMenuDraining || !exportsMap.count("Game") || !exportsMap.count("GameUI"))return;
 const auto g=exportsMap.at("Game"),ui=exportsMap.at("GameUI");
 const int mode=int16_t(rd16(g+0x4854)),slot=rd16(g+0x4886),index=int16_t(rd16(ui+0x56));
 const auto state=rd32(ui+0x7c);titleMenu.validate(mode,state,slot);
 const int raw=int(R(0)),directions[]={3,9,5,7};
 const int key=raw>=13&&raw<=16?directions[raw-13]:raw;
 const bool openExtra=mode==16&&state==2&&slot>=0&&slot<10&&index>=48&&index<=49&&key==7;
 const bool selectOriginal=mode==16&&state==2&&slot>=0&&slot<10&&index>=0&&index<50&&
   (key==6||key==17)&&bossRushUI.titlesStore.getEquipped(slot)!=destinia::TitleId::Nenhum&&
   (index==0||rd32(g+0x7758+u32(index)*4)!=0);
 if(!titleMenu.open && !openExtra && !selectOriginal)return;
 if(titleMenuPending<0)titleMenuPending=raw;
 u32 lr=0;
 if(uc_reg_read(engine,UC_ARM_REG_LR,&lr)!=UC_ERR_OK || uc_reg_write(engine,UC_ARM_REG_PC,&lr)!=UC_ERR_OK){fatal=true;uc_emu_stop(engine);}
}

static bool titleDrainNativeMenu(){
 if(titleMenuDraining || (titleMenuPending<0&&!titleLoadPending))return true;
 titleMenuDraining=true;
 struct DrainReset{~DrainReset(){titleMenuDraining=false;}} reset;
 const auto g=exportsMap.at("Game"),ui=exportsMap.at("GameUI");
 if(titleLoadPending){
  titleLoadPending=false;const auto slot=rd16(g+0x4886);auto& store=bossRushUI.titlesStore;
  if(slot<10&&store.canSave()&&store.isExclusive(slot)&&store.isUnlocked(slot,store.getEquipped(slot))&&rd16(g+0x48f0)!=0){
   std::vector<uint8_t> before(32056);if(!rd(g,before.data(),before.size()))return false;
   const uint16_t none=0;
   if(!wr(g+0x48f0,&none,sizeof none)||!callFn(exportsMap.at("GameUI_calcHeroStat"),{})){
    if(!wr(g,before.data(),before.size())){fatal=true;executionFailed=true;}return false;
   }
  }
 }
 if(titleMenuPending<0)return true;
 const int raw=titleMenuPending;titleMenuPending=-1;
 const int mode=int16_t(rd16(g+0x4854)),slot=rd16(g+0x4886),index=int16_t(rd16(ui+0x56));
 const int directions[]={3,9,5,7};const int key=raw>=13&&raw<=16?directions[raw-13]:raw;
 bool ok=true;
 if(!titleMenu.open){
  if((key==6||key==17)&&mode==16&&rd32(ui+0x7c)==2&&index>=0&&index<50&&slot>=0&&slot<10&&bossRushUI.titlesStore.getEquipped(slot)!=destinia::TitleId::Nenhum){
   if(!titleSelectOriginal(raw))titleMenuMessage="ERRO AO SALVAR TITULO";
  }else if(titleMenu.enter(mode,rd32(ui+0x7c),index,slot,key))titleEnterExtra();
 }
 else {
  titleMenu.validate(mode,rd32(ui+0x7c),slot);
  if(titleMenu.open){
   if(key==3||key==9)titleMenuMessage.clear();
   const auto action=titleMenu.key(key,bossRushUI.titlesStore);
   if(action==destinia::TitleMenuAction::Back)wr16(ui+0x56,49);
   else if(action==destinia::TitleMenuAction::Cancel)ok=callFn(exportsMap.at("MainMenu_mKeyPressed"),{27});
   else if(action==destinia::TitleMenuAction::Equip){if(!titleSelectExtra(titleMenu.selectedTitle()))titleMenuMessage="ERRO AO SALVAR TITULO";}
   else if(action==destinia::TitleMenuAction::Locked)titleMenuMessage="TITULO BLOQUEADO";
  }
 }
 return ok;
}

static bool installTitleMenu(){
 if(!exportsMap.count("MainMenu_mKeyPressed")||!exportsMap.count("Font_drawStringChar")||
    !exportsMap.count("_drawMenuStatus")||(exportsMap.at("_drawMenuStatus")&~1u)!=SO_BASE+0x31610||
    !exportsMap.count("GameCanvas_LoadGame")||(exportsMap.at("GameCanvas_LoadGame")&~1u)!=SO_BASE+0x1b9e8||
    !exportsMap.count("GameUI_drawMainMenu")||(exportsMap.at("GameUI_drawMainMenu")&~1u)!=SO_BASE+0x31df8)return false;
 const auto at=exportsMap.at("MainMenu_mKeyPressed")&~1u;uc_hook hook{},pageHook{};
 if(uc_hook_add(uc,&hook,UC_HOOK_CODE,reinterpret_cast<void*>(titleMenuNativeHook),nullptr,at,at)!=UC_ERR_OK)return false;
 const u32 pageAt=SO_BASE+0x2ca04;uint8_t code[2]{};
 if(uc_mem_read(uc,pageAt,code,sizeof code)!=UC_ERR_OK||code[0]!=0x48||code[1]!=0x46)return false;
 if(uc_hook_add(uc,&pageHook,UC_HOOK_CODE,reinterpret_cast<void*>(titleMenuPageCountHook),nullptr,pageAt,pageAt)!=UC_ERR_OK)return false;
 titleStatusBuffer=hmalloc(128);if(!titleStatusBuffer)return false;
 const auto fontAt=exportsMap.at("Font_drawStringChar")&~1u;uc_hook statusHook{};
 if(uc_hook_add(uc,&statusHook,UC_HOOK_CODE,reinterpret_cast<void*>(titleStatusTextHook),nullptr,fontAt,fontAt)!=UC_ERR_OK)return false;
 const auto loadAt=exportsMap.at("GameCanvas_LoadGame")&~1u;uc_hook loadHook{};
 if(uc_hook_add(uc,&loadHook,UC_HOOK_CODE,reinterpret_cast<void*>(titleMenuLoadHook),nullptr,loadAt,loadAt)!=UC_ERR_OK)return false;
 titleDrawDescriptor=hmalloc(12);const u32 scratch=hmalloc(FB_BYTES);titleDrawTrampoline=hmalloc(4);
 if(!titleDrawDescriptor||!scratch||!titleDrawTrampoline)return false;
 const u32 width=SCREEN_W,height=SCREEN_H;const uint16_t bxLr=0x4770;
 if(!wr(titleDrawDescriptor,&width,4)||!wr(titleDrawDescriptor+4,&height,4)||!wr(titleDrawDescriptor+8,&scratch,4)||!wr(titleDrawTrampoline,&bxLr,2))return false;
 uc_hook returnHook{};
 if(uc_hook_add(uc,&returnHook,UC_HOOK_CODE,reinterpret_cast<void*>(titleMenuDrawReturnHook),nullptr,titleDrawTrampoline,titleDrawTrampoline)!=UC_ERR_OK)return false;
 const auto paintAt=exportsMap.at("GameUI_drawMainMenu")&~1u;uc_hook paintHook{};
 return uc_hook_add(uc,&paintHook,UC_HOOK_CODE,reinterpret_cast<void*>(titleMenuNativePaintHook),nullptr,paintAt,paintAt)==UC_ERR_OK;
}
