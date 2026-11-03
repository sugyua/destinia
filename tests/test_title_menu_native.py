import os,pathlib,shlex,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
class TitleMenuModels(unittest.TestCase):
 def test_model_and_original_styled_view(self):
  with tempfile.TemporaryDirectory(prefix='destinia-title-unit-',ignore_cleanup_errors=True) as d:
   for name in ('title_menu_cases','title_menu_view_cases'):
    exe=pathlib.Path(d)/(name+'.exe');b=subprocess.run(['g++','-std=c++17',str(ROOT/'tests'/(name+'.cpp')),'-o',str(exe)],text=True,capture_output=True);self.assertEqual(b.returncode,0,b.stderr)
    r=subprocess.run([str(exe)],text=True,capture_output=True,timeout=30);self.assertEqual(r.returncode,0,r.stdout+r.stderr)
@unittest.skipUnless(os.environ.get('DESTINIA_NATIVE_TESTS')=='1','Requires original game and native libraries')
class TitleMenuNative(unittest.TestCase):
 def test_native_navigation_and_touch_in_both_hosts(self):
  flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','sdl2','unicorn','zlib'],text=True))
  with tempfile.TemporaryDirectory(prefix='destinia-title-arm-',ignore_cleanup_errors=True) as d:
   base=pathlib.Path(d)
   for variant in ('host','zoom'):
    exe=base/(variant+'.exe');cmd=['g++','-std=c++17','-O0',str(ROOT/'tests/title_menu_native.cpp'),'-o',str(exe)]+(['-DDESTINIA_TEST_ZOOM'] if variant=='zoom' else [])+flags
    b=subprocess.run(cmd,text=True,capture_output=True);self.assertEqual(b.returncode,0,b.stderr)
    case=base/variant;env=dict(os.environ,LOCALAPPDATA=str(case/'appdata'),SDL_VIDEODRIVER='dummy',SDL_AUDIODRIVER='dummy')
    args=[str(exe),'--game',os.environ.get('DESTINIA_GAME_ROOT',str(ROOT/'game')),'--save',str(case/'save'),'--settings',str(case/'settings.ini'),'--reports',str(case/'reports'),'--headless','1','--lang','en','--mute','--no-save-preferences','--out',str(case/'frames')]
    r=subprocess.run(args,cwd=base,env=env,text=True,capture_output=True,timeout=120);self.assertEqual(r.returncode,0,r.stdout+r.stderr);self.assertIn('TITLE_MENU_NAVIGATION_ARM_OK',r.stdout)
if __name__=='__main__':unittest.main()
