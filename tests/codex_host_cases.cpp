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

 // Open the real original menu, then finish its animation for this ABI test.
 if(codexMenuVisible()) return 30;
 if(!callFn(exportsMap.at("openMainmenu"),{0}))return 31;
 auto ui=exportsMap.at("GameUI");uint16_t phase=2;wr(ui+0x76,&phase,2);
 if(!codexMenuVisible())return 32;
 if(codexMenuTouch(200,100,0))return 45;
 if(!callFn(exportsMap.at("Java_game_destiniaeng_GameThread_jniTouch"),{ENV_OBJ,0x10,200,100,0}) || !rd16(exportsMap.at("MMain")+0x390c))return 46;
 if(!codexMenuKey(5,true) || !codexMenuKey(17,true) || !codexOpen)return 47;
 if(rd16(exportsMap.at("MMain")+0x390c))return 48;
 codexMenuTouch(200,100,1);codexMenuKey(10,true);codexMenuKey(7,true);
 const u32 state=rd32(ui+0x7c);auto button=destinia::codexMenuButton(SCREEN_W,SCREEN_H);
 if(!codexMenuTouch(button.x+30,button.y+12,0) || !codexOpen)return 33;
 if(!codexMenuTouch(button.x+30,button.y+12,1))return 34;
 if(!codexMenuKey(10,true) || codexOpen || !codexTabSelected || rd32(ui+0x7c)!=state)return 35;
 if(!codexMenuKey(7,true) || codexTabSelected)return 44;
 // The first/last native tabs link to the added book without any invalid ARM ID.
 if(!codexMenuKey(5,true) || !codexTabSelected || rd32(ui+0x7c)!=state)return 36;
 if(!codexMenuKey(17,true) || !codexOpen)return 37;
 if(!codexMenuKey(10,true) || codexOpen)return 38;
 if(!codexMenuKey(7,true) || codexTabSelected || rd32(ui+0x7c)!=0)return 39;
 wr32(ui+0x7c,0x80000000);if(!codexMenuKey(7,true) || !codexTabSelected)return 40;
 if(!codexMenuKey(5,true) || codexTabSelected || rd32(ui+0x7c)!=0x80000000)return 41;
 wr32(ui+0x7c,0x50000001);if(codexMenuVisible() || codexMenuTouch(button.x,button.y,0))return 42;
 // Menu-only entry: nothing is painted over normal gameplay.
 auto original=std::vector<uint8_t>(FB_BYTES,0x35),closed=original;
 destinia::codexPaint(closed.data(),SCREEN_W,SCREEN_H,codex,false,0,true);
 if(closed!=original)return 43;
 codex.select(0);for(int id=1;id<=5;++id)codex.observe(10000+id,id,true);
 codex.observe(11033,33,false);codex.observe(11109,109,true);
 auto progress=codex.entries;std::vector<uint8_t> gameBefore(32056),gameAfter(32056);rd(game,gameBefore.data(),gameBefore.size());
 codexPortraits.clear();codexPage=codexSelection=0;codexChangePage(0);if(codexPortraits.size()!=5)return 50;
 for(int id=1;id<=5;++id)if(codexPortraits.at(id).pixels.empty())return 51;
 codexChangePage(1);if(codexPage!=1 || codexSelection!=32 || codexPortraits.size()!=6 || codexPortraits.at(33).pixels.empty())return 52;
 codexChangePage(1);if(codexPage!=2 || codexSelection!=64 || codexPortraits.size()!=7 || codexPortraits.at(109).pixels.empty())return 53;
 codexChangePage(1);if(codexPage!=2)return 54;codexChangePage(-1);if(codexPage!=1 || codexSelection!=32)return 56;
 // A partial last page preserves the selected column whenever that column exists.
 codexSelection=63;codexPage=1;codexChangePage(1);if(codexSelection!=71 || codexPage!=2)return 57;
 codexPage=codexSelection=0;codexOpen=true;
 if(!codexMenuKey(7,true) || codexSelection!=1 || !codexMenuKey(9,true) || codexSelection!=9)return 58;
 if(!codexMenuKey(5,true) || codexSelection!=8 || !codexMenuKey(3,true) || codexSelection!=0)return 59;
 auto pick=destinia::codexCell(18,SCREEN_W,SCREEN_H);if(!codexMenuTouch(pick.x+10,pick.y+10,0) || codexSelection!=18 || !codexMenuTouch(pick.x+10,pick.y+10,1))return 60;
 auto next=destinia::codexNext(SCREEN_W,SCREEN_H);if(!codexMenuTouch(next.x+10,next.y+10,0) || codexPage!=1 || codexSelection!=50 || !codexMenuTouch(next.x+10,next.y+10,1))return 61;
 // The detail bar is not a pagination control and does not leak to original touch.
 auto details=destinia::codexDetails(SCREEN_W,SCREEN_H);if(!codexMenuTouch(details.x+10,details.y+10,0) || codexPage!=1 || codexSelection!=50 || !codexMenuTouch(details.x+10,details.y+10,1))return 62;
 rd(game,gameAfter.data(),gameAfter.size());if(gameBefore!=gameAfter || codex.entries[0].size()!=progress[0].size() || codex.entries[0][1].kills!=progress[0][1].kills)return 55;
 // Slot with only the last species opens its actual catalog page, not a compressed list.
 codex.select(2);codex.observe(12000,109,false);wr32(game+0x4886,2);codexEnterMenu();if(codexSelection!=72 || codexPage!=2)return 63;
 codex.select(3);wr32(game+0x4886,3);codexEnterMenu();if(codexSelection!=0 || codexPage!=0 || !codex.entries[3].empty())return 64;
 std::puts("CODEX_ARM_OK");uc_close(uc);uc=nullptr;return 0;
}
