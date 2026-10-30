"""Threshold regression with synthetic memory; never reads a personal save."""
from pathlib import Path
import os, subprocess, tempfile, unittest
ROOT=Path(__file__).resolve().parents[1]
class AutoPotionThreshold(unittest.TestCase):
    def test_native_threshold_full_hp_missing_maximum_and_cooldown(self):
        with tempfile.TemporaryDirectory(prefix='destinia-potion-') as folder:
            binary=Path(folder)/('potion.exe' if os.name=='nt' else 'potion')
            build=subprocess.run(['g++','-std=c++17','-O0',str(ROOT/'tests/auto_potion_threshold_cases.cpp'),'-o',str(binary)],capture_output=True,text=True)
            self.assertEqual(build.returncode,0,build.stderr)
            result=subprocess.run([str(binary)],capture_output=True,text=True,timeout=15)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
            self.assertIn('AUTO_POTION_THRESHOLD_CASES_OK',result.stdout)
