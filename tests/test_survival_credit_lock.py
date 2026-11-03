"""ARM reward credit must honor the same lock as slot deletion."""
import os,pathlib,shlex,subprocess,tempfile,unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
@unittest.skipUnless(os.environ.get('DESTINIA_NATIVE_TESTS')=='1','Requires original game and native libraries')
class SurvivalCreditLock(unittest.TestCase):
 def test_credit_is_blocked_without_spending_or_losing_reward(self):
  flags=[f for f in shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','sdl2','unicorn','zlib'],text=True)) if f not in ('-mwindows','-Dmain=SDL_main')]
  with tempfile.TemporaryDirectory(prefix='destinia-survival-lock-',ignore_cleanup_errors=True) as d:
   base=pathlib.Path(d)
   for variant in ('host','zoom'):
    exe=base/(variant+'.exe');cmd=['g++','-std=c++17','-O0',str(ROOT/'tests/survival_arm_native.cpp'),'-o',str(exe)]
    if variant=='zoom':cmd+=['-DDESTINIA_TEST_ZOOM']
    subprocess.run(cmd+flags,check=True)
    for mode in ('credit','fungus_arena','fungus_codex_lock','fungus_codex_stop','fungus_invalid_codex','title_retry','title_stop','full'):
     case=base/(variant+'-'+mode);env=dict(os.environ,LOCALAPPDATA=str(case/'appdata'),SDL_VIDEODRIVER='dummy',SDL_AUDIODRIVER='dummy')
     if mode=='credit':env['DESTINIA_CREDIT_ONLY']='1'
     else:env.pop('DESTINIA_CREDIT_ONLY',None)
     if mode in ('title_retry','title_stop'):env['DESTINIA_TITLE_LOCK_PROBE']='1'
     else:env.pop('DESTINIA_TITLE_LOCK_PROBE',None)
     if mode=='fungus_invalid_codex':
      env['DESTINIA_FUNGUS_INVALID_CODEX_TEST']='1';(case/'save').mkdir(parents=True);(case/'save'/'codex-v1.txt').write_text('invalid-personal-codex')
     else:env.pop('DESTINIA_FUNGUS_INVALID_CODEX_TEST',None)
     if mode=='fungus_arena':env['DESTINIA_FUNGUS_ARENA_TEST']='1'
     else:env.pop('DESTINIA_FUNGUS_ARENA_TEST',None)
     if mode in ('fungus_codex_lock','fungus_codex_stop'):env['DESTINIA_FUNGUS_CODEX_LOCK_TEST']='1'
     else:env.pop('DESTINIA_FUNGUS_CODEX_LOCK_TEST',None)
     if mode=='fungus_codex_stop':env['DESTINIA_FUNGUS_CODEX_STOP_TEST']='1'
     else:env.pop('DESTINIA_FUNGUS_CODEX_STOP_TEST',None)
     if mode=='title_stop':env['DESTINIA_TITLE_STOP_PROBE']='1'
     else:env.pop('DESTINIA_TITLE_STOP_PROBE',None)
     args=[str(exe),'--game',os.environ.get('DESTINIA_GAME_ROOT',str(ROOT/'game')),'--save',str(case/'save'),'--settings',str(case/'settings.ini'),'--reports',str(case/'reports'),'--headless','1','--lang','en','--mute','--no-save-preferences','--out',str(case/'frames')]
     result=subprocess.run(args,env=env,capture_output=True,text=True,timeout=180)
     self.assertEqual(result.returncode,0,result.stdout+result.stderr)
     self.assertIn({'credit':'SURVIVAL_CREDIT_LOCK_OK','fungus_arena':'FUNGUS_ARENA_COUNT_OK','fungus_codex_lock':'FUNGUS_CODEX_LOCK_OK','fungus_codex_stop':'FUNGUS_CODEX_STOP_OK','fungus_invalid_codex':'FUNGUS_INVALID_CODEX_OK','title_retry':'TITLE_LOCK_RETRY_OK','title_stop':'TITLE_STOP_RETRY_OK','full':'SURVIVAL_ARM_OK'}[mode],result.stdout)
