"""Exercise menu cropping and pointer transition using the ARM engine and synthetic saves."""
import os
from pathlib import Path
import shlex
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]

@unittest.skipUnless(os.environ.get('DESTINIA_NATIVE_TESTS') == '1', 'Requer SDL2, Unicorn e dados originais')
class ZoomMenuNativeTests(unittest.TestCase):
    def test_menu_click_opens_options_and_restores_full_view(self):
        flags = shlex.split(subprocess.check_output(['pkg-config', '--cflags', '--libs', 'sdl2', 'unicorn', 'zlib'], text=True))
        with tempfile.TemporaryDirectory(prefix='destinia-zoom-menu-native-') as directory:
            base = Path(directory)
            exe = base / 'test.exe'
            subprocess.run(['g++', '-std=c++17', '-O0', str(ROOT / 'tests/zoom_menu_native.cpp'), '-o', str(exe)] + flags, check=True)
            env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy', LOCALAPPDATA=str(base / 'appdata'))
            run = subprocess.run([str(exe), '--game', str(ROOT / 'game'), '--save', str(base / 'save'), '--reports', str(base / 'reports'), '--settings', str(base / 'settings.ini'), '--scale', '2', '--windowed', '--no-save-preferences', '--mute'], env=env, cwd=base, capture_output=True, text=True, timeout=100)
            self.assertEqual(run.returncode, 0, run.stdout + run.stderr)
