#define SDL_MAIN_HANDLED
#define main destiniaEntry
#ifdef DESTINIA_TEST_ZOOM
#include "../host_zoom/main.cpp"
#else
#include "../host/main.cpp"
#endif
#undef main
int main(int argc,char** argv) {
 int result=runHost(argc,argv);if(result || !codexAvailable) return 20;
 auto game=exportsMap.at("Game");wr32(game+0x4886,0);wr32(game+0x4878,0);
 u32 p=DATA_BASE+0x4000;std::vector<uint8_t> blank(136);wr(p,blank.data(),blank.size());
 auto shortAt=[&](u32 at,uint16_t v){wr(p+at,&v,2);};
 shortAt(8,1);shortAt(0x12,1);shortAt(0x1a,50);shortAt(0x1c,50);wr32(p+0x20,10);
 if(!callFn(exportsMap.at("drawMonsterOddEff"),{p}) || codex.entries[0].at(1).kills!=0) return 21;
 shortAt(0x12,2);wr32(p+0x20,0);
 if(!callFn(exportsMap.at("checkQuestMonster"),{p})) return 22;
 if(!callFn(exportsMap.at("checkQuestMonster"),{p}) || codex.entries[0].at(1).kills!=1) return 23;
 shortAt(8,4);shortAt(0x1a,65000);shortAt(0x12,1);wr32(p+0x20,10);
 if(!callFn(exportsMap.at("drawMonsterOddEff"),{p}) || codex.entries[0].count(4)) return 24;
 if(!callFn(exportsMap.at("GameCanvas_LoadGame"),{1,0}) || codex.slot!=1) return 28;
 codexFlush();destinia::Codex loaded;loaded.names=codex.names;loaded.load(fs::path(saveDir)/"codex-v1.txt");
 if(loaded.entries[0].at(1).kills!=1) return 25;
 std::vector<uint8_t> panel(FB_BYTES+16,0);std::fill(panel.end()-16,panel.end(),0xab);destinia::codexPaint(panel.data(),SCREEN_W,SCREEN_H,codex,true,0,true);
 for(auto i=panel.end()-16;i!=panel.end();++i)if(*i!=0xab)return 26;
 if(std::all_of(panel.begin(),panel.end()-16,[](uint8_t b){return !b;}))return 27;
 std::puts("CODEX_ARM_OK");uc_close(uc);uc=nullptr;return 0;
}
