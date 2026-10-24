"""Read-only inventory projection; no campaign or UI claim."""
import os,pathlib,subprocess,tempfile,unittest,shlex
ROOT=pathlib.Path(__file__).resolve().parents[1]
class InventoryViewTests(unittest.TestCase):
 def test_projection_preserves_items_and_selection(self):
  with tempfile.TemporaryDirectory(prefix='destinia inventory ') as folder:
   exe=pathlib.Path(folder)/('inventory.exe' if os.name=='nt' else 'inventory')
   built=subprocess.run(['g++','-std=c++17',str(ROOT/'tests/inventory_view_cases.cpp'),'-o',str(exe)],capture_output=True,text=True)
   self.assertEqual(built.returncode,0,built.stderr)
   run=subprocess.run([str(exe)],capture_output=True,text=True,timeout=20)
   self.assertEqual(run.returncode,0,run.stdout+run.stderr)

 @unittest.skipUnless(os.environ.get('DESTINIA_NATIVE_TESTS')=='1','Requer ARM/SDL/Unicorn')
 def test_original_equipment_target_in_both_hosts(self):
  flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','sdl2','unicorn','zlib'],text=True))
  with tempfile.TemporaryDirectory(prefix='destinia inventory ARM ') as folder:
   for host in ('host','host_zoom'):
    case=pathlib.Path(folder)/host;case.mkdir();exe=case/'probe'
    cmd=['g++','-std=c++17','-O0',str(ROOT/'tests/inventory_view_native.cpp'),'-o',str(exe)]
    if host=='host_zoom':cmd+=['-DDESTINIA_TEST_ZOOM']
    build=subprocess.run(cmd+flags,capture_output=True,text=True);self.assertEqual(build.returncode,0,build.stderr)
    env=dict(os.environ,SDL_VIDEODRIVER='dummy',SDL_AUDIODRIVER='dummy')
    run=subprocess.run([str(exe),'--game',str(ROOT/'game'),'--settings',str(case/'config.ini'),'--save',str(case/'save'),'--reports',str(case/'reports'),'--headless','1','--out',str(case/'frames'),'--mute','--no-save-preferences'],env=env,capture_output=True,text=True,timeout=60)
    self.assertEqual(run.returncode,0,run.stdout+run.stderr);self.assertIn('INVENTORY_SELECTION_ARM_OK',run.stdout)
