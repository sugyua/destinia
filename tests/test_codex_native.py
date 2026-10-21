import os,pathlib,subprocess,tempfile,shlex,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
@unittest.skipUnless(os.environ.get('DESTINIA_NATIVE_TESTS')=='1','Requer SDL2, Unicorn e dados originais')
class CodexNativeTests(unittest.TestCase):
 def test_original_arm_callbacks_discover_count_dedup_and_ignore_offscreen(self):
  self.assertTrue((ROOT/'common/codex_host.h').exists(),'Ponte nativa do Códex ausente')
  flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','sdl2','unicorn','zlib'],text=True))
  with tempfile.TemporaryDirectory() as directory:
   base=pathlib.Path(directory)
   for variant in ['host','host_zoom']:
    exe=base/(variant+'.exe');command=['g++','-std=c++17','-O0',str(ROOT/'tests/codex_host_cases.cpp'),'-o',str(exe)]
    if variant=='host_zoom':command+=['-DDESTINIA_TEST_ZOOM']
    built=subprocess.run(command+flags,capture_output=True,text=True);self.assertEqual(built.returncode,0,built.stderr)
    env=dict(os.environ,LOCALAPPDATA=str(base/'appdata'),SDL_VIDEODRIVER='dummy',SDL_AUDIODRIVER='dummy')
    run=subprocess.run([str(exe),'--game',str(ROOT/'game'),'--save',str(base/variant/'save'),'--reports',str(base/variant/'reports'),'--headless','10','--out',str(base/variant/'frames'),'--mute','--no-save-preferences'],env=env,capture_output=True,text=True,timeout=90)
    self.assertEqual(run.returncode,0,run.stdout+run.stderr);self.assertIn('CODEX_ARM_OK',run.stdout)
