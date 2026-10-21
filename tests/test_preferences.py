import os
from pathlib import Path
import shlex
import subprocess
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[1]

class PreferencesTests(unittest.TestCase):
    def test_preferences_and_input_actions(self):
        self.assertTrue((ROOT/'common/user_settings.h').exists(),'Preferencias da 0.6 ainda nao implementadas')
        with tempfile.TemporaryDirectory(prefix='destinia preferences ') as directory:
            binary=Path(directory)/('preferences.exe' if os.name=='nt' else 'preferences')
            flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','sdl2'],text=True))
            run=subprocess.run(['g++','-std=c++17',str(ROOT/'tests/preferences_cases.cpp'),'-o',str(binary)]+flags,capture_output=True,text=True)
            self.assertEqual(run.returncode,0,run.stderr)
            result=subprocess.run([str(binary),str(Path(directory)/'dados')],capture_output=True,text=True,timeout=30)
            self.assertEqual(result.returncode,0,result.stderr)

if __name__=='__main__': unittest.main()
