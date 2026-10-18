import hashlib,unittest
from build_selection_loading_v1 import ROOT,build
import pefile
from capstone import Cs,CS_ARCH_X86,CS_MODE_32

class SelectionLoadingTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.raw=(ROOT/'game-fleet-origin-v1/ds9dw.exe').read_bytes()
        cls.out,cls.report=build(cls.raw)

    def test_reject_wrong_image(self):
        with self.assertRaises(ValueError):build(self.raw[:-1])

    def test_only_four_sites(self):
        self.assertEqual([x['va'] for x in self.report['patches']],
                         ['0x472630','0x434569','0x4345a3','0x434625'])

    def test_prior_fleet_and_notification_calls_unchanged(self):
        a=pefile.PE(data=self.raw);b=pefile.PE(data=self.out)
        for rva,count in ((0x6f030,0x3239),(0x94fc0,0x22d),(0x34700,0x193)):
            self.assertEqual(a.get_data(rva,count),b.get_data(rva,count))

    def test_stub_decoding_and_targets(self):
        code=bytes.fromhex(self.report['stub_bytes']);start=int(self.report['stub_va'],16)
        rows=list(Cs(CS_ARCH_X86,CS_MODE_32).disasm(code,start))
        self.assertEqual(sum(i.size for i in rows),len(code))
        self.assertEqual([i.op_str for i in rows if i.mnemonic=='call'],['0x4a1710'])
        self.assertEqual([i.op_str for i in rows if i.mnemonic=='jmp'],
                         ['0x472635','0x434570','0x4345a9','0x43462c'])
        self.assertEqual(code.count(bytes.fromhex('0fbf05e07e5f00')),3)

    def test_image_hash(self):
        self.assertEqual(hashlib.sha256(self.out).hexdigest(),self.report['candidate_sha256'])

if __name__=='__main__':unittest.main()
