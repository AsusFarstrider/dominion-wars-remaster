import unittest
from build_strip_clear_v1 import build, ROOT

class StripClear(unittest.TestCase):
    def test_exact_delta(self):
        raw = (ROOT/'game-ui-center-v2/ds9dw.exe').read_bytes()
        out, report = build(raw)
        pos = report['file_offset']
        self.assertEqual(len(out), len(raw))
        self.assertEqual(out[:pos], raw[:pos])
        self.assertEqual(out[pos+5:], raw[pos+5:])
        self.assertEqual(out[pos:pos+5], bytes.fromhex('b800000000'))
    def test_rectangle_model(self):
        # Existing code writes x1=EAX and x2=width-EAX, y1=H-strip,y2=H.
        self.assertEqual((0,600-154,960-0,600),(0,446,960,600))
        self.assertEqual((243,446,960-243,600),(243,446,717,600))
    def test_wrong_image_rejected(self):
        raw = bytearray((ROOT/'game-ui-center-v2/ds9dw.exe').read_bytes())
        raw[-1] ^= 1
        with self.assertRaises(ValueError): build(bytes(raw))

if __name__ == '__main__': unittest.main()
