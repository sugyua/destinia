import os
from pathlib import Path
import subprocess
import tempfile
import unittest

SCRIPT = Path(__file__).resolve().parents[1] / 'scripts/compile_windows.sh'

class CompileWindowsTests(unittest.TestCase):
    def run_build(self, target='host_zoom', fail=''):
        with tempfile.TemporaryDirectory(prefix='destinia build ') as directory:
            root = Path(directory)
            for name in ['host', 'host_zoom']:
                (root / name).mkdir()
                (root / name / 'CMakeLists.txt').write_text('# fixture')
            tools = root / 'tools'; tools.mkdir()
            log = root / 'calls'
            scripts = {
                'cygpath': '#!/bin/sh\nfor arg in "$@"; do last="$arg"; done\nprintf "%s\\n" "$last"\n',
                'cmake': '#!/bin/sh\nprintf "%s\\n" "$*" >> "$COMPILE_TEST_LOG"\nif [ "$COMPILE_TEST_FAIL" = configure ] && [ "$1" = -S ]; then exit 7; fi\nif [ "$COMPILE_TEST_FAIL" = build ] && [ "$1" = --build ]; then exit 8; fi\n',
                'g++': '#!/bin/sh\nexit 0\n', 'ninja': '#!/bin/sh\nexit 0\n', 'pkg-config': '#!/bin/sh\nexit 0\n',
            }
            for name, text in scripts.items():
                p = tools / name; p.write_text(text); p.chmod(0o755)
            env = dict(os.environ, PATH=str(tools) + ':' + os.environ['PATH'], COMPILE_TEST_LOG=str(log), COMPILE_TEST_FAIL=fail)
            run = subprocess.run(['bash', str(SCRIPT), str(root), target], env=env, capture_output=True, text=True)
            return run, log.read_text().splitlines() if log.exists() else []

    def test_builds_zoom_in_project_path_with_spaces(self):
        result, calls = self.run_build()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(len(calls), 2)
        self.assertTrue(calls[0].startswith('-S host_zoom -B host_zoom/build -G Ninja'))
        self.assertEqual(calls[1], '--build host_zoom/build')

    def test_builds_both_variants(self):
        result, calls = self.run_build('ambos')
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(len(calls), 4)
        self.assertIn('--build host/build', calls)
        self.assertIn('--build host_zoom/build', calls)

    def test_configuration_failure_prevents_build(self):
        result, calls = self.run_build(fail='configure')
        self.assertEqual(result.returncode, 7)
        self.assertEqual(len(calls), 1)

    def test_build_failure_is_returned_and_stops_second_host(self):
        result, calls = self.run_build('ambos', fail='build')
        self.assertEqual(result.returncode, 8)
        self.assertEqual(len(calls), 2)

    def test_invalid_target_does_not_invoke_cmake(self):
        result, calls = self.run_build('invalido')
        self.assertNotEqual(result.returncode, 0)
        self.assertEqual(calls, [])
