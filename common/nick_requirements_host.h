#pragma once

// In APK 1.0.6 this comparison only chooses the description drawn for the
// selected title. Do not change the unlock flag in the character or save.
static void nickRequirementComparison(uc_engine* engine,uint64_t,uint32_t,void*) {
  uint32_t visible=1;
  if(uc_reg_write(engine,UC_ARM_REG_R3,&visible)!=UC_ERR_OK) {
    fatal=true;
    uc_emu_stop(engine);
  }
}

static bool installNickRequirements() {
  std::vector<uint8_t> library;
  if(!readFile(soPath,library)) return false;
  uint64_t hash=14695981039346656037ull;
  for(auto byte:library) hash=(hash^byte)*1099511628211ull;
  if(hash!=0x74b2547839b88777ull || !exportsMap.count("_drawNickSel") ||
     (exportsMap.at("_drawNickSel")&~1u)!=SO_BASE+0x2c7dc) return false;
  const uint8_t expected[]={0x00,0x2b,0x14,0xd1};
  uint8_t actual[sizeof expected]{};
  const auto at=SO_BASE+0x2ca72;
  if(uc_mem_read(uc,at,actual,sizeof actual)!=UC_ERR_OK ||
     !std::equal(std::begin(expected),std::end(expected),std::begin(actual))) return false;
  uc_hook hook{};
  return uc_hook_add(uc,&hook,UC_HOOK_CODE,reinterpret_cast<void*>(nickRequirementComparison),nullptr,at,at)==UC_ERR_OK;
}
