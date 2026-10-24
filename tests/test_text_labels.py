"""Portuguese assets and physical skill hints; temporary files only."""
import os,pathlib,shlex,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
class TextLabelsTests(unittest.TestCase):
 def test_binding_labels_and_accent_pixels(self):
  # MinGW SDL pkg-config renames main for GUI apps; these are console fixtures.
  flags=[flag for flag in shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','sdl2'],text=True)) if flag!='-Dmain=SDL_main']
  with tempfile.TemporaryDirectory(prefix='destinia labels ') as folder:
   for name in ('skill_labels_cases','overlay_font_cases','portuguese_text_cases'):
    binary=pathlib.Path(folder)/name
    build=subprocess.run(['g++','-std=c++17',str(ROOT/'tests'/f'{name}.cpp'),'-o',str(binary)]+flags,capture_output=True,text=True)
    self.assertEqual(build.returncode,0,build.stderr)
    run=subprocess.run([str(binary)],capture_output=True,text=True,timeout=30)
    self.assertEqual(run.returncode,0,run.stdout+run.stderr)
 @unittest.skipUnless(os.environ.get('DESTINIA_NATIVE_TESTS')=='1','Requer ARM/SDL/Unicorn')
 def test_native_quickslot_mapping(self):
  flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','sdl2','unicorn','zlib'],text=True))
  with tempfile.TemporaryDirectory(prefix='destinia skills ARM ') as folder:
   for host in ('host','host_zoom'):
    case=pathlib.Path(folder)/host;case.mkdir();binary=case/'probe'
    command=['g++','-std=c++17','-O0',str(ROOT/'tests/skill_labels_native.cpp'),'-o',str(binary)]
    if host=='host_zoom':command+=['-DDESTINIA_TEST_ZOOM']
    build=subprocess.run(command+flags,capture_output=True,text=True)
    self.assertEqual(build.returncode,0,build.stderr)
    env=dict(os.environ,SDL_VIDEODRIVER='dummy',SDL_AUDIODRIVER='dummy')
    result=subprocess.run([str(binary),'--game',str(ROOT/'game'),'--save',str(case/'save'),'--settings',str(case/'config.ini'),'--reports',str(case/'reports'),'--headless','1','--out',str(case/'frames'),'--mute','--no-save-preferences'],env=env,capture_output=True,text=True,timeout=60)
    self.assertEqual(result.returncode,0,result.stdout+result.stderr)
    self.assertIn('QUICKSLOT_MAPPING_ARM_OK',result.stdout+result.stderr)
 def test_both_hosts_accept_portuguese(self):
  flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','sdl2','unicorn','zlib'],text=True))
  with tempfile.TemporaryDirectory(prefix='destinia text ') as folder:
   for host in ('host','host_zoom'):
    case=pathlib.Path(folder)/host;case.mkdir();binary=case/'host'
    build=subprocess.run(['g++','-std=c++17','-O0',str(ROOT/host/'main.cpp'),'-o',str(binary)]+flags,capture_output=True,text=True)
    self.assertEqual(build.returncode,0,build.stderr)
    env=dict(os.environ,SDL_VIDEODRIVER='dummy',SDL_AUDIODRIVER='dummy')
    result=subprocess.run([str(binary),'--game',str(ROOT/'game'),'--save',str(case/'save'),'--settings',str(case/'config.ini'),'--reports',str(case/'reports'),'--lang','pt','--headless','1','--out',str(case/'frames'),'--mute','--no-save-preferences'],env=env,capture_output=True,text=True,timeout=60)
    self.assertEqual(result.returncode,0,result.stdout+result.stderr)
    self.assertIn('textos PT aplicados',result.stdout+result.stderr,result.stdout+result.stderr)
if __name__=='__main__':unittest.main()
