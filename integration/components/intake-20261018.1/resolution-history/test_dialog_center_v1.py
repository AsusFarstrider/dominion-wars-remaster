import unittest
from build_dialog_center_v1 import build,ROOT
class DialogCenter(unittest.TestCase):
    def test_exact_delta(self):
        raw=(ROOT/'game-movie-center-v1/ds9dw.exe').read_bytes();out,r=build(raw);pos=r['file_offset']
        self.assertEqual(len(raw),len(out));self.assertEqual(raw[:pos],out[:pos]);self.assertEqual(raw[pos+4:],out[pos+4:])
        self.assertEqual(out[pos:pos+4],bytes.fromhex('66bf2003'))
    def test_center_model(self):
        for width in (200,300,400):
            self.assertEqual(80+800//2-width//2,960//2-width//2)
    def test_hash_gate(self):
        with self.assertRaises(ValueError):build(b'wrong image')
if __name__=='__main__':unittest.main()
