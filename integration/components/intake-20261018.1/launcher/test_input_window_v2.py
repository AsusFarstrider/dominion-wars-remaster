import unittest
from test_input_window_v1 import InputWindowTests
from build_input_window_v1 import build,SOURCE

class InputWindowV2Tests(InputWindowTests):
 @classmethod
 def setUpClass(cls):
  cls.raw=SOURCE.read_bytes();cls.out,cls.report=build(cls.raw,hwnd_address=0xa387a4)
 def test_import_and_cache_operands(self):
  code=bytes.fromhex(self.report['stub_bytes'])
  for op in ('ff35a487a300','ff1578d24b00','66a3e49eab00','668915e69eab00'):
   self.assertIn(bytes.fromhex(op),code)
 def test_window_handle_provenance(self):
  self.assertEqual(0xa38770+0x34,0xa387a4)
  self.assertEqual(self.report['hwnd_address'],'0xa387a4')
  self.assertNotIn(bytes.fromhex('ff35e83d7100'),bytes.fromhex(self.report['stub_bytes']))

if __name__=='__main__':unittest.main()
