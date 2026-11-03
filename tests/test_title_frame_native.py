"""Use jniRun with a synthetic map to check the added title page composition."""
import os,pathlib,shlex,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
@unittest.skipUnless(os.environ.get('DESTINIA_NATIVE_TESTS')=='1','Requires original game and native libraries')
class TitleFrameTests(unittest.TestCase):
 def test_native_page_is_redirected_and_framebuffer_restored(self):
  flags=[f for f in shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','sdl2','unicorn','zlib'],text=True)) if f not in ('-mwindows','-Dmain=SDL_main')]
  with tempfile.TemporaryDirectory(prefix='destinia-title-frame-',ignore_cleanup_errors=True) as d:
   base=pathlib.Path(d)
   for variant in ('host','host_zoom'):
    exe=base/(variant+'.exe');cmd=['g++','-std=c++17','-O0',str(ROOT/'tests/title_loop_probe_native.cpp'),'-o',str(exe)]
    if variant=='host_zoom':cmd+=['-DDESTINIA_TEST_ZOOM']
    subprocess.run(cmd+flags,check=True)
    for scale in (75,100,125):
     case=base/(variant+str(scale));case.mkdir();(case/'settings.ini').write_text(f'interface.{variant}.escala={scale}\n')
     env=dict(os.environ,LOCALAPPDATA=str(case/'appdata'),SDL_VIDEODRIVER='dummy',SDL_AUDIODRIVER='dummy',DESTINIA_TITLE_TEST_ABORT='1')
     args=[str(exe),'--game',os.environ.get('DESTINIA_GAME_ROOT',str(ROOT/'game')),'--save',str(case/'save'),'--settings',str(case/'settings.ini'),'--reports',str(case/'reports'),'--headless','1','--lang','en','--mute','--no-save-preferences','--out',str(case/'frames')]
     result=subprocess.run(args,env=env,capture_output=True,text=True,timeout=120)
     self.assertEqual(result.returncode,0,result.stdout+result.stderr)
     self.assertIn('TITLE_LOOP_PROBE_OK',result.stdout)
