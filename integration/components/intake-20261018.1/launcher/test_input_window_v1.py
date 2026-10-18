import unittest,struct
from build_input_window_v1 import build,SOURCE
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
import pefile

class InputWindowTests(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  cls.raw=SOURCE.read_bytes();cls.out,cls.report=build(cls.raw)
 def test_source_gate(self):
  with self.assertRaises(ValueError):build(self.raw[:-1])
 def test_replay_and_balanced_local_frame(self):
  code=bytes.fromhex(self.report['stub_bytes']);va=int(self.report['stub_va'],16)
  rows=list(Cs(CS_ARCH_X86,CS_MODE_32).disasm(code,va));starts={i.address for i in rows}
  self.assertEqual(sum(i.size for i in rows),len(code))
  self.assertEqual(rows[0].mnemonic,'pushfd');self.assertEqual(rows[1].mnemonic,'pushal')
  self.assertIn(bytes.fromhex('83c410619d')+bytes.fromhex(self.report['displaced_bytes']),code)
  for row in rows:
   if row.mnemonic.startswith('j'):
    self.assertTrue(int(row.op_str,16) in starts or int(row.op_str,16)==0x49e5fb)
 def test_original_display_metric_initialization_unchanged(self):
  before=pefile.PE(data=self.raw);after=pefile.PE(data=self.out)
  self.assertEqual(before.get_data(0x29865,0x3d),after.get_data(0x29865,0x3d))
 def test_import_and_cache_operands(self):
  code=bytes.fromhex(self.report['stub_bytes'])
  for op in ('ff35e83d7100','ff1578d24b00','66a3e49eab00','668915e69eab00'):
   self.assertIn(bytes.fromhex(op),code)
 def test_hook_returns_after_exact_displaced_span(self):
  pe=pefile.PE(data=self.out);hook=pe.get_data(0x9e5ee,13)
  self.assertEqual(hook[0],0xe9);self.assertEqual(hook[5:],b'\x90'*8)

if __name__=='__main__':unittest.main()
