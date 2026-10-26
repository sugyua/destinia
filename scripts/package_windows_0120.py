#!/usr/bin/env python3
import zipfile, os, hashlib
from pathlib import Path

v = '0.12.0'
prefix = f'destinia-{v}-beta'
out_dir = Path('release/dist_0120')
out_dir.mkdir(parents=True, exist_ok=True)
out_zip = out_dir / f'destinia-{v}-beta-windows-x64.zip'

if out_zip.exists():
    out_zip.unlink()

# Atualizar BUILD_INFO.txt
build_info = f"""Repositorio: https://github.com/sugyua/destinia
Versao: {v} Beta
Commit de compilacao: compilacao-local
Plataforma: Windows x64 / MSYS2 UCRT64 (Standalone)
Dependencias externas: runtime de DLLs incluso.
"""
Path('BUILD_INFO.txt').write_text(build_info, encoding='utf-8')

# Copiar estrutura exata de dist_0110
ref_zip = zipfile.ZipFile('release/dist_0110/destinia-0.11.0-beta-windows-x64.zip')

with zipfile.ZipFile(out_zip, 'w', compression=zipfile.ZIP_DEFLATED) as z:
    for entry in ref_zip.infolist():
        rel = entry.filename[len('destinia-0.11.0-beta/'):]
        target_name = f'{prefix}/{rel}'
        
        # Arquivos atualizados locais
        if rel == 'Destinia.exe' or rel == 'host/build/destinia_host.exe':
            data = Path('host/build/destinia_host.exe').read_bytes()
        elif rel == 'Destinia Zoom.exe' or rel == 'host_zoom/build/destinia_host.exe':
            data = Path('host_zoom/build/destinia_host.exe').read_bytes()
        elif rel == 'VERSION':
            data = Path('VERSION').read_bytes()
        elif rel == 'README.md':
            data = Path('README.md').read_bytes()
        elif rel == 'CHANGELOG.md':
            data = Path('CHANGELOG.md').read_bytes()
        elif rel == 'BUILD_INFO.txt':
            data = Path('BUILD_INFO.txt').read_bytes()
        elif rel.startswith('release/'):
            local_rel = Path(rel)
            if local_rel.is_file():
                data = local_rel.read_bytes()
            else:
                data = ref_zip.read(entry)
        else:
            local_p = Path(rel)
            if local_p.is_file():
                data = local_p.read_bytes()
            else:
                data = ref_zip.read(entry)
                
        info = zipfile.ZipInfo(target_name, (2026, 10, 26, 17, 30, 0))
        info.create_system = 3
        info.external_attr = 0o100755 << 16 if rel.endswith(('.exe', '.bat')) else 0o100644 << 16
        info.compress_type = zipfile.ZIP_DEFLATED
        z.writestr(info, data)
        
    # Incluir novas notas de 0.12.0
    notas_0120 = Path('release/NOTAS_0.12.0.md')
    if notas_0120.is_file():
        info = zipfile.ZipInfo(f'{prefix}/release/NOTAS_0.12.0.md', (2026, 10, 26, 17, 30, 0))
        info.create_system = 3
        info.external_attr = 0o100644 << 16
        info.compress_type = zipfile.ZIP_DEFLATED
        z.writestr(info, notas_0120.read_bytes())

h = hashlib.sha256(out_zip.read_bytes()).hexdigest()
sha_file = out_zip.with_suffix('.zip.sha256')
sha_file.write_text(f'{h}  {out_zip.name}\n', encoding='utf-8')
print('Gerado com sucesso:')
print('Arquivo:', out_zip)
print('Tamanho:', out_zip.stat().st_size, 'bytes')
print('SHA256:', h)
