import importlib.util
from pathlib import Path
import struct
import tempfile
import unittest
import zipfile
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
            with self.assertRaises(FileExistsError):module.package(ROOT,base/'out',exe,exe,exe,exe)
            struct.pack_into('<H',data,0x84,0x14c);exe.write_bytes(data)
            with self.assertRaises(ValueError):module.package(ROOT,base/'bad',exe,exe,exe,exe)
if __name__=='__main__':unittest.main()
