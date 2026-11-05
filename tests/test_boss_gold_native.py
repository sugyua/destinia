import os,pathlib,shlex,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
@unittest.skipUnless(os.environ.get('DESTINIA_NATIVE_TESTS')=='1','Requer SDL2, Unicorn e dados originais')
class BossGoldNative(unittest.TestCase):
 def test_gold_value_is_not_call_success_flag(self):
  flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','sdl2','unicorn','zlib'],text=True))
  with tempfile.TemporaryDirectory(prefix='destinia-boss-gold-',ignore_cleanup_errors=True) as folder:
   base=pathlib.Path(folder)
   for zoom in (False,True):
    exe=base/('zoom.exe' if zoom else 'normal.exe');cmd=['g++','-std=c++17','-O0']+(['-DDESTINIA_TEST_ZOOM'] if zoom else [])+[str(ROOT/'tests/boss_gold_native.cpp'),'-o',str(exe)]+flags
    b=subprocess.run(cmd,capture_output=True,text=True);self.assertEqual(b.returncode,0,b.stderr)
    env=dict(os.environ,SDL_VIDEODRIVER='dummy',SDL_AUDIODRIVER='dummy',LOCALAPPDATA=str(base/'appdata'))
    r=subprocess.run([str(exe),'--game',str(ROOT/'game'),'--save',str(base/str(zoom)/'save'),'--settings',str(base/'settings.ini'),'--reports',str(base/str(zoom)/'reports'),'--headless','1','--out',str(base/str(zoom)/'frames'),'--no-save-preferences','--mute'],cwd=base,env=env,capture_output=True,text=True,timeout=120)
    self.assertEqual(r.returncode,0,r.stdout+r.stderr);self.assertIn('BOSS_GOLD_READ_OK',r.stdout)
