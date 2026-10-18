import unittest
from build_banner_trace_v1 import build,ROOT
import pefile
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
class BannerTrace(unittest.TestCase):
    def test_sections_and_hook(self):
        raw=(ROOT/'game-fleet-middle-v1/ds9dw.exe').read_bytes();out,r=build(raw)
        a=pefile.PE(data=raw);b=pefile.PE(data=out)
        self.assertEqual(b.sections[-1].Name.rstrip(b'\0'),b'.dwobs')
        self.assertEqual(b.sections[-1].Characteristics,0xc0000040)
        self.assertEqual(b.get_data(0x33620,256),a.get_data(0x33620,256))
        self.assertEqual(b.OPTIONAL_HEADER.AddressOfEntryPoint,a.OPTIONAL_HEADER.AddressOfEntryPoint)
        ins=list(Cs(CS_ARCH_X86,CS_MODE_32).disasm(bytes.fromhex(r['stub_bytes']),int(r['stub_va'],16)))
        self.assertEqual(ins[-1].op_str,'0x4a2658')
        self.assertFalse(any(i.mnemonic=='call' for i in ins))
        self.assertEqual([i.mnemonic for i in ins][:2],['pushfd','pushal'])
        self.assertEqual(r['capacity'],64)
        # Jump targets must resolve to decoded instructions within the stub.
        addresses={i.address for i in ins}
        for i in ins:
            if i.mnemonic.startswith('j') and i.mnemonic!='jmp':self.assertIn(int(i.op_str,16),addresses)
    def test_hash_gate(self):
        with self.assertRaises(ValueError):build(b'wrong')
    def test_broad_capture_branches(self):
        raw=(ROOT/'game-fleet-middle-v1/ds9dw.exe').read_bytes();out,r=build(raw,True)
        self.assertEqual(r['capacity'],256)
        ins=list(Cs(CS_ARCH_X86,CS_MODE_32).disasm(bytes.fromhex(r['stub_bytes']),int(r['stub_va'],16)))
        addresses={i.address for i in ins}
        for i in ins:
            if i.mnemonic.startswith('j') and i.mnemonic!='jmp':self.assertIn(int(i.op_str,16),addresses)
        self.assertIn('unique',r['filter'])
if __name__=='__main__':unittest.main()
