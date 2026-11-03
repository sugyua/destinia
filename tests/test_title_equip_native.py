import os,pathlib,shlex,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
@unittest.skipUnless(os.environ.get('DESTINIA_NATIVE_TESTS')=='1','Requires original game and native libraries')
class TitleEquipNative(unittest.TestCase):
 def test_exclusive_equip_bonus_persistence_and_external_conflict(self):
  flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','sdl2','unicorn','zlib'],text=True))
  with tempfile.TemporaryDirectory(prefix='destinia-title-equip-',ignore_cleanup_errors=True) as d:
   base=pathlib.Path(d)
   for variant in ('host','zoom'):
    exe=base/(variant+'.exe');cmd=['g++','-std=c++17','-O0',str(ROOT/'tests/title_equip_native.cpp'),'-o',str(exe)]+(['-DDESTINIA_TEST_ZOOM'] if variant=='zoom' else [])+flags
    b=subprocess.run(cmd,text=True,capture_output=True);self.assertEqual(b.returncode,0,b.stderr)
    case=base/variant;env=dict(os.environ,LOCALAPPDATA=str(case/'appdata'),SDL_VIDEODRIVER='dummy',SDL_AUDIODRIVER='dummy')
    args=[str(exe),'--game',os.environ.get('DESTINIA_GAME_ROOT',str(ROOT/'game')),'--save',str(case/'save'),'--settings',str(case/'settings.ini'),'--reports',str(case/'reports'),'--headless','1','--lang','en','--mute','--no-save-preferences','--out',str(case/'frames')]
    r=subprocess.run(args,cwd=base,env=env,text=True,capture_output=True,timeout=120);self.assertEqual(r.returncode,0,r.stdout+r.stderr);self.assertIn('TITLE_EQUIP_ARM_OK',r.stdout)
if __name__=='__main__':unittest.main()
