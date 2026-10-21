import os
from pathlib import Path
import shlex
import subprocess
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[1]

@unittest.skipUnless(os.environ.get('DESTINIA_NATIVE_TESTS')=='1','Requer SDL2_mixer, SDL2, Unicorn e game original')
class PreferencesHostTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.build=tempfile.TemporaryDirectory(prefix='destinia preferences host ');cls.addClassCleanup(cls.build.cleanup)
        flags=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','sdl2','SDL2_mixer','unicorn','zlib'],text=True))
        cls.binaries={}
        for variant in ['host','host_zoom']:
            exe=Path(cls.build.name)/(variant+('.exe' if os.name=='nt' else ''))
            command=['g++','-std=c++17','-DHAS_SDL_MIXER',str(ROOT/'tests/preferences_host_cases.cpp'),'-o',str(exe)]
            if variant=='host_zoom':command+=['-DDESTINIA_TEST_ZOOM']
            subprocess.run(command+flags,capture_output=True,text=True,check=True)
            cls.binaries[variant]=exe

    def run_scenario(self,variant,scenario):
        temp=tempfile.TemporaryDirectory(prefix='destinia profile session ');self.addCleanup(temp.cleanup);base=Path(temp.name)
        config=base/'config.ini'
        text='video.host.tela_cheia=0\nvideo.host_zoom.tela_cheia=0\naudio.musica=35\naudio.efeitos=70\naudio.silenciar_sem_foco=1\npausa.sem_foco=1\npausa.sem_controle=1\n'
        config.write_text(text)
        command=[str(self.binaries[variant]),scenario,'--game',str(ROOT/'game'),'--settings',str(config),'--no-save-preferences','--save',str(base/'save'),'--reports',str(base/'relatorios')]
        if scenario=='audio':command+=['--headless','10','--out',str(base/'frames')]
        env=dict(os.environ,SDL_VIDEODRIVER='dummy',SDL_AUDIODRIVER='dummy',LOCALAPPDATA=str(base/'dados locais'))
        run=subprocess.run(command,cwd=base,env=env,capture_output=True,text=True,timeout=60)
        self.assertEqual(run.returncode,0,run.stderr+run.stdout)
        self.assertEqual(config.read_text(),text,'Diagnostico alterou preferencias pessoais')
        logs='\n'.join(p.read_text() for p in (base/'relatorios').glob('jogo_*/log*.txt'))
        return run,logs

    def test_focus_pause_mutes_audio_and_releases_aliases_once(self):
        for variant in self.binaries:
            with self.subTest(variant=variant):
                run,logs=self.run_scenario(variant,'focus')
                self.assertIn('runs=3 muted=1 restored=1',run.stdout)
                self.assertEqual(logs.count('ENTRADA tecla=3 pressionada'),1)
                self.assertEqual(logs.count('ENTRADA tecla=3 solta'),1)

    def test_selected_controller_disconnect_releases_input_and_can_resume(self):
        for variant in self.binaries:
            with self.subTest(variant=variant):
                run,logs=self.run_scenario(variant,'disconnect')
                self.assertIn('runs=4',run.stdout)
                self.assertEqual(logs.count('ENTRADA tecla=7 pressionada'),1)
                self.assertEqual(logs.count('ENTRADA tecla=7 solta'),1)

    def test_window_preferences_survive_reopen_and_validation_rejects_bad_values(self):
        with tempfile.TemporaryDirectory(prefix='destinia window preferences ') as directory:
            base=Path(directory)
            config=base/'config.ini'
            config.write_text('audio.musica=35\nchave_futura=preservar\n')
            env=dict(os.environ,SDL_VIDEODRIVER='dummy',SDL_AUDIODRIVER='dummy',LOCALAPPDATA=str(base/'local'))
            for variant,exe in self.binaries.items():
                for extra in (['--windowed','--scale','2'],[]):
                    command=[str(exe),'persist','--game',str(ROOT/'game'),'--settings',str(config),'--save',str(base/variant/'save'),'--reports',str(base/'reports')]+extra
                    run=subprocess.run(command,cwd=base,env=env,capture_output=True,text=True,timeout=60)
                    self.assertEqual(run.returncode,0,run.stderr)
                    text=config.read_text()
                    self.assertIn('video.'+variant+'.largura=800',text)
                    self.assertIn('video.'+variant+'.altura=480',text)
                    self.assertIn('video.'+variant+'.tela_cheia=0',text)
                    self.assertIn('audio.musica=35',text)
                    self.assertIn('chave_futura=preservar',text)
                invalid=base/'invalid.ini';invalid.write_text('audio.musica=999\n')
                rejected=subprocess.run([str(exe),'persist','--settings',str(invalid),'--save',str(base/'must-not-exist')],cwd=base,env=env,capture_output=True,text=True,timeout=10)
                self.assertNotEqual(rejected.returncode,0)
                self.assertFalse((base/'must-not-exist').exists())
                checked=subprocess.run([str(exe),'persist','--check-settings',str(invalid)],cwd=base,env=env,capture_output=True,text=True,timeout=10)
                self.assertNotEqual(checked.returncode,0)

    def test_real_mixer_honors_user_and_native_volumes_and_decodes_game_audio(self):
        for variant in self.binaries:
            with self.subTest(variant=variant):
                run,logs=self.run_scenario(variant,'audio')
                self.assertIn('CASE_OK',run.stdout)

if __name__=='__main__': unittest.main()
