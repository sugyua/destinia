#include "../common/diagnostics_panel.h"
#include <cassert>
#include <cmath>
#include <cstdint>
#include <vector>

int main() {
#ifdef _WIN32
  assert(destinia::currentProcessWorkingSetBytes() > 0);
#endif
  destinia::DiagnosticsPanel panel;
  assert(!panel.visible());
  panel.toggle();
  assert(panel.visible());
  for (int i = 0; i < 60; ++i) panel.recordFrame(1000.0 / 60.0, 4.0, 2u * 1024u * 1024u);
  const auto stats = panel.stats();
  assert(std::abs(stats.fps - 60.0) < 0.1);
  assert(std::abs(stats.averageEmulationMs - 4.0) < 0.01);
  assert(stats.workingSetBytes == 2u * 1024u * 1024u);
  panel.recordFrame(1000.0 / 60.0, 4.0, 0);
  assert(panel.stats().workingSetBytes == 0);
  panel.recordFrame(-1.0, std::nan(""), 0);
  assert(std::abs(panel.stats().fps - 60.0) < 0.1);
  std::vector<uint8_t> pixels(400u * 240u * 2u);
  panel.paint(pixels.data(), 400, 240);
  bool painted = false;
  for (uint8_t value : pixels) painted |= value != 0;
  assert(painted);
  panel.toggle();
  assert(!panel.visible());
  return 0;
}
