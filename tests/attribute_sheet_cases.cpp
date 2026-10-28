#include "../common/attribute_sheet.h"
#include <cassert>
#include <vector>

int main() {
  destinia::CombatStats stats{};
  stats[5] = 12345; stats[6] = 6789; stats[7] = 321; stats[8] = 234;
  stats[9] = 125; stats[10] = 86; stats[11] = 49; stats[12] = 30;
  const auto rows = destinia::secondaryAttributeRows(stats);
  assert(rows.size() == 8);
  assert(rows[0].label == "PV MAX" && rows[0].value == "12345");
  assert(rows[1].label == "PM MAX" && rows[1].value == "6789");
  assert(rows[4].label == "HIT" && rows[4].value == "12.5%");
  assert(rows[5].label == "EVA" && rows[5].value == "8.6%");
  assert(rows[6].label == "CRI" && rows[6].value == "4.9%");
  assert(rows[7].label == "BLD" && rows[7].value == "3.0%");
  destinia::AttributeSheet sheet;
  sheet.toggle();
  assert(sheet.visible());
  std::vector<uint8_t> pixels(400u * 240u * 2u);
  sheet.paint(pixels.data(), 400, 240, stats);
  bool painted = false;
  for (uint8_t value : pixels) painted |= value != 0;
  assert(painted);
  sheet.toggle();
  assert(!sheet.visible());
  return 0;
}
