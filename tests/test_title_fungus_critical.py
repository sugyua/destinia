import os,pathlib,shlex,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
@unittest.skipUnless(os.environ.get('DESTINIA_NATIVE_TESTS')=='1','Requires original game and native libraries')
class FungusCritical(unittest.TestCase):
 def test_original_critical_branch_with_fixed_random(self):
  flags=[f for f in shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','sdl2','unicorn','zlib'],text=True)) if f not in ('-mwindows','-Dmain=SDL_main')]
  with tempfile.TemporaryDirectory(prefix='destinia-critical-',ignore_cleanup_errors=True) as d:
   base=pathlib.Path(d)
   for variant in ('host','zoom'):
    exe=base/(variant+'.exe');cmd=['g++','-std=c++17','-O0',str(ROOT/'tests/title_fungus_critical_native.cpp'),'-o',str(exe)]
    if variant=='zoom':cmd+=['-DDESTINIA_TEST_ZOOM']
    subprocess.run(cmd+flags,check=True);q=base/variant
    env=dict(os.environ,LOCALAPPDATA=str(q/'appdata'),SDL_VIDEODRIVER='dummy',SDL_AUDIODRIVER='dummy')
    args=[str(exe),'--game',os.environ.get('DESTINIA_GAME_ROOT',str(ROOT/'game')),'--save',str(q/'save'),'--settings',str(q/'settings.ini'),'--reports',str(q/'reports'),'--headless','1','--lang','en','--mute','--no-save-preferences','--out',str(q/'frames')]
    result=subprocess.run(args,env=env,capture_output=True,text=True,timeout=120)
    self.assertEqual(result.returncode,0,(variant,result.stdout,result.stderr))
    self.assertIn('FUNGUS_CRITICAL_ARM_OK',result.stdout)
