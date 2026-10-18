import unittest
from build_panel_details_v1 import build,ROOT
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
class PanelDetails(unittest.TestCase):
    def test_delta(self):
        raw=(ROOT/'game-brief-center-v1/ds9dw.exe').read_bytes();out,r=build(raw)
        allowed=set(range(r['virtual_size_field'],r['virtual_size_field']+4))
        for p in r['patches']:allowed.update(range(p['file_offset'],p['file_offset']+len(bytes.fromhex(p['before']))))
        allowed.update(range(r['stub_file_offset'],r['stub_file_offset']+len(bytes.fromhex(r['stub_bytes']))))
        self.assertEqual(len(raw),len(out));self.assertEqual(len(r['patches']),5)
        self.assertTrue(all(i in allowed for i,(a,b) in enumerate(zip(raw,out)) if a!=b))
        ins=list(Cs(CS_ARCH_X86,CS_MODE_32).disasm(bytes.fromhex(r['stub_bytes']),int(r['stub_va'],16)))
        self.assertEqual([i.op_str for i in ins if i.mnemonic=='jmp'],['0x40f630','0x49f050'])
        self.assertFalse(any(i.mnemonic.startswith(('add','sub')) for i in ins))
    def test_hash_gate(self):
        with self.assertRaises(ValueError):build(b'wrong')
    def test_top_info_variant(self):
        raw=(ROOT/'game-brief-center-v1/ds9dw.exe').read_bytes()
        a,ra=build(raw);b,rb=build(raw,True)
        added=[p for p in rb['patches'] if p['va']=='0x46f044']
        self.assertEqual(len(added),1);self.assertEqual(len(rb['patches']),6)
        pos=added[0]['file_offset']
        self.assertEqual(a[:pos],b[:pos]);self.assertEqual(a[pos+5:],b[pos+5:])
if __name__=='__main__':unittest.main()
