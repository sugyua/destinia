import os,pathlib,shlex,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
@unittest.skipUnless(os.environ.get('DESTINIA_NATIVE_TESTS')=='1','Requer ARM original, SDL2 e Unicorn')
class OfflineStoreTests(unittest.TestCase):
 def test_purchases_release_network_without_credit_with_or_without_callback(self):
  flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','sdl2','unicorn','zlib'],text=True))
  with tempfile.TemporaryDirectory(prefix='destinia offline ') as directory:
   base=pathlib.Path(directory)
   for variant in ('host','host_zoom'):
    exe=base/(variant+'.exe');cmd=['g++','-std=c++17',str(ROOT/'tests/offline_store_native.cpp'),'-o',str(exe)]
    if variant=='host_zoom':cmd+=['-DDESTINIA_TEST_ZOOM']
    built=subprocess.run(cmd+flags,capture_output=True,text=True);self.assertEqual(built.returncode,0,built.stderr)
    run=subprocess.run([str(exe),'--game',str(ROOT/'game'),'--save',str(base/variant/'save'),'--reports',str(base/variant/'reports'),'--settings',str(base/'settings.ini'),'--headless','1','--out',str(base/variant/'frames'),'--mute','--no-save-preferences'],capture_output=True,text=True,env=dict(os.environ,LOCALAPPDATA=str(base/'local'),SDL_VIDEODRIVER='dummy',SDL_AUDIODRIVER='dummy'),timeout=60)
    self.assertEqual(run.returncode,0,run.stdout+run.stderr);self.assertIn('OFFLINE_STORE_ARM_OK',run.stdout)
