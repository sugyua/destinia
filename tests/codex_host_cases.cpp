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
 // Normal deaths notify quests while state is still 1, before state 2 is written.
 shortAt(0x12,1);wr32(p+0x20,0);
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
 const u32 state=rd32(ui+0x7c);auto button=destinia::nativeCodexRect(285,0,23,24,SCREEN_W,SCREEN_H);
 if(!codexMenuTouch(button.x+10,button.y+12,0) || !codexOpen)return 33;
 if(!codexMenuTouch(button.x+10,button.y+12,1))return 34;
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
 auto pick=destinia::nativeCodexCell(18,SCREEN_W,SCREEN_H);if(!codexMenuTouch(pick.x+10,pick.y+10,0) || codexSelection!=18 || !codexMenuTouch(pick.x+10,pick.y+10,1))return 60;
 auto next=destinia::nativeCodexNext(SCREEN_W,SCREEN_H);if(!codexMenuTouch(next.x+10,next.y+10,0) || codexPage!=1 || codexSelection!=50 || !codexMenuTouch(next.x+10,next.y+10,1))return 61;
 // The detail bar is not a pagination control and does not leak to original touch.
 auto details=destinia::nativeCodexRect(92,160,140,40,SCREEN_W,SCREEN_H);if(!codexMenuTouch(details.x+10,details.y+10,0) || codexPage!=1 || codexSelection!=50 || !codexMenuTouch(details.x+10,details.y+10,1))return 62;
 rd(game,gameAfter.data(),gameAfter.size());if(gameBefore!=gameAfter || codex.entries[0].size()!=progress[0].size() || codex.entries[0][1].kills!=progress[0][1].kills)return 55;
 // Slot with only the last species opens its actual catalog page, not a compressed list.
 codex.select(2);codex.observe(12000,109,false);wr32(game+0x4886,2);codexEnterMenu();if(codexSelection!=72 || codexPage!=2)return 63;
 codex.select(3);wr32(game+0x4886,3);codexEnterMenu();if(codexSelection!=0 || codexPage!=0 || !codex.entries[3].empty())return 64;

 // Action opens native details; categories and drop subpages never reach ARM.
 codex.select(0);wr32(game+0x4886,0);codexEnterMenu();codexSelection=0;codexPage=0;
 const auto beforeDetails=codex.entries[0].at(1).kills;
 if(!codexMenuKey(17,true) || !codexDetailsOpen || codexDetailPage!=0)return 70;
 if(codexDetailInfo.sections.size()!=3 || codexDetailInfo.sections[0].id!="drops")return 71;
 if(!codexMenuKey(9,true) || codexDetailScroll!=1 || codexDetailPage!=0)return 72;
 if(!codexMenuKey(7,true) || codexDetailPage!=1 || codexDetailScroll!=0)return 73;
 if(!codexMenuKey(6,true) || codexDetailPage!=2)return 74;
 codexMenuKey(7,true);if(codexDetailPage!=2)return 75;
 codexMenuKey(10,true);if(codexDetailsOpen || !codexOpen || codexSelection!=0)return 76;
 // The selected unknown species must remain undiscovered after opening info.
 codexSelection=20;codexMenuKey(6,true);if(!codexDetailsOpen || codex.entries[0].count(21))return 77;
 codexMenuKey(10,true);codexSelection=0;codexMenuKey(17,true);
 auto previous=destinia::nativeCodexPrevious(SCREEN_W,SCREEN_H);next=destinia::nativeCodexNext(SCREEN_W,SCREEN_H);
 auto tap=[&](destinia::CodexRect r){codexMenuTouch(r.x+5,r.y+5,0);codexMenuTouch(r.x+5,r.y+5,1);};
 tap(next);if(codexDetailPage!=1)return 78;tap(previous);if(codexDetailPage!=0)return 79;
 tap(destinia::nativeCodexDown(SCREEN_W,SCREEN_H));if(codexDetailScroll!=1)return 80;
 tap(destinia::nativeCodexUp(SCREEN_W,SCREEN_H));if(codexDetailScroll!=0)return 81;
 tap(destinia::nativeCodexBack(SCREEN_W,SCREEN_H));if(codexDetailsOpen || !codexOpen)return 82;
 if(codex.entries[0].at(1).kills!=beforeDetails)return 83;
 // Facts are read only when an actual monster is observed, separately per slot.
 codexOpen=false;codexTabSelected=false;shortAt(8,61);shortAt(0x10,2);shortAt(0x12,1);shortAt(0x1a,50);shortAt(0x1c,50);wr32(p+0x20,170);wr32(p+0x24,170);wr32(p+0x4c,0);wr32(game+0x4878,0);
 uint16_t area=2,map=49;wr(game+0x4870,&area,2);wr(game+0x4872,&map,2);
 if(!callFn(exportsMap.at("drawMonsterOddEff"),{p}))return 84;
 codexSelection=60;codexPage=1;codexOpen=true;codexMenuKey(17,true);
 if(codexDetailInfo.hp.find("170")==std::string::npos || codexDetailInfo.hp.find("2")==std::string::npos)return 85;
 if(!codexFacts[0].count(61) || codexFacts[1].count(61))return 86;
 codexMenuKey(10,true);codexMenuKey(10,true);
 // Real original skin/font rendering on both hosts, independent of pure legacy raster tests.
 if(!callFn(exportsMap.at("GameCanvas_init"),{exportsMap.at("GameCanvas")}))return 90;
 if(!callFn(exportsMap.at("openMainmenu"),{0}))return 91;
 uint16_t nativeAnim=16;wr(ui+0x76,&nativeAnim,2);codexEnterMenu();
 std::vector<std::pair<u32,size_t>> ranges={{game,32056},{ui,12088},{exportsMap.at("MMain"),14632},{exportsMap.at("Grp"),124},{exportsMap.at("frameBuffer"),20}};
 std::vector<std::vector<uint8_t>> snapshots;for(auto item:ranges){snapshots.emplace_back(item.second);rd(item.first,snapshots.back().data(),item.second);}
 std::vector<uint8_t> nativeFrame(FB_BYTES+16,0xab);nativeCodexPainter.paint(nativeFrame.data());if(const char* capture=std::getenv("DESTINIA_CODEX_CAPTURE"))writeBmp(std::string(capture)+std::to_string(SCREEN_W)+".bmp",nativeFrame.data());auto nativeFirst=nativeFrame;nativeCodexPainter.paint(nativeFrame.data());if(nativeFirst!=nativeFrame)return 92;
 for(int scale:{75,100,125}){const int previousScale=preferences.uiScale;preferences.uiScale=scale;nativeCodexPainter.paint(nativeFrame.data());preferences.uiScale=previousScale;}codexEnterDetails();nativeCodexPainter.paint(nativeFrame.data());for(size_t i=0;i<ranges.size();i++){std::vector<uint8_t> check(ranges[i].second);rd(ranges[i].first,check.data(),check.size());if(check!=snapshots[i])return 93;}
 for(auto i=nativeFrame.end()-16;i!=nativeFrame.end();++i)if(*i!=0xab)return 94;
 for(int tab=0;tab<9;tab++){auto rect=destinia::nativeCodexRect(88+22*tab,12,1,1,SCREEN_W,SCREEN_H);codexMenuTouch(rect.x,rect.y,0);codexMenuTouch(rect.x,rect.y,1);if(rd32(ui+0x7c)>>28!=u32(tab))return 95;}
 auto book=destinia::nativeCodexRect(297,12,1,1,SCREEN_W,SCREEN_H);codexMenuTouch(book.x,book.y,0);codexMenuTouch(book.x,book.y,1);if(!codexOpen)return 96;
 std::puts("CODEX_ARM_OK");uc_close(uc);uc=nullptr;return 0;
}
