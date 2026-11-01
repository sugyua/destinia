#pragma once
// The same checked ARM binding is used by both PC hosts and their native tests.
static bool hostUsePotion(int type, const std::vector<uint8_t>& table) {
  try {
    auto readChecked=[](u32 a){uint16_t v=0;if(!rd(a,&v,2))throw destinia::AutoPotionAccessError("Auto-pocao: leitura ARM invalida");return v;};
    auto writeChecked=[](u32 a,uint16_t v){if(!wr(a,&v,2))throw destinia::AutoPotionAccessError("Auto-pocao: escrita ARM invalida");};
    if(!exportsMap.count("_pressedQuickSlot"))return false;
    return destinia::useNativePotion(exportsMap.at("Game"),type,readChecked,writeChecked,
      [](){const bool ok=callFn(exportsMap.at("_pressedQuickSlot"),{0});if(executionFailed||fatal)autoPotion.config.enabled=false;return ok;},
      [&](uint16_t id,int t){return destinia::validPotionData(table,id,t);});
  } catch(const destinia::AutoPotionRestoreError& e) {
    hostReport("[auto-pocao] %s\n",e.what());
    throw; // Never continue with an unverified temporary shortcut.
  } catch(const destinia::AutoPotionAccessError& e) {
    hostReport("[auto-pocao] %s; recurso desativado\n",e.what());
    autoPotion.config.enabled=false;
    autoPotion.lastNotice="AUTO-POCAO DESATIVADA: ERRO";
    autoPotion.noticeTimer=120;
    return false;
  }
}
