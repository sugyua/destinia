"""Testes reais dos dois hosts. Habilitar com DESTINIA_NATIVE_TESTS=1 e SDL2/Unicorn."""
import os
from pathlib import Path
import shlex
import struct
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class RuntimeSupportTests(unittest.TestCase):
    def test_retention_backups_locks_and_failure_preservation(self):
        self.assertTrue((ROOT/'common/runtime_support.h').is_file(),'Suporte de log/backup ainda nao implementado')
        with tempfile.TemporaryDirectory(prefix='destinia support ') as directory:
            exe=Path(directory)/'support'
            compiled=subprocess.run(['g++','-std=c++17',str(ROOT/'tests/runtime_support_cases.cpp'),'-o',str(exe)],capture_output=True,text=True)
            self.assertEqual(compiled.returncode,0,compiled.stderr)
            result=subprocess.run([str(exe),str(Path(directory)/'dados')],capture_output=True,text=True,timeout=30)
            self.assertEqual(result.returncode,0,result.stderr)


@unittest.skipUnless(os.environ.get('DESTINIA_NATIVE_TESTS') == '1', 'Requer compilador, SDL2, Unicorn e dados originais')
class RuntimeHostTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.build = tempfile.TemporaryDirectory(prefix='destinia-runtime-bin-')
        cls.addClassCleanup(cls.build.cleanup)
        flags = shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','sdl2','unicorn','zlib'], text=True))
        cls.binaries = {}
        for variant in ['host','host_zoom']:
            exe = Path(cls.build.name)/variant
            command = ['g++','-std=c++17','-O0',str(ROOT/'tests/runtime_host_cases.cpp'),'-o',str(exe)]
            if variant == 'host_zoom': command.append('-DDESTINIA_TEST_ZOOM')
            subprocess.run(command+flags, check=True, capture_output=True, text=True)
            cls.binaries[variant] = exe

    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='destinia runtime ')
        self.addCleanup(self.temp.cleanup)
        self.base = Path(self.temp.name)

    def run_host(self, variant, args=(), env=None, use_save=True, headless=True):
        variables = dict(os.environ, LOCALAPPDATA=str(self.base/'local app data'),
                         SDL_VIDEODRIVER='dummy', SDL_AUDIODRIVER='dummy')
        variables.pop('DESTINIA_SAVE_DIR',None)
        if env: variables.update(env)
        command = [str(self.binaries[variant]),'--game',str(ROOT/'game'),'--mute']
        if use_save: command += ['--save',str(self.base/'save')]
        if headless: command += ['--headless','10','--out',str(self.base/'frames')]
        return subprocess.run(command+list(args),cwd=self.base,env=variables,capture_output=True,text=True,timeout=60)

    def test_zero_size_file_operations_return_zero_without_reading_memory(self):
        for variant, exe in self.binaries.items():
            for operation in ['fread','fwrite']:
                with self.subTest(variant=variant,operation=operation):
                    result=subprocess.run([str(exe),'test-'+operation+'-zero'],capture_output=True,text=True,timeout=10)
                    self.assertEqual(result.returncode,0,result.stderr)

    def test_version_can_be_checked_without_starting_or_creating_a_save(self):
        for variant,exe in self.binaries.items():
            with self.subTest(variant=variant):
                result=subprocess.run([str(exe),'--version'],cwd=self.base,capture_output=True,text=True,timeout=10)
                self.assertEqual(result.returncode,0,result.stderr)
                self.assertEqual(result.stdout.strip(),'Destinia '+(ROOT/'VERSION').read_text().strip()+' Beta')
                self.assertFalse((self.base/'save').exists())

    def test_native_android_messages_are_logged_without_verbose_mode(self):
        for variant,exe in self.binaries.items():
            with self.subTest(variant=variant):
                reports=self.base/variant
                result=subprocess.run([str(exe),'test-native-log',str(reports)],capture_output=True,text=True,timeout=10)
                self.assertEqual(result.returncode,0,result.stderr)
                text='\n'.join(p.read_text() for p in reports.glob('jogo_*/log*.txt'))
                self.assertIn('mensagem nativa observada',text)

    def test_stub_exceptions_are_stopped_at_c_callback_boundary(self):
        for variant,exe in self.binaries.items():
            with self.subTest(variant=variant):
                result=subprocess.run([str(exe),'test-stub-exception'],capture_output=True,text=True,timeout=10)
                self.assertEqual(result.returncode,0,result.stderr)
                self.assertIn('falha de arquivo simulada',result.stderr)

    def test_runtime_emulation_failure_returns_error(self):
        for variant in self.binaries:
            with self.subTest(variant=variant):
                result=self.run_host(variant,env={'DESTINIA_TEST_EMU_FAIL':'1'})
                self.assertIn('PROBLEMA',result.stderr)
                self.assertNotEqual(result.returncode,0,'Falha emulada retornou sucesso')

    def test_sdl_resource_failures_return_error(self):
        for variant in self.binaries:
            for resource in ['window','renderer','texture']:
                with self.subTest(variant=variant,resource=resource):
                    result=self.run_host(variant,headless=False,env={'DESTINIA_TEST_SDL_FAIL':resource})
                    self.assertNotEqual(result.returncode,0,'Falha SDL retornou sucesso')
                    self.assertIn('falha injetada '+resource,result.stderr)

    def test_session_txt_is_created_with_start_and_exit(self):
        reports=self.base/'relatorios de teste'
        for variant in self.binaries:
            with self.subTest(variant=variant):
                result=self.run_host(variant,['--reports',str(reports)])
                self.assertEqual(result.returncode,0,result.stderr)
                logs=list(reports.glob('jogo_*/log*.txt'))
                self.assertTrue(logs,'Log TXT de sessao nao criado')
                text='\n'.join(p.read_text() for p in logs)
                self.assertIn((ROOT/'VERSION').read_text().strip()+' Beta',text)
                self.assertIn('initGame ok',text)
                self.assertIn('codigo=0',text)

    def test_backup_contains_original_progress_and_does_not_change_source(self):
        save=self.base/'save'; (save/'personagem').mkdir(parents=True)
        original=b'progresso antigo\x00\xff'
        (save/'personagem'/'marcador.bin').write_bytes(original)
        for variant in self.binaries:
            with self.subTest(variant=variant):
                result=self.run_host(variant)
                self.assertEqual(result.returncode,0,result.stderr)
                copies=list((self.base/'save_backups').glob('backup_*/personagem/marcador.bin'))
                self.assertTrue(copies,'Backup automatico nao foi criado')
                self.assertTrue(all(p.read_bytes()==original for p in copies))
                self.assertEqual((save/'personagem'/'marcador.bin').read_bytes(),original)

    def test_global_config_and_environment_share_save_across_hosts(self):
        local=self.base/'local app data'/'Destinia'; local.mkdir(parents=True)
        configured=self.base/'partida compartilhada'
        (local/'config.ini').write_text('save_dir='+str(configured)+'\n')
        for variant in self.binaries:
            with self.subTest(variant=variant):
                result=self.run_host(variant,use_save=False)
                self.assertEqual(result.returncode,0,result.stderr)
                self.assertTrue(configured.is_dir(),'Configuracao global ignorada')
                self.assertIn(str(configured).replace('\\','/'),result.stderr.replace('\\','/'))
        override=self.base/'pasta por ambiente'
        result=self.run_host('host',use_save=False,env={'DESTINIA_SAVE_DIR':str(override)})
        self.assertEqual(result.returncode,0,result.stderr)
        self.assertTrue(override.is_dir())
        result=self.run_host('host',env={'DESTINIA_SAVE_DIR':str(override)})
        self.assertIn(str(self.base/'save').replace('\\','/'),result.stderr.replace('\\','/'))

    def test_invalid_configuration_does_not_start_with_a_new_save(self):
        local=self.base/'local app data'/'Destinia'; local.mkdir(parents=True)
        (local/'config.ini').write_text('save_dir=\n')
        for variant in self.binaries:
            with self.subTest(variant=variant):
                result=self.run_host(variant,use_save=False)
                self.assertGreater(result.returncode,0)
                self.assertIn('save_dir vazio',result.stderr)
                self.assertFalse((local/'save').exists())

    def test_backup_failure_stops_before_initializing_game(self):
        save=self.base/'save'; save.mkdir(); (save/'partida.bin').write_bytes(b'progresso intacto')
        (self.base/'save_backups').write_text('nao e um diretorio')
        for variant in self.binaries:
            with self.subTest(variant=variant):
                result=self.run_host(variant)
                self.assertGreater(result.returncode,0)
                self.assertNotIn('initGame ok',result.stderr)
                self.assertEqual((save/'partida.bin').read_bytes(),b'progresso intacto')

    def test_malformed_elf_headers_return_error_without_crashing(self):
        original=(ROOT/'game/lib/armeabi/libdestinia_jni.so').read_bytes()
        cases=[]
        for name,offset,value,fmt in [('phoff',28,0xfffffff0,'I'),('phentsize',42,0,'H'),('phnum',44,0xffff,'H'),
                                       ('class',4,2,'B'),('endian',5,2,'B')]:
            data=bytearray(original); struct.pack_into('<'+fmt,data,offset,value); cases.append((name,data))
        cases.append(('truncated',original[:70]))
        phoff=struct.unpack_from('<I',original,28)[0]
        phnum=struct.unpack_from('<H',original,44)[0]
        segments=[(phoff+i*32,struct.unpack_from('<8I',original,phoff+i*32)) for i in range(phnum)]
        load=[p for _,p in segments if p[0]==1]
        dynamic=next(p for _,p in segments if p[0]==2)
        tags={}
        for offset in range(dynamic[1],dynamic[1]+dynamic[4],8):
            tag,value=struct.unpack_from('<iI',original,offset)
            if tag==0: break
            tags[tag]=(offset,value)
        def file_offset(address):
            return next(p[1]+address-p[2] for p in load if p[2]<=address<p[2]+p[4])
        def mutation(name,offset,value):
            data=bytearray(original); struct.pack_into('<I',data,offset,value); cases.append((name,data))
        mutation('segment_mem_overflow',next(off for off,p in segments if p[0]==1)+20,0xffffffff)
        mutation('string_table_bounds',tags[5][0]+4,0xfffffffc)
        mutation('string_size_bounds',tags[10][0]+4,0xffffffff)
        mutation('hash_bounds',tags[4][0]+4,0xfffffffc)
        mutation('symbol_count_bounds',file_offset(tags[4][1])+4,0xffffffff)
        mutation('symbol_name_bounds',file_offset(tags[6][1])+16,0xffffffff)
        mutation('relocation_target_bounds',file_offset(tags[17][1]),0xfffffffc)
        mutation('relocation_size_bounds',tags[18][0]+4,0xffffffff)
        mutation('relocation_symbol_bounds',file_offset(tags[23][1])+4,0xffffff16)
        long_names=bytearray(original)
        start=file_offset(tags[5][1]); size=tags[10][1]
        long_names[start:start+size]=b'\x00'+b'x'*(size-2)+b'\x00'
        cases.append(('oversized_symbol_names',long_names))
        unterminated=bytearray(original)
        for offset in range(dynamic[1],dynamic[1]+dynamic[4],8):
            tag,_=struct.unpack_from('<iI',unterminated,offset)
            if tag==0: struct.pack_into('<i',unterminated,offset,1)
        cases.append(('dynamic_without_terminator',unterminated))
        for variant in self.binaries:
            for name,data in cases:
                with self.subTest(variant=variant,case=name):
                    bad=self.base/(name+'.so'); bad.write_bytes(data)
                    result=self.run_host(variant,['--so',str(bad)])
                    self.assertGreater(result.returncode,0,'ELF invalido causou crash ou retornou sucesso')
                    self.assertIn('ELF',result.stderr)


if __name__=='__main__': unittest.main()
