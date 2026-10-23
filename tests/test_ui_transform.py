import os,pathlib,shlex,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
class UiTransformTests(unittest.TestCase):
 def test_pixel_inverse_and_codex_fit(self):
  with tempfile.TemporaryDirectory(prefix='destinia ui matrix ') as d:
   exe=pathlib.Path(d)/'matrix.exe';b=subprocess.run(['g++','-std=c++17',str(ROOT/'tests/ui_transform_cases.cpp'),'-lz','-o',str(exe)],capture_output=True,text=True);self.assertEqual(b.returncode,0,b.stderr);r=subprocess.run([str(exe)],capture_output=True,text=True);self.assertEqual(r.returncode,0,r.stderr)
 @unittest.skipUnless(os.environ.get('DESTINIA_NATIVE_TESTS')=='1','Requer ARM/SDL/Unicorn')
 def test_real_arm_raster_clipping_alpha_controls_and_game_frame(self):
  flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','sdl2','unicorn','zlib'],text=True))
  with tempfile.TemporaryDirectory(prefix='destinia ui ARM ') as d:
   root=pathlib.Path(d)
   for host in ('host','host_zoom'):
    exe=root/(host+'.exe');cmd=['g++','-std=c++17','-O1',str(ROOT/'tests/ui_renderer_native.cpp'),'-o',str(exe)]+(['-DDESTINIA_TEST_ZOOM'] if host=='host_zoom' else [])+flags
    b=subprocess.run(cmd,capture_output=True,text=True);self.assertEqual(b.returncode,0,b.stderr)
    for percent in (75,100,125):
     case=root/(host+str(percent));r=subprocess.run([str(exe),str(percent),'--game',str(ROOT/'game'),'--save',str(case/'save'),'--settings',str(case/'settings.ini'),'--headless','1','--out',str(case/'frames'),'--reports',str(case/'reports'),'--mute'],capture_output=True,text=True,timeout=90,env=dict(os.environ,SDL_VIDEODRIVER='dummy',SDL_AUDIODRIVER='dummy',LOCALAPPDATA=str(root/'local')));self.assertEqual(r.returncode,0,r.stdout+r.stderr);self.assertIn('UI_RENDERER_FILL_CLIP_ALPHA_OK',r.stdout)
