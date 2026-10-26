"""Portable C++/web regression runner. No original game assets or saves needed."""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class ProgressionTests(unittest.TestCase):
    def test_native_and_web_parity(self):
        self.assertIsNotNone(shutil.which('g++'), 'g++ must be on PATH')
        self.assertIsNotNone(shutil.which('node'), 'Node must be on PATH')
        with tempfile.TemporaryDirectory(prefix='destinia-progression-') as temp:
            binary = Path(temp) / ('cases.exe' if os.name == 'nt' else 'cases')
            subprocess.run(['g++', '-std=c++17', '-O2', '-Wall', '-Wextra',
                            str(ROOT / 'tests/progression_cases.cpp'), '-o', str(binary)],
                           cwd=ROOT, check=True)
            subprocess.run([str(binary)], cwd=ROOT, check=True)
            env = dict(os.environ, PROGRESSION_CPP=str(binary))
            subprocess.run(['node', '--test', 'web/tests/progression.test.mjs'],
                           cwd=ROOT, env=env, check=True)


if __name__ == '__main__':
    unittest.main()
