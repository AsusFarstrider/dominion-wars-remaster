import unittest
from build_movie_center_v1 import build,ROOT
class MovieOrigin(unittest.TestCase):
    def test_exact_delta(self):
        raw=(ROOT/'game-strip-clear-v1/ds9dw.exe').read_bytes();out,r=build(raw)
        allowed=set(range(r['virtual_size_field'],r['virtual_size_field']+4))
        for p in r['patches']:allowed.update(range(p['file_offset'],p['file_offset']+len(bytes.fromhex(p['before']))))
        allowed.update(range(r['stub_file_offset'],r['stub_file_offset']+len(bytes.fromhex(r['stub_bytes']))))
        self.assertEqual(len(out),len(raw))
        self.assertTrue(all(i in allowed for i,(a,b) in enumerate(zip(raw,out)) if a!=b))
    def test_placement(self):
        for width,x in ((800,0),(640,80)):
            self.assertEqual(x+80,(960-width)//2)
    def test_hash_gate(self):
        with self.assertRaises(ValueError):build(b'not an executable')
if __name__=='__main__':unittest.main()
