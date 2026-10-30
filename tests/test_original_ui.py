"""Compare original ARM frame raster and verify isolated cached UI captures."""
import os,pathlib,shlex,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
@unittest.skipUnless(os.environ.get('DESTINIA_NATIVE_TESTS')=='1','Requires original ARM data, SDL2 and Unicorn')
class OriginalUiTests(unittest.TestCase):
 def test_original_resources_and_state_restoration(self):
  flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','sdl2','unicorn','zlib'],text=True))
  with tempfile.TemporaryDirectory(prefix='destinia-original-ui-') as directory:
   base=pathlib.Path(directory)
   for variant in ('host','zoom'):
    exe=base/(variant+'.exe');command=['g++','-std=c++17','-O0',str(ROOT/'tests/original_ui_native.cpp'),'-o',str(exe)]
    if variant=='zoom':command+=['-DDESTINIA_TEST_ZOOM']
    result=subprocess.run(command+flags,capture_output=True,text=True);self.assertEqual(result.returncode,0,result.stderr)
    env=dict(os.environ,LOCALAPPDATA=str(base/'appdata'),SDL_VIDEODRIVER='dummy',SDL_AUDIODRIVER='dummy')
    result=subprocess.run([str(exe),'--game',os.environ.get('DESTINIA_GAME_ROOT',str(ROOT/'game')),'--save',str(base/variant/'save'),'--settings',str(base/variant/'settings.ini'),'--reports',str(base/variant/'reports'),'--headless','1','--out',str(base/variant/'frames'),'--mute','--no-save-preferences'],env=env,capture_output=True,text=True,timeout=90)
    self.assertEqual(result.returncode,0,result.stdout+result.stderr);self.assertIn('ORIGINAL_UI_ARM_OK',result.stdout)

@unittest.skipUnless(os.environ.get('DESTINIA_NATIVE_TESTS')=='1','Requires C++ compiler')
class OriginalPanelBindings(unittest.TestCase):
 def test_all_added_panel_bindings(self):
  with tempfile.TemporaryDirectory(prefix='destinia-panel-bindings-',ignore_cleanup_errors=True) as directory:
   for fixture in ('original_quest_cases','original_secondary_cases','original_panel_bridge_cases','boss_mage_cases'):
    exe=pathlib.Path(directory)/(fixture+'.exe')
    build=subprocess.run(['g++','-std=c++17','-O0',str(ROOT/'tests'/str(fixture+'.cpp')),'-o',str(exe),'-lz'],capture_output=True,text=True)
    self.assertEqual(build.returncode,0,build.stderr)
    result=subprocess.run([str(exe)],capture_output=True,text=True,timeout=20)
    self.assertEqual(result.returncode,0,result.stdout+result.stderr)
