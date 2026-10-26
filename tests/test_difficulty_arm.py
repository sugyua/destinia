import os, pathlib, shlex, subprocess, tempfile, unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
class DifficultyArmTests(unittest.TestCase):
 @unittest.skipUnless(os.environ.get('DESTINIA_NATIVE_TESTS')=='1','Requires original ARM and SDL2/Unicorn')
 def test_native_both_hosts(self):
  flags=[f for f in shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','sdl2','unicorn','zlib'],text=True)) if f not in ['-mwindows','-Dmain=SDL_main']]
  with tempfile.TemporaryDirectory(prefix='destinia-difficulty-arm-') as t:
   for variant in ['host','host_zoom']:
    case=pathlib.Path(t)/variant;case.mkdir();exe=case/'probe.exe'
    subprocess.run(['g++','-std=c++17','-O0',str(ROOT/'tests/difficulty_arm_native.cpp'),'-o',str(exe)]+(['-DDESTINIA_TEST_ZOOM'] if variant=='host_zoom' else [])+flags,check=True)
    for mode in ['classico','expandido']:
     run_case=case/mode;run_case.mkdir();(run_case/'config.ini').write_text(f'interface.{variant}.escala=100\njogo.modo={mode}\n',encoding='ascii')
     env=dict(os.environ,SDL_VIDEODRIVER='dummy',SDL_AUDIODRIVER='dummy')
     run=subprocess.run([str(exe),'--game',str(ROOT/'game'),'--save',str(run_case/'save'),'--settings',str(run_case/'config.ini'),'--reports',str(run_case/'reports'),'--headless','1','--out',str(run_case/'frames'),'--mute','--no-save-preferences'],env=env,capture_output=True,text=True,timeout=90)
     self.assertEqual(run.returncode,0,run.stdout+run.stderr);self.assertIn('DIFFICULTY_GAMEPLAY_ARM_OK',run.stdout)
     print(variant+'/'+mode+': '+ ' | '.join(line for line in run.stdout.splitlines() if line.startswith(('PROFILE ','DROP_PARITY','LATE_REWARD','MAP_SPAWN'))))
if __name__=='__main__':unittest.main()
