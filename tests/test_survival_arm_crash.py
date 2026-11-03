import os,pathlib,shlex,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
@unittest.skipUnless(os.environ.get('DESTINIA_NATIVE_TESTS')=='1','Requires original game and native libraries')
class SurvivalArmCrash(unittest.TestCase):
 def test_actual_arm_reward_transaction_interrupted(self):
  flags=[f for f in shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','sdl2','unicorn','zlib'],text=True)) if f not in ('-mwindows','-Dmain=SDL_main')]
  names=('save0.dat','refinement-v1.txt','titles-v1.txt','codex-v1.txt','survival-v1.txt')
  with tempfile.TemporaryDirectory(prefix='destinia-arm-crash-',ignore_cleanup_errors=True) as d:
   base=pathlib.Path(d);recover=base/'recover.exe'
   subprocess.run(['g++','-std=c++17','-O0',str(ROOT/'tests/survival_crash_recover.cpp'),'-o',str(recover)],check=True)
   for variant in ('host','zoom'):
    exe=base/(variant+'.exe');cmd=['g++','-std=c++17','-O0','-DDESTINIA_TEST_SAVE_OPS',str(ROOT/'tests/survival_arm_native.cpp'),'-o',str(exe)]
    if variant=='zoom':cmd+=['-DDESTINIA_TEST_ZOOM']
    subprocess.run(cmd+flags,check=True)
    for point in (1,2,3,4,5,99,100):
     q=base/(variant+str(point));save=q/'save';baseline=q/'baseline'
     env=dict(os.environ,LOCALAPPDATA=str(q/'appdata'),SDL_VIDEODRIVER='dummy',SDL_AUDIODRIVER='dummy',DESTINIA_SURVIVAL_CRASH_POINT=str(point),DESTINIA_CRASH_BASELINE=str(baseline))
     args=[str(exe),'--game',os.environ.get('DESTINIA_GAME_ROOT',str(ROOT/'game')),'--save',str(save),'--settings',str(q/'settings.ini'),'--reports',str(q/'reports'),'--headless','1','--lang','en','--mute','--no-save-preferences','--out',str(q/'frames')]
     r=subprocess.run(args,env=env,capture_output=True,text=True,timeout=180)
     self.assertEqual(r.returncode,72 if point==99 else (73 if point==100 else 71),(variant,point,r.stdout,r.stderr))
     r=subprocess.run([str(recover),str(save)],capture_output=True,text=True)
     self.assertEqual(r.returncode,0,(variant,point,r.stdout,r.stderr))
     if point!=99:
      for name in names:
       old=baseline/name;now=save/name
       self.assertEqual(old.exists(),now.exists(),(variant,point,name))
       if old.exists():self.assertEqual(old.read_bytes(),now.read_bytes(),(variant,point,name))
     else:
      self.assertIn(b'unlocked 0 ',(save/'titles-v1.txt').read_bytes())
      self.assertIn(b'slot 0 20 ',(save/'survival-v1.txt').read_bytes())
