import unittest
from build_selection_loading_v1 import ROOT,build
import pefile
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
class RepaintTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.raw=(ROOT/'game-fleet-origin-v1/ds9dw.exe').read_bytes()
        cls.out,cls.report=build(cls.raw,True,False)
    def test_fill_argument_dereferences_holder(self):
        code=bytes.fromhex(self.report['stub_bytes'])
        self.assertTrue(code.startswith(bytes.fromhex('9c60a1649ba3008b006a0050')))
        self.assertNotIn(bytes.fromhex('ff35649ba300'),code)
    def test_only_menu_site_changed(self):
        self.assertEqual([r['va'] for r in self.report['patches']],['0x472630'])
        a=pefile.PE(data=self.raw);b=pefile.PE(data=self.out)
        self.assertEqual(a.get_data(0x344b0,0x3e3),b.get_data(0x344b0,0x3e3))
    def test_valid_decode_and_replay(self):
        code=bytes.fromhex(self.report['stub_bytes']);va=int(self.report['stub_va'],16)
        rows=list(Cs(CS_ARCH_X86,CS_MODE_32).disasm(code,va))
        self.assertEqual(sum(r.size for r in rows),len(code))
        self.assertEqual(rows[-1].op_str,'0x472635')
        self.assertEqual([r.mnemonic for r in rows[-4:]],['popal','popfd','mov','jmp'])
    def test_bad_base_rejected(self):
        with self.assertRaises(ValueError):build(self.raw[:-1],True,False)
if __name__=='__main__':unittest.main()
