#!/usr/bin/env python3
import zipfile, hashlib
from pathlib import Path

v = '0.12.0'
prefix = f'destinia-{v}-beta'
out_dir = Path('release/dist_0120')
out_dir.mkdir(parents=True, exist_ok=True)
out_zip = out_dir / f'destinia-{v}-beta-source.zip'

if out_zip.exists():
    out_zip.unlink()

include_dirs = ['common', 'host', 'host_zoom', 'web', 'scripts']
include_files = ['README.md', 'CHANGELOG.md', 'VERSION', 'AGENTS.md', 'Compilar Destinia.bat', 'BUILD_INFO.txt']

with zipfile.ZipFile(out_zip, 'w', compression=zipfile.ZIP_DEFLATED) as z:
    for d in include_dirs:
        for p in Path(d).rglob('*'):
            if p.is_file():
                if any(part in p.parts for part in ['build', 'node_modules', '.git', '__pycache__', '.temp']):
                    continue
                if p.suffix in ['.exe', '.dll', '.o', '.obj']:
                    continue
                rel = p.as_posix()
                target = f'{prefix}/{rel}'
                info = zipfile.ZipInfo(target, (2026, 10, 26, 17, 30, 0))
                info.create_system = 3
                info.external_attr = 0o100755 << 16 if p.suffix in ['.sh', '.bat'] else 0o100644 << 16
                info.compress_type = zipfile.ZIP_DEFLATED
                z.writestr(info, p.read_bytes())

    for f in include_files:
        p = Path(f)
        if p.is_file():
            target = f'{prefix}/{f}'
            info = zipfile.ZipInfo(target, (2026, 10, 26, 17, 30, 0))
            info.create_system = 3
            info.external_attr = 0o100755 << 16 if p.suffix in ['.sh', '.bat'] else 0o100644 << 16
            info.compress_type = zipfile.ZIP_DEFLATED
            z.writestr(info, p.read_bytes())

h = hashlib.sha256(out_zip.read_bytes()).hexdigest()
sha_file = out_zip.with_suffix('.zip.sha256')
sha_file.write_text(f'{h}  {out_zip.name}\n', encoding='utf-8')
print('Source zip gerado com sucesso:')
print('Arquivo:', out_zip)
print('Tamanho:', out_zip.stat().st_size, 'bytes')
print('SHA256:', h)
