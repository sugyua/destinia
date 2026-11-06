#include "../common/weapons_catalog.h"
#include "../common/weapons_catalog_defs.h"
#include "../common/weapons_catalog_ui.h"
#include "../common/weapons_catalog_host.h"
#include <cassert>
#include <iostream>
#include <vector>
#include <filesystem>

using namespace destinia;

void testDefs() {
  const auto& list = allCatalogWeapons();
  assert(list.size() == 32); // 8 por classe x 4 classes = 32 armas base
  assert(allCatalogEquipment().size() == 64); // 32 armas + 32 armaduras

  auto war = getCatalogWeaponsByClass(1);
  assert(war.size() == 8);

  auto gun = getCatalogWeaponsByClass(2);
  assert(gun.size() == 8);

  auto rog = getCatalogWeaponsByClass(3);
  assert(rog.size() == 8);

  auto mag = getCatalogWeaponsByClass(4);
  assert(mag.size() == 8);
  assert(getCatalogArmorsByClass(4).size() == 8);

  std::cout << "[TEST] Defs canônicas validadas com sucesso: " << list.size() << " armas base catalogadas.\n";
}

void testUIAndPainting() {
  WeaponsCatalog catalog;
  // Registra 5 armas para o Guerreiro no slot 0
  catalog.record(0, 1001, 1, 0, 1); // Comum
  catalog.record(0, 1001, 1, 1, 1); // Mágica
  catalog.record(0, 1001, 1, 2, 1); // Rara
  catalog.record(0, 1001, 1, 3, 1); // Épica
  catalog.record(0, 1001, 1, 4, 1); // Lendária

  WeaponsCatalogUI ui;
  assert(ui.selectedClass == 1);
  assert(ui.page == 0);

  // Testa troca de abas
  ui.nextTab();
  assert(ui.selectedClass == 2);
  ui.nextTab();
  assert(ui.selectedClass == 3);
  ui.nextTab();
  assert(ui.selectedClass == 4);
  ui.nextTab();
  assert(ui.selectedClass == 1);

  // Keyboard/touch tab navigation must include Mago and cycle through four jobs.
  ui.prevTab(); assert(ui.selectedClass == 4);
  ui.nextTab(); assert(ui.selectedClass == 1);
  for(int index=0;index<4;++index){
    const auto r=ui.tabRect(index,400,240);
    assert(r.x>=0 && r.x+r.w<=400);
    assert(ui.handleTouch(r.x+r.w/2,r.y+r.h/2,400,240,8));
    assert(ui.selectedClass==index+1);
  }
  ui.selectedClass=1;


  // Testa buffer de renderização 400x240 (RGB565 = 2 bytes por pixel)
  std::vector<uint8_t> frameBuffer(400 * 240 * 2, 0);
  ui.paint(frameBuffer.data(), 400, 240, 0, catalog, 100);

  // Testa buffer de renderização 800x480
  std::vector<uint8_t> frameBufferZoom(800 * 480 * 2, 0);
  ui.paint(frameBufferZoom.data(), 800, 480, 0, catalog, 100);

  std::cout << "[TEST] Renderização de UI em 400x240 e 800x480 aprovada sem estouro de buffer.\n";
}

void testHostLifecycle() {
  namespace fs = std::filesystem;
  const fs::path testDir = "tests_tmp_catalog_dir";
  fs::create_directories(testDir);

  WeaponsCatalogHost host;
  host.init(testDir);
  assert(host.ready);
  assert(!host.open);

  // Adiciona armas diretamente e salva
  host.catalog.record(0, 2001, 1, 0, 2);
  saveWeaponsCatalog(testDir, host.catalog);

  // Recarrega em uma nova instância para verificar persistência
  WeaponsCatalogHost host2;
  host2.init(testDir);
  assert(host2.catalog.isRecorded(0, 2001, 0));
  assert(!host2.catalog.isRecorded(0, 2001, 1));

  // Testa toggle e teclas
  bool opened = host2.toggle(0, 0, [](uint32_t) -> uint16_t { return 0; });
  assert(opened);
  assert(host2.open);
  assert(host2.handleKey('\t',true)); // Tab must cycle classes, not pages
  assert(host2.ui.selectedClass == 2);
  assert(host2.handleKey('4',true));
  assert(host2.ui.selectedClass == 4);
  assert(host2.handleKey(1073741903,true)); // Right arrow pages, class preserved
  assert(host2.ui.selectedClass == 4);

  // ESC fecha
  assert(host2.handleKey(27, true));
  assert(!host2.open);

  // Limpa diretório temporário
  fs::remove_all(testDir);
  std::cout << "[TEST] Ciclo de vida do host e persistência sidecar validados com 100% de sucesso.\n";
}

int main() {
  std::cout << "=== INICIANDO TESTES DO CATALOGO DE ARMAS (0.11.0) ===\n";
  testDefs();
  testUIAndPainting();
  testHostLifecycle();
  std::cout << "=== TODOS OS TESTES PASSARAM COM SUCESSO! ===\n";
  return 0;
}
