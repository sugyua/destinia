"""Original art is read locally; every ARM probe uses temporary saves/config."""
import os
from pathlib import Path
import shlex
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class BossMageTests(unittest.TestCase):
    def test_palette_map_policy_and_centered_bar(self):
        with tempfile.TemporaryDirectory(prefix='destinia mage pure ') as temp:
            for case in ['boss_mage_cases', 'menu_bar_cases']:
                binary = Path(temp) / (case + '.exe')
                subprocess.run(['g++', '-std=c++17', '-Wall', '-Wextra',
                                str(ROOT / 'tests' / (case + '.cpp')), '-o', str(binary), '-lz'], check=True)
                subprocess.run([str(binary), str(ROOT / 'game/assets')], check=True)

    @unittest.skipUnless(os.environ.get('DESTINIA_NATIVE_TESTS') == '1', 'Requires original ARM/SDL2/Unicorn')
    def test_real_arm_both_hosts(self):
        flags = [f for f in shlex.split(subprocess.check_output(
            ['pkg-config', '--cflags', '--libs', 'sdl2', 'unicorn', 'zlib'], text=True)) if f != '-mwindows']
        with tempfile.TemporaryDirectory(prefix='destinia mage ARM ') as temp:
            for variant in ['host', 'host_zoom']:
                case = Path(temp) / variant
                case.mkdir()
                binary = case / 'probe.exe'
                command = ['g++', '-std=c++17', '-O0', str(ROOT / 'tests/boss_mage_native.cpp'), '-o', str(binary)]
                if variant == 'host_zoom':
                    command.append('-DDESTINIA_TEST_ZOOM')
                subprocess.run(command + flags, check=True, capture_output=True)
                save = case / 'save'
                save.mkdir()
                sentinel = save / 'save0.dat'
                sentinel.write_bytes(b'ORIGINAL\0MAGE\xffSAVE')
                env = dict(os.environ, SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy')
                run = subprocess.run([str(binary), '--game', str(ROOT / 'game'), '--save', str(save),
                                      '--settings', str(case / 'config.ini'), '--reports', str(case / 'reports'),
                                      '--headless', '1', '--out', str(case / 'frames'), '--mute', '--no-save-preferences'],
                                     env=env, capture_output=True, text=True, timeout=90)
                self.assertEqual(run.returncode, 0, run.stdout + run.stderr)
                self.assertIn('BOSS_MAGE_ARM_OK', run.stdout)
                self.assertEqual(sentinel.read_bytes(), b'ORIGINAL\0MAGE\xffSAVE')


if __name__ == '__main__':
    unittest.main()
