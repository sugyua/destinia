#pragma once

#include "overlay_font.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <string>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace destinia {

inline uint64_t currentProcessWorkingSetBytes() {
#ifdef _WIN32
  struct Counters {
    DWORD cb;
    DWORD pageFaultCount;
    SIZE_T peakWorkingSetSize;
    SIZE_T workingSetSize;
    SIZE_T quotaPeakPagedPoolUsage;
    SIZE_T quotaPagedPoolUsage;
    SIZE_T quotaPeakNonPagedPoolUsage;
    SIZE_T quotaNonPagedPoolUsage;
    SIZE_T pagefileUsage;
    SIZE_T peakPagefileUsage;
  } counters{};
  using Query = BOOL(WINAPI*)(HANDLE, void*, DWORD);
  static Query query = []() -> Query {
    HMODULE kernel = GetModuleHandleW(L"kernel32.dll");
    auto convert = [](FARPROC proc) -> Query {
      Query fn = nullptr;
      static_assert(sizeof(fn) == sizeof(proc), "Windows procedure pointer size mismatch");
      std::memcpy(&fn, &proc, sizeof(fn));
      return fn;
    };
    auto fn = kernel ? convert(GetProcAddress(kernel, "K32GetProcessMemoryInfo")) : nullptr;
    if (fn) return fn;
    HMODULE psapi = LoadLibraryW(L"psapi.dll");
    return psapi ? convert(GetProcAddress(psapi, "GetProcessMemoryInfo")) : nullptr;
  }();
  if (query) {
    counters.cb = sizeof(counters);
    if (query(GetCurrentProcess(), &counters, sizeof(counters))) return counters.workingSetSize;
  }
#endif
  return 0;
}

struct DiagnosticsStats {
  double fps = 0.0;
  double averageEmulationMs = 0.0;
  uint64_t workingSetBytes = 0;
};

class DiagnosticsPanel {
 public:
  bool visible() const { return shown; }
  void toggle() { shown = !shown; }
  const DiagnosticsStats& stats() const { return current; }

  void recordFrame(double frameMs, double emulationMs, uint64_t workingSetBytes) {
    current.workingSetBytes = workingSetBytes;
    if (!std::isfinite(frameMs) || !std::isfinite(emulationMs) || frameMs <= 0.0 || emulationMs < 0.0)
      return;
    windowMs += frameMs;
    emulationSumMs += emulationMs;
    ++frames;
    if (windowMs >= 1000.0 - 1e-6 && frames) {
      current.fps = frames * 1000.0 / windowMs;
      current.averageEmulationMs = emulationSumMs / frames;
      windowMs = 0.0;
      emulationSumMs = 0.0;
      frames = 0;
    }
  }

  void paint(uint8_t* pixels, int width, int height) const {
    if (!shown || !pixels || width < 24 || height < 28) return;
    const int x = 5, y = 5, panelW = std::min(230, width - 10), panelH = 39;
    if (panelW < 24 || y + panelH > height) return;
    fillRect(pixels, width, height, x, y, panelW, panelH, 0x0841);
    for (int px = x; px < x + panelW; ++px) {
      putPixel(pixels, width, height, px, y, 0x7bef);
      putPixel(pixels, width, height, px, y + panelH - 1, 0x7bef);
    }
    for (int py = y; py < y + panelH; ++py) {
      putPixel(pixels, width, height, x, py, 0x7bef);
      putPixel(pixels, width, height, x + panelW - 1, py, 0x7bef);
    }
    char line1[48], line2[48];
    std::snprintf(line1, sizeof(line1), "FPS %.0f  EMU %.1fMS", current.fps, current.averageEmulationMs);
    std::snprintf(line2, sizeof(line2), "MEM %.1FMB  F12 FECHA", current.workingSetBytes / (1024.0 * 1024.0));
    paintOverlayText(pixels, width, height, x + 7, y + 6, line1, 0xffff);
    paintOverlayText(pixels, width, height, x + 7, y + 22, line2, 0x7bef);
  }

 private:
  bool shown = false;
  DiagnosticsStats current{};
  double windowMs = 0.0;
  double emulationSumMs = 0.0;
  uint64_t frames = 0;

  static void putPixel(uint8_t* pixels, int width, int height, int x, int y, uint16_t color) {
    if (x < 0 || y < 0 || x >= width || y >= height) return;
    const size_t at = (static_cast<size_t>(y) * width + x) * 2;
    pixels[at] = uint8_t(color & 0xff);
    pixels[at + 1] = uint8_t(color >> 8);
  }
  static void fillRect(uint8_t* pixels, int width, int height, int x, int y, int w, int h, uint16_t color) {
    for (int py = y; py < y + h; ++py)
      for (int px = x; px < x + w; ++px) putPixel(pixels, width, height, px, py, color);
  }
};

}  // namespace destinia
