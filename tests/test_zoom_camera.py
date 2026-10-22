"""Original ARM camera writers, grey margins, viewport and HUD with temporary saves."""
import os,pathlib,shlex,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
@unittest.skipUnless(os.environ.get('DESTINIA_NATIVE_TESTS')=='1','Requer SDL2, Unicorn e dados originais')
class ZoomCameraTests(unittest.TestCase):
 def test_original_camera_writers_small_large_maps_and_fullscreen(self):
  flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','sdl2','unicorn','zlib'],text=True))
  with tempfile.TemporaryDirectory(prefix='destinia camera ') as directory:
   base=pathlib.Path(directory);exe=base/'camera.exe'
   built=subprocess.run(['g++','-std=c++17','-O0',str(ROOT/'tests/zoom_native_smoke.cpp'),'-o',str(exe)]+flags,capture_output=True,text=True)
   self.assertEqual(built.returncode,0,built.stderr)
   for fullscreen in (False,True):
    env=dict(os.environ,SDL_VIDEODRIVER='dummy',SDL_AUDIODRIVER='dummy',LOCALAPPDATA=str(base/'local'))
    if fullscreen:env['DESTINIA_TEST_FULLSCREEN']='1'
    run=subprocess.run([str(exe),'--game',str(ROOT/'game'),'--save',str(base/str(fullscreen)/'save'),'--reports',str(base/str(fullscreen)/'reports'),'--settings',str(base/'settings.ini'),'--no-save-preferences','--mute'],capture_output=True,text=True,env=env,cwd=base,timeout=90)
    self.assertEqual(run.returncode,0,run.stdout+run.stderr)
    self.assertIn('camera ARM centralizada e fundo cinza',run.stderr)
