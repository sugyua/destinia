"""FC snapshots and predictions against original ARM stats."""
import os,pathlib,shlex,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
@unittest.skipUnless(os.environ.get('DESTINIA_NATIVE_TESTS')=='1','Requer dados ARM, SDL2 e Unicorn')
class CombatPowerTests(unittest.TestCase):
 def test_stats_buffs_prediction_and_snapshot_restore(self):
  flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','sdl2','unicorn','zlib'],text=True))
  with tempfile.TemporaryDirectory(prefix='destinia touch ') as directory:
   base=pathlib.Path(directory)
   for variant in ('host','host_zoom'):
    exe=base/variant
    command=['g++','-std=c++17','-O0',str(ROOT/'tests/combat_power_native.cpp'),'-o',str(exe)]
    if variant=='host_zoom':command+=['-DDESTINIA_TEST_ZOOM']
    built=subprocess.run(command+flags,capture_output=True,text=True)
    self.assertEqual(built.returncode,0,built.stderr)
    case=base/(variant+'-drops');case.mkdir()
    env=dict(os.environ,SDL_VIDEODRIVER='dummy',SDL_AUDIODRIVER='dummy')
    result=subprocess.run([str(exe),'--game',str(ROOT/'game'),'--save',str(case/'save'),'--settings',str(case/'config.ini'),'--reports',str(case/'reports'),'--headless','1','--out',str(case/'frames'),'--mute','--no-save-preferences'],env=env,capture_output=True,text=True,timeout=60)
    self.assertEqual(result.returncode,0,result.stdout+result.stderr)
    self.assertIn('FC_COMPARATOR_ARM_OK',result.stdout)
