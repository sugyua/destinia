#include "auto_potion_safe_cases.cpp"

// Teste de regressão: fallback do notice (PR #10)
// Garante que paintNotice desenha via raster quando originalTextPainter recusa.
static int testNoticeFallback() {
  destinia::AutoPotionController ctrl;
  ctrl.lastNotice = "AUTO-POCAO: PV RECUPERANDO!";
  ctrl.noticeTimer = 60;

  std::vector<uint8_t> pixels(400 * 240 * 2, 0);

  // Painel original aceita, mas pintor de texto recusa -> deve usar raster
  destinia::originalPanelPainter = [](uint8_t*, int, int, int, int, int, int) { return true; };
  destinia::originalTextPainter  = [](uint8_t*, int, int, int, int, const std::string&, uint16_t, int, int) { return false; };

  ctrl.paintNotice(pixels.data(), 400, 240);

  // Algum pixel deve ter sido escrito pelo raster
  bool anyWritten = std::any_of(pixels.begin(), pixels.end(), [](uint8_t b) { return b != 0; });
  return anyWritten ? 0 : 1;
}

static int __noticeFallbackResult = (testNoticeFallback() == 0) ? 0 : ([] { exit(99); return 0; }());
