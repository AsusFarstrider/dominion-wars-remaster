import struct
from test_layout_trace_v3 import LayoutObserverTests
from build_layout_trace_v3 import ROOT, build
import pefile


class UniqueLayoutObserverTests(LayoutObserverTests):
    @classmethod
    def setUpClass(cls):
        cls.raw = (ROOT/'game-fleet-middle-v1/ds9dw.exe').read_bytes()
        cls.out, cls.report = build(cls.raw, unique=True)

    def test_version_and_capacity(self):
        pe = pefile.PE(data=self.out)
        self.assertEqual(struct.unpack('<IIII',pe.sections[-1].get_data()[:16]),
                         (0x44575458,4,0,256))

    def test_distinct_rectangle_comparison_and_stack_arguments(self):
        code = bytes.fromhex(self.report['stub_bytes'])
        for sequence in ('8b74242c', '8b5424248b1e8b6e04', '3917',
                         '395f04', '396f08', '83c72049'):
            self.assertIn(bytes.fromhex(sequence),code)
        # Entry ESP: return +0, text +4, rectangle +8; observer saves 36 bytes.
        self.assertEqual(36+8,0x2c)
        self.assertEqual(36+12,0x30)
        self.assertEqual(36+16,0x34)
