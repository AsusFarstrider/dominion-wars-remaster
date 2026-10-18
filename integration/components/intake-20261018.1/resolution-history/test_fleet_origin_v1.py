import hashlib,struct,unittest
from build_fleet_origin_v1 import ROOT,build,branch,CLIPS
import pefile
from capstone import Cs,CS_ARCH_X86,CS_MODE_32

class FleetOriginTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.raw=(ROOT/'game-notification-origin-v1/ds9dw.exe').read_bytes()
        cls.original=(ROOT/'game-test/ds9dw.exe').read_bytes()
        cls.out,cls.report=build(cls.raw,cls.original)

    def test_exact_inputs(self):
        with self.assertRaises(ValueError):build(self.raw[:-1],self.original)
        with self.assertRaises(ValueError):build(self.raw,self.original[:-1])

    def test_bounded_site_inventory(self):
        self.assertEqual(len(self.report['call_sites']),137)
        self.assertEqual(len(self.report['restored_literal_sites']),7)
        self.assertEqual(len(self.report['clip_sites']),10)
        self.assertEqual(len({r['va'] for r in self.report['patches']}),154)

    def test_existing_notification_and_global_helpers_untouched(self):
        a=pefile.PE(data=self.raw);b=pefile.PE(data=self.out)
        for rva,size in ((0x95126,5),(0x951cb,5),(0xa2650,0x100),
                         (0xf630,0x20),(0x9f050,0x20),(0x1e7f0,0x150),
                         (0x6f0ce,5),(0x6f0da,5)):
            self.assertEqual(a.get_data(rva,size),b.get_data(rva,size))

    def test_stubs_decode_and_references_are_bounded(self):
        code=bytes.fromhex(self.report['stub_bytes']);start=int(self.report['stub_va'],16)
        rows=list(Cs(CS_ARCH_X86,CS_MODE_32).disasm(code,start))
        self.assertEqual(sum(i.size for i in rows),len(code))
        for i in rows:
            if i.mnemonic in ('call','jmp'):
                self.assertIn(int(i.op_str,16),(0x40f630,0x49f050,0x4a2650,0x41e7f0))
        self.assertNotIn('0x50', [i.op_str for i in rows if i.mnemonic=='add'])

    def test_every_draw_call_points_at_its_adapter(self):
        p=pefile.PE(data=self.out)
        for r in self.report['call_sites']:
            va=int(r['va'],16);target=int(self.report['wrappers'][r['destination']],16)
            self.assertEqual(p.get_data(va-0x400000,5),branch(0xe8,va,target))

    def test_clip_slots_account_for_call_and_saved_state(self):
        code=bytes.fromhex(self.report['stub_bytes'])
        for slot,kind,value in CLIPS.values():
            self.assertIn(bytes.fromhex('66894424')+bytes([slot+12]),code)
        self.assertEqual(hashlib.sha256(self.out).hexdigest(),self.report['candidate_sha256'])

if __name__=='__main__':unittest.main()
