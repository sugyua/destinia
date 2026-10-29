from pathlib import Path
import json
import unittest
import re

ROOT=Path(__file__).resolve().parents[1]
class ReleaseCoherence(unittest.TestCase):
    def test_current_version_matches_entrypoints(self):
        v=(ROOT/'VERSION').read_text().strip()
        self.assertEqual(json.loads((ROOT/'release/manifest.json').read_text())['version'],v)
        web_version=json.loads((ROOT/'web/package.json').read_text())['version']
        self.assertEqual(web_version,'0.13.0-beta')
        readme=(ROOT/'README.md').read_text(encoding='utf-8')
        self.assertTrue(readme.startswith('# Destinia '+v+' Beta\n'))
        self.assertIn('/releases/tag/v'+v+'-beta','\n'.join(readme.splitlines()[:12]))
        self.assertIn('## '+v+' Beta',(ROOT/'CHANGELOG.md').read_text(encoding='utf-8'))
        self.assertIn(web_version.rsplit('-beta',1)[0]+' WEB',(ROOT/'web/dist/index.html').read_text(encoding='utf-8'))
        for host in ['host','host_zoom']:
            source=(ROOT/host/'main.cpp').read_text(encoding='utf-8')
            self.assertIn('"Destinia " DESTINIA_VERSION " Beta"',source)
            self.assertIn('../common/version_stamp.h',source)
            self.assertNotIn('Destinia 0.12.0 Beta',source)
            cmake=(ROOT/host/'CMakeLists.txt').read_text(encoding='utf-8')
            self.assertIn('VERSION" DESTINIA_VERSION',cmake)
        self.assertTrue((ROOT/'release'/('NOTAS_'+v+'.md')).is_file())

    def test_source_manifest_covers_local_cpp_includes(self):
        files=set(json.loads((ROOT/'release/manifest.json').read_text())['files'])
        pending=[ROOT/'host/main.cpp',ROOT/'host_zoom/main.cpp'];seen=set()
        while pending:
            p=pending.pop().resolve()
            if p in seen:continue
            seen.add(p)
            self.assertIn(p.relative_to(ROOT).as_posix(),files)
            for name in re.findall(r'^\s*#include\s+"([^"]+)"',p.read_text(encoding='utf-8'),re.M):
                child=(p.parent/name).resolve()
                if child.is_file() and child.is_relative_to(ROOT):pending.append(child)

    def test_release_does_not_bundle_external_unicorn_library(self):
        files=json.loads((ROOT/'release/manifest.json').read_text())['files']
        self.assertNotIn('web/dist/vendor/unicorn_arm.js',files)
        self.assertIn('/web/dist/vendor/unicorn_arm.js export-ignore',(ROOT/'.gitattributes').read_text())
