#include <cassert>
#include <cstdint>
#include <vector>
#include <iostream>
#include "../common/auto_potion.h"

void test_auto_potion() {
    destinia::AutoPotionController autoPotion;
    autoPotion.config.enabled = true; // padrao agora e false (IDs ainda sao placeholder); teste liga explicitamente
    std::vector<uint8_t> mem(0x5000, 0);

    auto rd16 = [&](uint32_t addr) -> uint16_t { return *reinterpret_cast<uint16_t*>(&mem[addr]); };
    auto wr16 = [&](uint32_t addr, uint16_t val) { *reinterpret_cast<uint16_t*>(&mem[addr]) = val; };

    uint32_t gameBase = 0;
    
    // Setup Game State (In-Game)
    wr16(0x4854, 0);
    
    // Setup HP: cur=10, max=100 (10% -> low)
    wr16(0x48f2, 10);
    wr16(0x48c4, 100);
    
    // Setup MP: cur=50, max=100 (50% -> ok)
    wr16(0x48f6, 50);
    wr16(0x48c6, 100);

    // Setup Inventory
    uint32_t bag1 = 0x433c + 0x92;
    wr16(bag1, 2); // count = 2 items

    // Item 0: HP Potion, Qty 1
    uint32_t rec0 = bag1 + 2 + 6*0;
    wr16(rec0 + 0, destinia::HP_POTION_ID);
    wr16(rec0 + 4, 1);

    // Item 1: MP Potion, Qty 1
    uint32_t rec1 = bag1 + 2 + 6*1;
    wr16(rec1 + 0, destinia::MP_POTION_ID);
    wr16(rec1 + 4, 1);

    // Run update
    bool used = autoPotion.update(gameBase, rd16, wr16);
    
    // Assertions
    assert(used == true && "Deveria ter usado uma poção");
    assert(rd16(rec0 + 4) == 0 && "Poção de HP deve ter sido consumida");
    assert(rd16(rec1 + 4) == 1 && "Poção de MP NÃO deve ter sido consumida");
    assert(rd16(0x48f2) > 10 && "HP deve ter sido restaurado");

    std::cout << "[PASS] Auto-Potion Test\n";
}

int main() {
    test_auto_potion();
    return 0;
}
