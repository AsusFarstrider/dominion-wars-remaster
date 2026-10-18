import unittest
from build_brief_center_v1 import build,ROOT
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
class BriefCenter(unittest.TestCase):
    def test_delta(self):
        raw=(ROOT/'game-fleet-repaint-v1/ds9dw.exe').read_bytes();out,r=build(raw)
        allowed=set(range(r['virtual_size_field'],r['virtual_size_field']+4))
        for p in r['patches']:allowed.update(range(p['file_offset'],p['file_offset']+len(bytes.fromhex(p['before']))))
        allowed.update(range(r['stub_file_offset'],r['stub_file_offset']+len(bytes.fromhex(r['stub_bytes']))))
        self.assertEqual(len(out),len(raw));self.assertEqual(len(r['patches']),15)
        self.assertTrue(all(i in allowed for i,(a,b) in enumerate(zip(raw,out)) if a!=b))
        ins=list(Cs(CS_ARCH_X86,CS_MODE_32).disasm(bytes.fromhex(r['stub_bytes']),int(r['stub_va'],16)))
        self.assertEqual([i.op_str for i in ins if i.mnemonic=='call'],['0x4a1710'])
        self.assertEqual([i.op_str for i in ins if i.mnemonic=='jmp'],['0x415b16','0x40f630','0x49f050'])
    def test_stack_slots(self):
        # Wrapper return address + arguments, then saved EAX: positions of X.
        self.assertEqual(4+4+4,12) # image,X,Y
        self.assertEqual(4+4+4+4,16) # image,surface,X,Y
    def test_hash_gate(self):
        with self.assertRaises(ValueError):build(b'wrong')
if __name__=='__main__':unittest.main()
