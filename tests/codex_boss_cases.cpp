#include "common/codex_details.h"
#include "common/codex_ui.h"
#include "common/codex_native_layout.h"
#include <cassert>
#include <iostream>

int main() {
    using namespace destinia;
    
    // 1. Verificacao de classificacao nativa: species >= 100 e chefe
    assert(!isCodexBoss(1));
    assert(!isCodexBoss(61));
    assert(isCodexBoss(100));
    assert(isCodexBoss(105));
    
    // 2. Abas de filtro na UI
    auto tab0 = codexFilterTab(0, 800, 480);
    auto tab1 = codexFilterTab(1, 800, 480);
    auto tab2 = codexFilterTab(2, 800, 480);
    
    assert(codexFilterHit(tab0.x + 2, tab0.y + 2, 800, 480) == 0);
    assert(codexFilterHit(tab1.x + 2, tab1.y + 2, 800, 480) == 1);
    assert(codexFilterHit(tab2.x + 2, tab2.y + 2, 800, 480) == 2);
    assert(codexFilterHit(tab0.x - 10, tab0.y, 800, 480) == -1);
    
    // 3. Abas nativas de filtro (layout Destinia Zoom)
    auto nTab0 = nativeCodexFilterTab(0, 800, 480);
    auto nTab1 = nativeCodexFilterTab(1, 800, 480);
    auto nTab2 = nativeCodexFilterTab(2, 800, 480);
    
    assert(nativeCodexFilterHit(nTab0.x + 2, nTab0.y + 2, 800, 480) == 0);
    assert(nativeCodexFilterHit(nTab1.x + 2, nTab1.y + 2, 800, 480) == 1);
    assert(nativeCodexFilterHit(nTab2.x + 2, nTab2.y + 2, 800, 480) == 2);
    assert(nativeCodexFilterHit(nTab0.x - 10, nTab0.y, 800, 480) == -1);
    
    std::cout << "CODEX_BOSS_CASES_OK" << std::endl;
    return 0;
}
