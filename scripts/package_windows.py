#!/usr/bin/env python3
"""Pacote Windows x64: somente dois EXEs do port, inicializadores e documentacao."""
import argparse
import hashlib
import os
from pathlib import Path
import re
import struct
import zipfile

FILES=['README.md','INSTALL.md','CHANGELOG.md','THIRD_PARTY.md','VERSION',
       'Abrir Destinia.bat','Abrir Destinia Zoom 800x480.bat','Configurar Destinia.bat',
       'Configurar pasta de saves.bat','scripts/configure_game.ps1','scripts/game_preferences.ps1',
       'scripts/configure_saves.ps1','scripts/shared_saves.ps1','docs/PREFERENCIAS.md',
       'docs/SAVES_E_LOGS.md','docs/ROADMAP.md','docs/RELEASE_0.6.md','release/NOTAS_0.6.md',
       'docs/CODEX.md','docs/CORRECOES_0.7_FIX2.md','docs/INVESTIGACAO_CLASSES.md','docs/RELEASE_0.7.md','release/NOTAS_0.7.md']

def pe64(path):
    data=Path(path).read_bytes()
    if len(data)<64 or data[:2]!=b'MZ':raise ValueError('Executavel Windows invalido')
    offset=struct.unpack_from('<I',data,0x3c)[0]
    if offset+26>len(data) or data[offset:offset+4]!=b'PE\0\0' or struct.unpack_from('<H',data,offset+4)[0]!=0x8664 or struct.unpack_from('<H',data,offset+24)[0]!=0x20b:
        raise ValueError('O pacote exige PE Windows x64 (AMD64)')
    return data

def package(root,output,normal,zoom):
    root,output=Path(root).resolve(),Path(output).resolve()
    version=(root/'VERSION').read_text().strip()
    if not re.fullmatch(r'\d+\.\d+(?:\.\d+)?',version):raise ValueError('Versao invalida')
    prefix='destinia-'+version+'-beta';archive=output/(prefix+'-windows-x64.zip');checksum=archive.with_suffix('.zip.sha256')
    if archive.exists() or checksum.exists():raise FileExistsError('Artefato ja existe; use outra pasta de saida')
    inputs={name:(root/name).read_bytes() for name in FILES}
    inputs['host/build/destinia_host.exe']=pe64(normal)
    inputs['host_zoom/build/destinia_host.exe']=pe64(zoom)
    commit=os.environ.get('GITHUB_SHA','compilacao-local')
    if commit!='compilacao-local' and not re.fullmatch(r'[0-9a-f]{40}',commit):raise ValueError('Commit de build invalido')
    inputs['BUILD_INFO.txt']=('Repositorio: https://github.com/sugyua/destinia\nVersao: '+version+' Beta\nCommit de compilacao: '+commit+'\nPlataforma: Windows x64 / MSYS2 UCRT64\nDependencias externas: consulte README.md e INSTALL.md.\n').encode('utf-8')
    inputs['LEIA_PRIMEIRO.txt']=(
        'Destinia '+version+' Beta - pode apresentar bugs.\n'
        'Instale as dependencias externas listadas em README.md / INSTALL.md.\n'
        'Este pacote nao contem DLLs, instaladores, APK extraido ou saves pessoais.\n'
        'Coloque seus dados originais em game/lib/armeabi/libdestinia_jni.so, game/assets e game/res/raw.\n'
        'Abra Abrir Destinia.bat ou Abrir Destinia Zoom 800x480.bat. Nao precisa compilar este pacote.\n'
        'Configurar Destinia.bat ajusta video, audio, pausas e comandos.\n'
        'Progresso compartilhado: LOCALAPPDATA/Destinia/save ou a pasta global configurada.\n'
    ).encode('utf-8')
    output.mkdir(parents=True,exist_ok=True)
    try:
        with zipfile.ZipFile(archive,'x',compression=zipfile.ZIP_DEFLATED) as zipped:
            for name,data in sorted(inputs.items()):
                info=zipfile.ZipInfo(prefix+'/'+name,(2026,10,21,0,0,0));info.create_system=3;info.external_attr=0o100644<<16;info.compress_type=zipfile.ZIP_DEFLATED
                zipped.writestr(info,data)
        checksum.write_text(hashlib.sha256(archive.read_bytes()).hexdigest()+'  '+archive.name+'\n',encoding='ascii')
    except Exception:
        archive.unlink(missing_ok=True);raise
    return archive

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root',type=Path,default=Path(__file__).resolve().parents[1]);parser.add_argument('--out',type=Path,required=True)
    parser.add_argument('--normal',type=Path,required=True);parser.add_argument('--zoom',type=Path,required=True)
    args=parser.parse_args()
    try:print(package(args.root,args.out,args.normal,args.zoom))
    except (ValueError,OSError) as error:parser.exit(1,str(error)+'\n')
if __name__=='__main__':main()
