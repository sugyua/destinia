"""QoL pure policy; optional real ARM probes use temporary saves only."""
import os,pathlib,shlex,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
class LootQolTests(unittest.TestCase):
 def test_categories_radius_and_markers(self):
  with tempfile.TemporaryDirectory(prefix='destinia loot ') as folder:
   exe=pathlib.Path(folder)/('loot.exe' if os.name=='nt' else 'loot')
   result=subprocess.run(['g++','-std=c++17',str(ROOT/'tests/loot_qol_cases.cpp'),'-o',str(exe)],capture_output=True,text=True)
   self.assertEqual(result.returncode,0,result.stderr)
   run=subprocess.run([str(exe)],capture_output=True,text=True,timeout=20);self.assertEqual(run.returncode,0,run.stdout+run.stderr)
 @unittest.skipUnless(os.environ.get('DESTINIA_NATIVE_TESTS')=='1','Requer biblioteca ARM original, SDL2 e Unicorn')
 def test_real_collection_and_visual_filter_both_hosts(self):
  flags=[f for f in shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','sdl2','unicorn','zlib'],text=True)) if f!='-mwindows']
  with tempfile.TemporaryDirectory(prefix='destinia loot ARM ') as folder:
   for variant in ('host','host_zoom'):
    case=pathlib.Path(folder)/variant;case.mkdir();exe=case/('probe.exe' if os.name=='nt' else 'probe')
    cmd=['g++','-std=c++17','-O0',str(ROOT/'tests/loot_qol_native.cpp'),'-o',str(exe)]+(['-DDESTINIA_TEST_ZOOM'] if variant=='host_zoom' else [])+flags
    built=subprocess.run(cmd,capture_output=True,text=True);self.assertEqual(built.returncode,0,built.stderr)
    save=case/'save';save.mkdir();sentinel=save/'save0.dat';sentinel.write_bytes(b'ORIGINAL-SAVE-SENTINEL')
    env=dict(os.environ,SDL_VIDEODRIVER='dummy',SDL_AUDIODRIVER='dummy')
    run=subprocess.run([str(exe),'--game',str(ROOT/'game'),'--save',str(save),'--settings',str(case/'config.ini'),'--reports',str(case/'reports'),'--headless','1','--out',str(case/'frames'),'--mute','--no-save-preferences'],env=env,capture_output=True,text=True,timeout=60)
    self.assertEqual(run.returncode,0,run.stdout+run.stderr);self.assertIn('LOOT_QOL_ARM_OK',run.stdout);self.assertEqual(sentinel.read_bytes(),b'ORIGINAL-SAVE-SENTINEL')
