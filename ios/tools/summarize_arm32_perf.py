#!/usr/bin/env python3
"""Summarize reproducible Unicorn-TCI ARM32 renderer benchmarks.

The game framebuffer is checked byte-for-byte in the workflow, and the
samples are host-macOS timings, NOT FPS estimates for the iPhone.
"""
from __future__ import annotations
import re
import statistics
import sys
from pathlib import Path

FRAME = re.compile(r"ARM32_FRAME_PERF frame=(\d+) exec_ms=(\d+) wait_ms=(\d+)")
HOT = re.compile(r"ARM32_HOT_FUNCTION rank=(\d+) blocks=(\d+) name=(.+)")
def load(path: str):
    text = Path(path).read_text(errors="replace")
    frames = {int(a): (int(b), int(c)) for a,b,c in FRAME.findall(text)}
    hot = [(int(rank),int(blocks),name) for rank,blocks,name in HOT.findall(text)]
    if len(frames) < 49:
        raise ValueError(f"{path}: expected 49+ frames, got {len(frames)}")
    return frames,hot

def median(values):
    return round(statistics.median(values),2) if values else None

def report(label, frames, hot):
    print(f"\n{label}: {len(frames)} quadros amostrados no runner macOS")
    for name,span in [("boot inicial",range(1,46)),
                      ("primeiro carregamento",[46]),
                      ("splash / menu",range(47,50))]:
        samples=[frames[i][0] for i in span if i in frames]
        print(f"  {name:21s}: mediana {median(samples)} ms; "
              f"max {max(samples) if samples else '-'} ms")
    print("  Top 12 rotinas ARM32:")
    for rank,blocks,name in hot[:12]:
        print(f"    {rank:2d}. {blocks:>9d} instrucoes; {name}")

def main():
    if len(sys.argv) not in (3,4):
        raise SystemExit(
            "Uso: summarize_arm32_perf.py baseline.log optimized.log [direct.log]")
    (base,h1),(opt,h2)=[load(p) for p in sys.argv[1:3]]
    report("MODO BASE",base,h1)
    report("ACELERACAO NATIVA",opt,h2)
    indices=range(47,50)
    before=[base[i][0] for i in indices if i in base]
    after=[opt[i][0] for i in indices if i in opt]
    if before and after and median(after):
        print(f"\nSpeedup (splash 47..49): {median(before)/median(after):.2f}x")
    if len(sys.argv)==4:
        direct,hot=load(sys.argv[3])
        report("HEAP NATIVO MAPEADO",direct,hot)
        for group,indices in (("boot",range(1,46)),("splash",range(47,50))):
            a=[opt[i][0] for i in indices if i in opt]
            b=[direct[i][0] for i in indices if i in direct]
            if a and b and median(b):
                print(f"Mapeamento direto ({group}): {median(a)/median(b):.2f}x")
    print("Medicoes do macOS; iPhone requer medicao no dispositivo.")
if __name__=="__main__":
    main()
