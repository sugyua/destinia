import os,pathlib,shlex,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
@unittest.skipUnless(os.environ.get('DESTINIA_NATIVE_TESTS')=='1','Requer SDL2/Unicorn e dados originais')
class ManaGuardArm(unittest.TestCase):
 def test_repair_and_native_save_reload(self):
  flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','sdl2','unicorn','zlib'],text=True))
  with tempfile.TemporaryDirectory(prefix='destinia-mana-arm-',ignore_cleanup_errors=True) as d:
   base=pathlib.Path(d)
   for zoom in (False,True):
    exe=base/('zoom.exe' if zoom else 'normal.exe')
    cmd=['g++','-std=c++17']+(['-DDESTINIA_TEST_ZOOM'] if zoom else [])+[str(ROOT/'tests/mana_guard_arm_native.cpp'),'-o',str(exe)]+flags
    built=subprocess.run(cmd,text=True,capture_output=True);self.assertEqual(built.returncode,0,built.stderr)
    env=dict(os.environ,SDL_VIDEODRIVER='dummy',SDL_AUDIODRIVER='dummy',LOCALAPPDATA=str(base/'appdata'))
    run=subprocess.run([str(exe),'--game',str(ROOT/'game'),'--save',str(base/str(zoom)/'save'),'--settings',str(base/'settings.ini'),'--reports',str(base/str(zoom)/'reports'),'--headless','1','--out',str(base/str(zoom)/'frames'),'--no-save-preferences','--mute'],cwd=base,env=env,text=True,capture_output=True,timeout=120)
    self.assertEqual(run.returncode,0,run.stdout+run.stderr);self.assertIn('MANA_ARM_OK',run.stdout)
