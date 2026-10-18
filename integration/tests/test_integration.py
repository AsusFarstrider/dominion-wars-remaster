import base64
import json
import os
import struct
import sys
import unittest
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from binary_delta import make_delta,make_bolt_delta,apply_delta,sha
from build_remaster import INTAKE,compose_engine


class DeltaTests(unittest.TestCase):
    def test_exact_round_trip_growing_and_shrinking(self):
        for old,new in ((b'abc',b'ABCde'),(b'old large',b'new'),(b'no change',b'no change')):
            self.assertEqual(apply_delta(old,make_delta(old,new)),new)

    def test_source_hash_not_just_size_required(self):
        patch=make_delta(b'abc',b'new')
        with self.assertRaises(ValueError):apply_delta(b'xyz',patch)

    def test_bad_output_hash_rejected(self):
        patch=make_delta(b'abc',b'new');patch['output_sha256']='0'*64
        with self.assertRaises(ValueError):apply_delta(b'abc',patch)

    def test_copy_bounds_and_operation_kind(self):
        for bad in (dict(kind='copy',offset=-1,length=1),dict(kind='copy',offset=2,length=20),dict(kind='run',data='ignored')):
            patch=make_delta(b'abc',b'new');patch['operations']=[bad]
            with self.assertRaises(ValueError):apply_delta(b'abc',patch)

    def test_invalid_base64_and_size_rejected(self):
        patch=make_delta(b'abc',b'new');patch['operations']=[dict(kind='bytes',data='!bad')]
        with self.assertRaises(ValueError):apply_delta(b'abc',patch)
        patch['output_size']=129*1024*1024
        with self.assertRaises(ValueError):apply_delta(b'abc',patch)

    def test_assets_require_real_bolt_directory(self):
        with self.assertRaises(ValueError):make_bolt_delta(b'not an archive',b'also invalid')


class CompositionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        path=Path(os.environ.get('DW_TEST_ORIGINAL_GAME','E:/Codex/research/installed-1.04'))/'ds9dw.exe'
        if not path.exists():raise unittest.SkipTest('Supply owned canonical game through DW_TEST_ORIGINAL_GAME')
        cls.original=path.read_bytes()
        cls.lock=json.loads((INTAKE/'source-lock.json').read_text())
        cls.combined,cls.report=compose_engine(cls.original,cls.lock)

    def test_resolution_preserved_outside_checked_input_port(self):
        self.assertTrue(self.report['resolution_bytes_preserved_outside_input_port'])
        self.assertTrue(self.report['source_native_stub_body_preserved'])

    def test_new_input_cave_is_not_old_conflicting_cave(self):
        old=json.loads((INTAKE/'launcher/input-window-patch-v2.json').read_text())
        self.assertNotEqual(self.report['input_hook']['stub_va'],old['stub_va'])
        self.assertEqual(self.report['input_hook']['hwnd_address'],'0xa387a4')

    def test_game_logic_remains_original_except_identified_patch(self):
        self.assertNotEqual(sha(self.combined),self.lock['resolution_sha256'])
        self.assertEqual(len(self.combined),995328)
        self.assertEqual(self.combined[:2],b'MZ')
        self.assertEqual(apply_delta(self.original,make_delta(self.original,self.combined)),self.combined)

    def test_wrong_version_rejected(self):
        altered=bytearray(self.original);altered[-1]^=1
        with self.assertRaises(ValueError):compose_engine(bytes(altered),self.lock)

    def test_input_body_uses_no_global_metric_spoof(self):
        code=bytes.fromhex(self.report['input_hook']['stub_bytes'])
        self.assertIn(bytes.fromhex('ff35a487a300ff1578d24b00'),code)
        self.assertIn(bytes.fromhex('66a3e49eab00668915e69eab00'),code)
        self.assertIn(bytes.fromhex('9c60'),code[:2])

    def test_loading_and_menu_hooks_not_overwritten_by_input(self):
        import pefile
        before=pefile.PE(data=apply_delta(self.original,json.loads((INTAKE/'resolution.patch.json').read_text())))
        after=pefile.PE(data=self.combined)
        for rva,length in ((0x72630,5),(0x34569,7),(0x345a3,6),(0x34625,7),(0x34700,0x193)):
            self.assertEqual(before.get_data(rva,length),after.get_data(rva,length))


if __name__=='__main__':unittest.main()
