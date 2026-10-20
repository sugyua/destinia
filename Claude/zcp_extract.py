#!/usr/bin/env python3
"""Descomprime os containers ZCP da Gamevil (Destinia).
Formato: 'ZCP\\0' | u32 tam_comprimido | u32 tam_descomprimido | zlib stream
Uso: python3 zcp_extract.py <pasta_assets> <pasta_saida>
"""
import sys, struct, zlib, pathlib, collections

def unzcp(b):
    if b[:4] != b'ZCP\0':
        return None
    csz, usz = struct.unpack('<II', b[4:12])
    d = zlib.decompress(b[12:12 + csz])
    assert len(d) == usz, (len(d), usz)
    return d

def main(src, dst):
    src, dst = pathlib.Path(src), pathlib.Path(dst)
    dst.mkdir(parents=True, exist_ok=True)
    stats = collections.defaultdict(collections.Counter)
    for f in sorted(src.iterdir()):
        raw = f.read_bytes()
        try:
            d = unzcp(raw)
        except Exception as e:
            print('ERRO', f.name, e); continue
        out = d if d is not None else raw
        (dst / f.name).write_bytes(out)
        stats[f.suffix][out[:4]] += 1
    for ext, c in stats.items():
        print(ext, [(k.hex(' '), v) for k, v in c.most_common(5)])

if __name__ == '__main__':
    main(*sys.argv[1:3])
