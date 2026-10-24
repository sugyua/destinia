#!/usr/bin/env python3
"""Rebuild the private demo data from the author's extracted Android APK."""
import argparse,hashlib,json,struct
from pathlib import Path
p=argparse.ArgumentParser(description=__doc__);p.add_argument('--game',type=Path,required=True);p.add_argument('--out',type=Path,default=Path('dist/game'));args=p.parse_args()
library=(args.game/'lib/armeabi/libdestinia_jni.so').read_bytes()
if hashlib.sha256(library).hexdigest()!='f12f1d20ab42646ef81df27516851acccb9fffe7be07a054db83890d992df7e9':raise SystemExit('Biblioteca Android diferente da revisada')
entries=[];parts=[];offset=0
for path in sorted((args.game/'assets').rglob('*')):
 if not path.is_file():continue
 if path.is_symlink():raise SystemExit('Link simbolico nao permitido')
 name=path.relative_to(args.game/'assets').as_posix()
 if any(p.lower().startswith('save') for p in Path(name).parts) or path.suffix.lower() in {'.sav','.save','.srm','.log'}:raise SystemExit('Dado pessoal nao permitido')
 data=path.read_bytes();entries.append({'name':name,'offset':offset,'size':len(data)});parts.append(data);offset+=len(data)
manifest=json.dumps(entries,separators=(',',':')).encode();args.out.mkdir(parents=True,exist_ok=True)
(args.out/'assets.bin').write_bytes(struct.pack('<I',len(manifest))+manifest+b''.join(parts));(args.out/'libdestinia_jni.so').write_bytes(library)
print(str(len(entries))+' recursos originais e biblioteca revisados; sem saves ou dados do usuario.')
