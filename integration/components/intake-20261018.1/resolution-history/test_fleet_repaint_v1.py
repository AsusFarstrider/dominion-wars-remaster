import unittest,sys
from build_fleet_repaint_v1 import build,ROOT
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
class FleetRepaint(unittest.TestCase):
    def test_delta_and_stub(self):
        raw=(ROOT/'game-dialog-center-v1/ds9dw.exe').read_bytes();out,r=build(raw)
        allowed=set(range(r['virtual_size_field'],r['virtual_size_field']+4))
        for p in r['patches']:allowed.update(range(p['file_offset'],p['file_offset']+len(bytes.fromhex(p['before']))))
        allowed.update(range(r['stub_file_offset'],r['stub_file_offset']+len(bytes.fromhex(r['stub_bytes']))))
        self.assertEqual(len(raw),len(out))
        self.assertTrue(all(i in allowed for i,(a,b) in enumerate(zip(raw,out)) if a!=b))
        ins=list(Cs(CS_ARCH_X86,CS_MODE_32).disasm(bytes.fromhex(r['stub_bytes']),int(r['stub_va'],16)))
        self.assertEqual([i.op_str for i in ins if i.mnemonic=='call'],['0x4a1710'])
        self.assertEqual(ins[-1].op_str,'0x46f036')
        self.assertEqual([i.mnemonic for i in ins][:2],['pushfd','pushal'])
        self.assertEqual([i.mnemonic for i in ins][-4:],['popal','popfd','sub','jmp'])
    def test_wrong_hash(self):
        with self.assertRaises(ValueError):build(b'wrong')
if __name__=='__main__':unittest.main()
