import os,pathlib,shlex,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
@unittest.skipUnless(os.environ.get('DESTINIA_NATIVE_TESTS')=='1','Requires original game and native libraries')
class OriginalTitleRequirements(unittest.TestCase):
 def test_original_requirements_remain_visible_for_locked_titles(self):
  flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','sdl2','unicorn','zlib'],text=True))
  with tempfile.TemporaryDirectory(prefix='destinia-titles-',ignore_cleanup_errors=True) as directory:
   base=pathlib.Path(directory)
   for variant in ('host','zoom'):
    exe=base/(variant+'.exe');cmd=['g++','-std=c++17','-O0',str(ROOT/'tests/nick_requirements_native.cpp'),'-o',str(exe)]+(['-DDESTINIA_TEST_ZOOM'] if variant=='zoom' else [])+flags
    b=subprocess.run(cmd,capture_output=True,text=True);self.assertEqual(b.returncode,0,b.stderr)
    for lang in ('en','pt'):
     case=base/(variant+'-'+lang);env=dict(os.environ,LOCALAPPDATA=str(case/'appdata'),SDL_VIDEODRIVER='dummy',SDL_AUDIODRIVER='dummy')
     args=[str(exe),'--game',os.environ.get('DESTINIA_GAME_ROOT',str(ROOT/'game')),'--save',str(case/'save'),'--settings',str(case/'settings.ini'),'--reports',str(case/'reports'),'--headless','1','--lang',lang,'--mute','--no-save-preferences','--out',str(case/'frames')]
     r=subprocess.run(args,cwd=base,env=env,capture_output=True,text=True,timeout=120);self.assertEqual(r.returncode,0,r.stdout+r.stderr);self.assertIn('ORIGINAL_TITLE_REQUIREMENTS_OK',r.stdout)
if __name__=='__main__':unittest.main()
