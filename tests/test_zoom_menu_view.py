"""Verify Zoom menu viewport and pointer coordinate mapping."""
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]

class ZoomMenuViewTests(unittest.TestCase):
    def test_menu_and_other_states_and_pointer_mapping(self):
        with tempfile.TemporaryDirectory(prefix='destinia-zoom-menu-') as directory:
            exe = Path(directory) / 'zoom-menu.exe'
            subprocess.run(['g++', '-std=c++17', str(ROOT / 'tests/zoom_menu_view_cases.cpp'), '-o', str(exe)], check=True)
            subprocess.run([str(exe)], check=True)
