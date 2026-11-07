// Synthetic-only regression: never opens the player's original game saves.
#include <map>
#include "../common/weapons_catalog_ui.h"
#include "../common/bounties_host.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

static void assertUnchanged(const std::filesystem::path& root) {
    std::ifstream in(root / "save0.dat", std::ios::binary);
    std::string contents((std::istreambuf_iterator<char>(in)),{});
    assert(contents == "PERSONAL_SAVE_SENTINEL");
}

int main(int argc,char** argv) {
    assert(argc==2);
    namespace fs=std::filesystem;
    const fs::path root(argv[1]);
    fs::create_directories(root);
    {std::ofstream original(root/"save0.dat",std::ios::binary);original<<"PERSONAL_SAVE_SENTINEL";}
    destinia::BountiesHost host;
    host.init(root);host.toggle();
    auto& first=host.manager.getContracts(0).at(0);
    first.completed=true;first.currentCount=first.targetCount;
    auto& second=host.manager.getContracts(1).at(0);
    second.completed=true;second.currentCount=second.targetCount;
    auto claim=[&](unsigned slot)->bool {
        int rewards=0;
        const auto action=[&](uint32_t gold,uint16_t cash){assert(gold==15000);assert(cash==200);++rewards;};
        if(slot==0)host.handleKey(13,true,slot,action);
        else {
            auto rect=host.ui.claimButton(0,400,240);
            host.handleTouch(rect.x+2,rect.y+2,0,400,240,slot,action);
        }
        return rewards!=0;
    };

    // A directory occupying the sidecar path guarantees a disk write error.
    // Neither keyboard nor touch must credit the player or mark it claimed.
    fs::create_directory(root/"bounties-v1.txt");
    assert(!host.onMonsterKilled(2,61));
    assert(host.manager.contractsBySlot.count(2)==0);
    assert(!claim(0));assert(!host.manager.getContracts(0)[0].claimed);
    assert(!claim(1));assert(!host.manager.getContracts(1)[0].claimed);
    assert(fs::is_directory(root/"bounties-v1.txt"));
    assertUnchanged(root);
    fs::remove(root/"bounties-v1.txt");

    // Keyboard and touch must both persist BEFORE paying, exactly once.
    assert(claim(0));assert(host.manager.getContracts(0)[0].claimed);
    assert(!claim(0));
    auto saved=destinia::loadBounties(root);assert(saved.getContracts(0)[0].claimed);
    assert(claim(1));assert(host.manager.getContracts(1)[0].claimed);
    assert(!claim(1));
    saved=destinia::loadBounties(root);assert(saved.getContracts(1)[0].claimed);
    // Inject an OS replace error; old sidecar must remain byte-identical.
    struct FailReplace : destinia::AtomicSaveOps {
      void replace(const fs::path&,const fs::path&) const override {
        throw std::runtime_error("synthetic disk failure");
      }
    } blocked;
    const auto path=root/"bounties-v1.txt";
    auto loadBytes=[&]() {
      std::ifstream in(path,std::ios::binary);
      return std::string((std::istreambuf_iterator<char>(in)),{});
    };
    const auto oldBytes=loadBytes();
    assert(!destinia::saveBounties(root,host.manager,blocked));
    assert(loadBytes()==oldBytes);
    for(const auto& entry:fs::directory_iterator(root))
      assert(entry.path().extension()!=".tmp");
    // A malformed/empty sidecar must be preserved and disable claims.
    {std::ofstream bad(path,std::ios::trunc);}
    destinia::BountiesHost damaged;
    damaged.init(root);
    assert(!damaged.ready && !damaged.toggle());
    assert(fs::file_size(path)==0);
    assertUnchanged(root);
    std::cout<<"BOUNTIES_REGRESSION_PASS\n";
    return 0;
}
