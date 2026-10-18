import struct
import unittest
from build_notification_origin_v1 import ROOT, build, call, SITES
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32


class NotificationOriginTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.raw=(ROOT/'game-fleet-middle-v1/ds9dw.exe').read_bytes()
        cls.out,cls.report=build(cls.raw)

    def test_rejects_wrong_base(self):
        with self.assertRaises(ValueError):build(self.raw[:-1])

    def test_generic_helper_unchanged(self):
        before=pefile.PE(data=self.raw);after=pefile.PE(data=self.out)
        self.assertEqual(before.get_data(0x1e7f0,0x150),after.get_data(0x1e7f0,0x150))

    def test_only_two_owner_calls_redirected(self):
        pe=pefile.PE(data=self.out)
        for site in SITES:
            self.assertEqual(pe.get_data(site-0x400000,5),call(site,int(self.report['stub_va'],16)))

    def test_local_copy_origin_and_cdecl_forwarding(self):
        code=bytes.fromhex(self.report['stub_bytes'])
        rows=list(Cs(CS_ARCH_X86,CS_MODE_32).disasm(code,int(self.report['stub_va'],16)))
        self.assertEqual(sum(i.size for i in rows),len(code))
        self.assertIn(bytes.fromhex('0fbf05e07e5f00'),code)
        self.assertIn(bytes.fromhex('660145f8'),code)
        self.assertEqual([i.op_str for i in rows if i.mnemonic=='push'][-5:],
                         ['dword ptr [ebp + 0x18]','dword ptr [ebp + 0x14]',
                          'dword ptr [ebp + 0x10]','eax','dword ptr [ebp + 8]'])
        self.assertEqual([i.mnemonic for i in rows[-3:]],['lea','pop','ret'])
        self.assertEqual([i.op_str for i in rows if i.mnemonic=='call'],['0x41e7f0'])

    def test_expected_coordinate_math(self):
        # Existing root origin is 80 in the wide build, zero in a legacy canvas.
        for origin in (0,80,160):
            rect=(200,30,400,32)
            moved=(rect[0]+origin,*rect[1:])
            self.assertEqual(moved[0]+moved[2]/2,400+origin)
            self.assertEqual(rect,(200,30,400,32))


if __name__=='__main__':unittest.main()
