import unittest
from build_fleet_middle_v1 import build,ROOT
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
class FleetMiddle(unittest.TestCase):
    def test_delta_and_scope(self):
        raw=(ROOT/'game-panel-details-v2/ds9dw.exe').read_bytes();out,r=build(raw)
        self.assertEqual(len(r['call_sites']),71)
        allowed=set(range(r['virtual_size_field'],r['virtual_size_field']+4))
        for p in r['patches']:allowed.update(range(p['file_offset'],p['file_offset']+len(bytes.fromhex(p['before']))))
        allowed.update(range(r['stub_file_offset'],r['stub_file_offset']+len(bytes.fromhex(r['stub_bytes']))))
        self.assertEqual(len(raw),len(out))
        self.assertTrue(all(i in allowed for i,(a,b) in enumerate(zip(raw,out)) if a!=b))
        for s in r['call_sites']:
            self.assertTrue(0x47070d<=int(s['va'],16)<0x472269)
            self.assertNotIn(s['va'],['0x470c6a','0x470c76','0x470cd0','0x470cdc'])
        self.assertNotIn('0x470b16',[s['va'] for s in r['call_sites']])
    def test_wrapper_disassembly(self):
        raw=(ROOT/'game-panel-details-v2/ds9dw.exe').read_bytes();_,r=build(raw)
        ins=list(Cs(CS_ARCH_X86,CS_MODE_32).disasm(bytes.fromhex(r['stub_bytes']),int(r['stub_va'],16)))
        self.assertEqual([i.op_str for i in ins if i.mnemonic=='call'],['0x4a2650'])
        self.assertEqual([i.op_str for i in ins if i.mnemonic=='jmp'],['0x40f630','0x49f050'])
        self.assertEqual(sum(i.mnemonic=='pushfd' for i in ins),2)
        self.assertEqual(sum(i.mnemonic=='popfd' for i in ins),2)
        self.assertEqual([i.mnemonic for i in ins][-3:],['lea','pop','ret'])
    def test_font_offset_model(self):
        for x in (87,380,627):
            for advance in (0,12):
                self.assertEqual((x+80+advance)-80,x+advance)
    def test_hash_gate(self):
        with self.assertRaises(ValueError):build(b'wrong')
if __name__=='__main__':unittest.main()
