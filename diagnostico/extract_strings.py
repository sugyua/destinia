import re, sys, os, glob, json

ROOT = os.path.join(os.path.dirname(__file__), "..")
ASSETS = os.path.join(ROOT, "game", "assets")
MAIN = os.path.join(ROOT, "host_zoom", "main.cpp")

TDT_TARGETS = ["gametxt_en.tdt","gametxt_f_en.tdt","equip_name_en.tdt","equip_head_en.tdt",
    "qitem_name_en.tdt","skill0_desc_en.tdt","skill1_desc_en.tdt","skill2_desc_en.tdt",
    "pet_skill_en.tdt","attr0_desc_en.tdt","attr1_desc_en.tdt","attr2_desc_en.tdt"]
GDT_TARGETS = ["item_en.gdt","quest_en.gdt","quest_hard_en.gdt","mon_data_en.gdt"]

def read(path):
    with open(path, "rb") as f: return f.read()

def parse_tdt(data):
    if len(data) < 2: return []
    count = data[0] | (data[1] << 8); pos = 2; out = []
    for _ in range(count):
        if pos + 2 > len(data): break
        ln = data[pos] | (data[pos+1] << 8); pos += 2
        if pos + ln > len(data): break
        out.append(data[pos:pos+ln].decode("latin-1")); pos += ln
    return out

def parse_gdt(data):
    out = []; pos = 0
    while pos < len(data):
        while pos < len(data) and data[pos] == 0: pos += 1
        start = pos
        while pos < len(data) and data[pos] != 0: pos += 1
        if start == pos or pos == len(data) or pos - start > 512: continue
        field = data[start:pos]
        if all(0x20 <= b <= 0x7e for b in field):
            out.append(field.decode("ascii"))
    return out

# --- chaves ja presentes em portugueseText() ---
src = open(MAIN, encoding="utf-8", errors="replace").read()
m = re.search(r"portugueseText\(\)\s*\{.*?translations\s*=\s*\{(.*?)\n\s*\};", src, re.S)
keys = set()
if m:
    for k, v in re.findall(r'\{"((?:[^"\\]|\\.)*)",\s*"((?:[^"\\]|\\.)*)"\}', m.group(1)):
        keys.add(k)

# --- strings dos assets ---
from collections import defaultdict
by_file = defaultdict(list)
allstr = []
for f in TDT_TARGETS:
    p = os.path.join(ASSETS, f)
    if os.path.exists(p):
        for s in parse_tdt(read(p)):
            if s.strip(): by_file[f].append(s); allstr.append(s)
for f in GDT_TARGETS:
    p = os.path.join(ASSETS, f)
    if os.path.exists(p):
        for s in parse_gdt(read(p)):
            if s.strip(): by_file[f].append(s); allstr.append(s)

distinct = sorted(set(allstr))
missing = [s for s in distinct if s not in keys]

# replica a passada de SUBSTITUICAO DE SUBSTRINGS de translatePortugueseString:
# todas as chaves, maior-primeiro, com fronteira de palavra. Se o resultado == original
# e a string nao e nome de item procedural, ela PERMANECE em ingles = alvo real.
ordered = sorted(keys, key=lambda k: -len(k))
def is_word(c): return c.isalnum() or c == '_'
def substring_pass(s):
    r = s
    for k in ordered:
        if len(k) < 4 and k not in ("HP", "MP"): continue
        at = 0
        while True:
            i = r.find(k, at)
            if i < 0: break
            left = (not k[0].isalnum()) or i == 0 or not is_word(r[i-1])
            end = i + len(k)
            right = (not k[-1].isalnum()) or end == len(r) or not is_word(r[end])
            if left and right:
                r = r[:i] + keys_map[k] + r[i+len(k):]
                at = i + len(keys_map[k])
            else:
                at = i + len(k)
    return r
# mapa key->valor (para a substituicao)
keys_map = {}
if m:
    for k, v in re.findall(r'\{"((?:[^"\\]|\\.)*)",\s*"((?:[^"\\]|\\.)*)"\}', m.group(1)):
        keys_map[k] = v

def has_alpha(s): return re.search(r"[A-Za-z]{2,}", s) is not None
# alvo real: tem letras, NAO e chave exata, e a substituicao de substring NAO mudou nada
targets = [s for s in missing if has_alpha(s) and substring_pass(s) == s]
# separa nomes de item (provavelmente procedurais) dos textos descritivos
PROC_FILES = {"equip_name_en.tdt","equip_head_en.tdt","qitem_name_en.tdt"}
missing_text = targets

print(f"chaves ja em portugueseText(): {len(keys)}")
print(f"strings distintas nos assets-alvo: {len(distinct)}")
print(f"nao sao chave exata: {len(missing)}  (com letras: {len([s for s in missing if has_alpha(s)])})")
print(f"PERMANECEM em ingles apos substrings (alvo real): {len(targets)}")
print("--- por arquivo: distintas / permanecem em ingles ---")
for f in TDT_TARGETS + GDT_TARGETS:
    if f in by_file:
        u = set(by_file[f]); tt = [s for s in u if has_alpha(s) and s not in keys and substring_pass(s) == s]
        tag = " (procedural)" if f in PROC_FILES else ""
        print(f"  {f:22} {len(u):4d} / {len(tt):4d}{tag}")

out = os.path.join(os.path.dirname(__file__), "_missing_en.txt")
with open(out, "w", encoding="utf-8") as fh:
    for s in missing_text: fh.write(s.replace("\n", "\\n") + "\n")
print(f"\nescrito: {out}  ({len(missing_text)} linhas p/ traduzir)")
