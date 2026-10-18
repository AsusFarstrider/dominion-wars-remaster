import hashlib
import struct
import unittest
from build_layout_trace_v3 import ROOT, build
import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32


class LayoutObserverTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.raw = (ROOT/'game-fleet-middle-v1/ds9dw.exe').read_bytes()
        cls.out, cls.report = build(cls.raw)

    def test_exact_source_required(self):
        with self.assertRaises(ValueError): build(self.raw[:-1])

    def test_font_hook_removed_and_helper_redirected(self):
        pe = pefile.PE(data=self.out)
        self.assertEqual(pe.get_data(0xa2650,8),bytes.fromhex('83ec34a1789ba300'))
        hook = pe.get_data(0x1e7f0,5)
        self.assertEqual(hook[0],0xe9)
        self.assertEqual(0x41e7f5+struct.unpack('<i',hook[1:])[0],
                         int(self.report['stub_va'],16))

    def test_observer_restores_state_and_replays_displaced_instruction(self):
        code = bytes.fromhex(self.report['stub_bytes'])
        va = int(self.report['stub_va'],16)
        rows = list(Cs(CS_ARCH_X86,CS_MODE_32).disasm(code,va))
        starts = {i.address for i in rows}
        self.assertEqual(sum(i.size for i in rows),len(code))
        for i in rows:
            if i.mnemonic.startswith('j'):
                target = int(i.op_str,16)
                self.assertTrue(target in starts or target==0x41e7f5)
        self.assertEqual([i.mnemonic for i in rows[-4:]],['popal','popfd','mov','jmp'])
        self.assertEqual(rows[-2].op_str,'eax, 0x1000c')
        self.assertFalse(any(i.mnemonic=='call' for i in rows))

    def test_manifest_matches_image(self):
        self.assertEqual(hashlib.sha256(self.out).hexdigest(),self.report['candidate_sha256'])


if __name__ == '__main__': unittest.main()
