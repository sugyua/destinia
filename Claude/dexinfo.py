#!/usr/bin/env python3
"""Leitor minimo de classes.dex: lista classes, campos e metodos (com flags) de um prefixo.
Uso: python3 dexinfo.py classes.dex [prefixo, ex: Lgame/destiniaeng/]"""
import struct, sys

def uleb(b, p):
    r = s = 0
    while True:
        c = b[p]; p += 1
        r |= (c & 0x7f) << s; s += 7
        if not c & 0x80: return r, p

class Dex:
    def __init__(s, path):
        b = s.b = open(path, 'rb').read()
        h = struct.unpack_from('<' + 'I' * 28, b, 0x20)
        (s.nstr, s.ostr, s.ntype, s.otype, s.nproto, s.oproto,
         s.nfield, s.ofield, s.nmeth, s.ometh, s.ncls, s.ocls) = h[6:18]
        s.strs = []
        for i in range(s.nstr):
            off, = struct.unpack_from('<I', b, s.ostr + 4 * i)
            _, p = uleb(b, off)
            e = b.index(b'\0', p)
            s.strs.append(b[p:e].decode('utf8', 'replace'))
        s.types = [s.strs[struct.unpack_from('<I', b, s.otype + 4 * i)[0]] for i in range(s.ntype)]
    def typelist(s, off):
        if not off: return []
        n, = struct.unpack_from('<I', s.b, off)
        return [s.types[struct.unpack_from('<H', s.b, off + 4 + 2 * i)[0]] for i in range(n)]
    def proto(s, i):
        sh, ret, par = struct.unpack_from('<III', s.b, s.oproto + 12 * i)
        return '(' + ''.join(s.typelist(par)) + ')' + s.types[ret]
    def method(s, i):
        c, p, n = struct.unpack_from('<HHI', s.b, s.ometh + 8 * i)
        return s.types[c], s.strs[n], s.proto(p)
    def field(s, i):
        c, t, n = struct.unpack_from('<HHI', s.b, s.ofield + 8 * i)
        return s.types[c], s.strs[n], s.types[t]
    def classes(s):
        for i in range(s.ncls):
            ci, af, sup, itf, src, ann, cd, sv = struct.unpack_from('<8I', s.b, s.ocls + 32 * i)
            yield s.types[ci], af, s.types[sup] if sup != 0xffffffff else None, cd
    def class_members(s, cd):
        if not cd: return [], []
        p = cd
        sf, p = uleb(s.b, p); inf, p = uleb(s.b, p); dm, p = uleb(s.b, p); vm, p = uleb(s.b, p)
        fields = []; ms = []
        for n, kind in ((sf, 'static'), (inf, 'inst')):
            idx = 0
            for _ in range(n):
                d, p = uleb(s.b, p); fl, p = uleb(s.b, p); idx += d
                fields.append((kind, hex(fl), *s.field(idx)[1:]))
        for n in (dm, vm):
            idx = 0
            for _ in range(n):
                d, p = uleb(s.b, p); fl, p = uleb(s.b, p); co, p = uleb(s.b, p); idx += d
                ms.append((hex(fl), *s.method(idx)[1:], 'native' if fl & 0x100 else ''))
        return fields, ms

if __name__ == '__main__':
    d = Dex(sys.argv[1]); pre = sys.argv[2] if len(sys.argv) > 2 else 'Lgame/'
    for name, af, sup, cd in d.classes():
        if not name.startswith(pre): continue
        f, m = d.class_members(cd)
        print(f'\n{name} extends {sup}')
        for x in f: print('   F', *x)
        for x in m: print('   M', *x)
