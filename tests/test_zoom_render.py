"""Smoke real do host_zoom: DESTINIA_ZOOM_EXE=/caminho/do/binario python -m unittest ..."""
import os
from pathlib import Path
import struct
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]

@unittest.skipUnless(os.environ.get('DESTINIA_ZOOM_EXE'), 'Requer host_zoom compilado e game original')
class ZoomRenderTests(unittest.TestCase):
    def test_native_framebuffer_and_capture_preserve_original_resolution(self):
        exe = Path(os.environ['DESTINIA_ZOOM_EXE']).resolve()
        with tempfile.TemporaryDirectory(prefix='destinia-render-') as directory:
            output = Path(directory) / 'frames'
            save = Path(directory) / 'save'
            run = subprocess.run([str(exe), '--game', str(ROOT / 'game'), '--save', str(save),
                                  '--headless', '10', '--out', str(output), '--mute', '--lang', 'pt',
                                  '--trace-camera'], capture_output=True, text=True, timeout=120, cwd=ROOT)
            self.assertEqual(run.returncode, 0, run.stderr[-3000:])
            self.assertIn('initGame ok em 400x240', run.stderr)
            for frame in [1, 10]:
                bmp = (output / ('frame_%05d.bmp' % frame)).read_bytes()
                self.assertEqual(bmp[:2], b'BM')
                self.assertEqual(struct.unpack_from('<ii', bmp, 18), (400, 240))
                self.assertEqual(len(bmp), struct.unpack_from('<I', bmp, 2)[0])
                pixels = bmp[struct.unpack_from('<I', bmp, 10)[0]:]
                self.assertTrue(any(pixels), 'Captura inteiramente preta')

if __name__ == '__main__':
    unittest.main()
