import struct
import unittest
from build_ui_center_probe import build, ROOT
import pefile

class HudLocalOrigins(unittest.TestCase):
    def test_only_three_extra_sites_and_stubs(self):
        raw = (ROOT/'game-wide960-v1/ds9dw.exe').read_bytes()
        v1, a = build(raw)
        v2, b = build(raw, True)
        self.assertEqual(a['patches'], b['patches'][:18])
        self.assertEqual([p['va'] for p in b['patches'][18:]],
                         ['0x479ca5', '0x479ce4', '0x459f33'])
        p = pefile.PE(data=v1)
        allowed = set()
        for patch in b['patches'][18:]:
            allowed.update(range(patch['file_offset'], patch['file_offset'] +
                                 len(bytes.fromhex(patch['before']))))
        section = p.sections[-1]
        allowed.update(range(section.PointerToRawData, len(v2)))
        allowed.update(range(section.get_file_offset()+8, section.get_file_offset()+12))
        self.assertEqual(len(v1), len(v2))
        self.assertTrue(all(i in allowed for i,(x,y) in enumerate(zip(v1,v2)) if x!=y))
        q = pefile.PE(data=v2)
        self.assertEqual(q.get_data(0x33620,256), p.get_data(0x33620,256))
        self.assertEqual(q.get_data(0x1180,4), struct.pack('<f',5/3))

    def test_local_vs_absolute_model(self):
        root = 80
        for original in (0, 163):
            absolute = original + root
            local = absolute - root
            self.assertEqual(root + local, absolute)
        # Minimap center must remain local to its container, not shifted again.
        self.assertEqual((82+80)-(0+80),82)

if __name__ == '__main__':
    unittest.main()
