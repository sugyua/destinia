"""Disk-backed transaction tests including process interruption, never personal saves."""
import pathlib,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
class SafeSaveTests(unittest.TestCase):
 def test_batch_commit_abort_and_process_interruption(self):
  self.assertTrue((ROOT/'common/safe_save.h').exists(),'Gravacao transacional ainda ausente')
  with tempfile.TemporaryDirectory(prefix='destinia safe save ') as directory:
   base=pathlib.Path(directory);exe=base/'safe-save.exe'
   built=subprocess.run(['g++','-std=c++17',str(ROOT/'tests/safe_save_cases.cpp'),'-o',str(exe)],capture_output=True,text=True)
   self.assertEqual(built.returncode,0,built.stderr)
   save=base/'partida';run=subprocess.run([str(exe),str(save)],capture_output=True,text=True)
   self.assertEqual(run.returncode,0,run.stdout+run.stderr)
   before={p.name:p.read_bytes() for p in save.iterdir()}
   crash=subprocess.run([str(exe),str(save),'crash'],capture_output=True,text=True)
   self.assertEqual(crash.returncode,73,crash.stdout+crash.stderr)
   self.assertTrue((base/'partida.transaction'/'manifest').exists())
   # A separate process must recover the old batch before testing again.
   recover_source=base/'recover.cpp';recover_source.write_text('#include "'+str(ROOT/'common/safe_save.h')+'"\nint main(int n,char**v){try{destinia::SaveTransaction tx;tx.configure(v[1]);tx.recover();return 0;}catch(...){return 2;}}\n')
   recovery=base/'recover.exe';built=subprocess.run(['g++','-std=c++17',str(recover_source),'-o',str(recovery)],capture_output=True,text=True);self.assertEqual(built.returncode,0,built.stderr)
   recovered=subprocess.run([str(recovery),str(save)],capture_output=True,text=True);self.assertEqual(recovered.returncode,0,recovered.stderr)
   self.assertEqual({p.name:p.read_bytes() for p in save.iterdir()},before)
   crash=subprocess.run([str(exe),str(save),'delete-crash'],capture_output=True,text=True)
   self.assertEqual(crash.returncode,73,crash.stdout+crash.stderr)
   self.assertFalse((save/'a.dat').exists())
   recovered=subprocess.run([str(recovery),str(save)],capture_output=True,text=True);self.assertEqual(recovered.returncode,0,recovered.stderr)
   self.assertEqual({p.name:p.read_bytes() for p in save.iterdir()},before)
   journal=base/'partida.transaction';journal.mkdir();(journal/'owner').write_text('arquivo do usuario')
   refused=subprocess.run([str(recovery),str(save)],capture_output=True,text=True)
   self.assertNotEqual(refused.returncode,0);self.assertEqual((journal/'owner').read_text(),'arquivo do usuario')

   (journal/'owner').write_text('DESTINIA_SAVE_TRANSACTION_1\n');(journal/'new').mkdir();(journal/'old').mkdir();(journal/'manifest').write_text('truncado')
   refused=subprocess.run([str(recovery),str(save)],capture_output=True,text=True)
   self.assertNotEqual(refused.returncode,0);self.assertTrue((journal/'manifest').exists(),'Journal corrompido deve ser preservado')
   self.assertEqual({p.name:p.read_bytes() for p in save.iterdir()},before)

import os,shlex
@unittest.skipUnless(os.environ.get('DESTINIA_NATIVE_TESTS')=='1','Requer SDL2/Unicorn')
class SafeSaveHostTests(unittest.TestCase):
 def test_stdio_closed_save_is_rolled_back_when_arm_call_fails(self):
  flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','sdl2','unicorn','zlib'],text=True))
  with tempfile.TemporaryDirectory(prefix='destinia safe host ') as directory:
   base=pathlib.Path(directory)
   for variant in ('host','host_zoom'):
    exe=base/(variant+'.exe');command=['g++','-std=c++17',str(ROOT/'tests/safe_save_host_cases.cpp'),'-o',str(exe)]
    if variant=='host_zoom':command+=['-DDESTINIA_TEST_ZOOM']
    built=subprocess.run(command+flags,capture_output=True,text=True);self.assertEqual(built.returncode,0,built.stderr)
    run=subprocess.run([str(exe),str(base/variant)],capture_output=True,text=True,timeout=120)
    self.assertEqual(run.returncode,0,run.stdout+run.stderr);self.assertIn('SAFE_SAVE_HOST_OK',run.stdout)
