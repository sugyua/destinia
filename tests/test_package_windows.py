import importlib.util
from pathlib import Path
import struct
import tempfile
import unittest
import zipfile
import shutil
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('package_windows',ROOT/'scripts/package_windows.py')

class WindowsPackageTests(unittest.TestCase):
    def test_runtime_allowlist_and_rejects_wrong_machine(self):
        self.assertTrue(Path(spec.origin).exists(),'Empacotador Windows ausente')
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        with tempfile.TemporaryDirectory() as directory:
            base=Path(directory);exe=base/'host.exe'
            data=bytearray(256);data[:2]=b'MZ';struct.pack_into('<I',data,0x3c,0x80);data[0x80:0x84]=b'PE\0\0';struct.pack_into('<H',data,0x84,0x8664);struct.pack_into('<H',data,0x98,0x20b);exe.write_bytes(data)
            archive=module.package(ROOT,base/'out',exe,exe,exe,exe)
            with zipfile.ZipFile(archive) as zipped:
                names=zipped.namelist()
                self.assertEqual(sum(name.endswith('.exe') for name in names),4)
                self.assertTrue(any(name.endswith('/Destinia.exe') for name in names))
                self.assertTrue(any(name.endswith('/Destinia Zoom.exe') for name in names))
                self.assertTrue(any(name.endswith('/host/build/destinia_host.exe') for name in names))
                self.assertTrue(any(name.endswith('/host_zoom/build/destinia_host.exe') for name in names))
                self.assertFalse(any('/game/' in name or name.endswith(('.dll','.so','.dat','.log')) for name in names))
                self.assertTrue(any(name.endswith('/Configurar Destinia.bat') for name in names))
                version=(ROOT/'VERSION').read_text().strip()
                self.assertTrue(any(name.endswith('/release/NOTAS_'+version+'.md') for name in names),'Notas da versao atual ausentes')
                self.assertTrue(any(name.endswith('/docs/VERIFICACAO_'+version+'.md') for name in names),'Relatorio de verificacao atual ausente')
            with self.assertRaises(FileExistsError):module.package(ROOT,base/'out',exe,exe,exe,exe)
            struct.pack_into('<H',data,0x84,0x14c);exe.write_bytes(data)
            with self.assertRaises(ValueError):module.package(ROOT,base/'bad',exe,exe,exe,exe)

    def test_current_notes_required_and_external_dll_excluded(self):
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        with tempfile.TemporaryDirectory() as directory:
            base=Path(directory);root=base/'source';root.mkdir()
            for name in module.FILES:
                target=root/name;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(ROOT/name,target)
            (root/'VERSION').write_text('9.9.9\n')
            (root/'SDL2.dll').write_bytes(b'NOT FOR DISTRIBUTION')
            exe=base/'host.exe';data=bytearray(256);data[:2]=b'MZ';struct.pack_into('<I',data,0x3c,0x80);data[0x80:0x84]=b'PE\0\0';struct.pack_into('<H',data,0x84,0x8664);struct.pack_into('<H',data,0x98,0x20b);exe.write_bytes(data)
            with self.assertRaises(FileNotFoundError):module.package(root,base/'missing',exe,exe,exe,exe)
            self.assertFalse((base/'missing').exists())
            (root/'release/NOTAS_9.9.9.md').write_text('Beta test notes')
            archive=module.package(root,base/'out',exe,exe,exe,exe)
            with zipfile.ZipFile(archive) as z:
                self.assertFalse(any(n.endswith('.dll') for n in z.namelist()))
                self.assertEqual(z.read('destinia-9.9.9-beta/release/NOTAS_9.9.9.md'),b'Beta test notes')
if __name__=='__main__':unittest.main()
