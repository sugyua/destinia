"""Virtual controls against real ARM functions; states are controlled, not campaign."""
import os,pathlib,shlex,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
@unittest.skipUnless(os.environ.get('DESTINIA_NATIVE_TESTS')=='1','Requer dados ARM, SDL2 e Unicorn')
class TouchControlsTests(unittest.TestCase):
 def test_render_hitboxes_and_menu_state(self):
  flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','sdl2','unicorn','zlib'],text=True))
  with tempfile.TemporaryDirectory(prefix='destinia touch ') as directory:
   base=pathlib.Path(directory)
   for variant in ('host','host_zoom'):
    exe=base/variant
    command=['g++','-std=c++17','-O0',str(ROOT/'tests/touch_controls_native.cpp'),'-o',str(exe)]
    if variant=='host_zoom':command+=['-DDESTINIA_TEST_ZOOM']
    built=subprocess.run(command+flags,capture_output=True,text=True)
    self.assertEqual(built.returncode,0,built.stderr)
    for hidden in (False,True):
     with self.subTest(host=variant,hidden=hidden):
      case=base/(variant+str(hidden));case.mkdir()
      config=case/'config.ini';config.write_text(f'interface.{variant}.controles_virtuais={int(not hidden)}\n')
      env=dict(os.environ,SDL_VIDEODRIVER='dummy',SDL_AUDIODRIVER='dummy',LOCALAPPDATA=str(case/'local'))
      if hidden:env['DESTINIA_EXPECT_HIDDEN']='1'
      run=subprocess.run([str(exe),'--game',str(ROOT/'game'),'--save',str(case/'save'),'--reports',str(case/'reports'),'--settings',str(config),'--headless','1','--out',str(case/'frames'),'--mute','--no-save-preferences'],env=env,capture_output=True,text=True,timeout=60)
      self.assertEqual(run.returncode,0,run.stdout+run.stderr)
      self.assertIn('TOUCH_CONTROLS_ARM_OK',run.stdout)
      if hidden:
       altered=bytearray((ROOT/'game/lib/armeabi/libdestinia_jni.so').read_bytes());altered[-1]^=1
       library=case/'incompatible.so';library.write_bytes(altered)
       command=[str(exe),'--game',str(ROOT/'game'),'--so',str(library),'--save',str(case/'bad-save'),'--reports',str(case/'bad-reports'),'--settings',str(config),'--headless','1','--out',str(case/'bad-frames'),'--mute','--no-save-preferences']
       incompatible=subprocess.run(command,env=env,capture_output=True,text=True,timeout=60)
       self.assertEqual(incompatible.returncode,1,incompatible.stdout+incompatible.stderr)
       self.assertIn('biblioteca ARM incompativel',incompatible.stdout+incompatible.stderr)
