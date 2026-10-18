import unittest,hashlib,struct
from build_wide960_probe import patch,ROOT,ORIGINAL
class WidePatch(unittest.TestCase):
    def test_exact_delta(self):
        raw=(ROOT/'game-test/ds9dw.exe').read_bytes();out,changes,changed=patch(raw)
        self.assertEqual(len(out),len(raw));self.assertEqual(len(changes),7)
        self.assertEqual(hashlib.sha256(raw).hexdigest(),ORIGINAL)
        self.assertEqual(out[0x33620:0x33640],raw[0x33620:0x33640])
        self.assertEqual(out[:0x400],raw[:0x400])
        self.assertAlmostEqual(struct.unpack_from('<f',out,0x1180)[0],5/3,places=6)
        for p in changes:
            if p['label']!='projection horizontal scale':self.assertEqual(int.from_bytes(bytes.fromhex(p['after']),'little'),960)
        self.assertTrue(changed)
    def test_modified_input_rejected(self):
        raw=bytearray((ROOT/'game-test/ds9dw.exe').read_bytes());raw[-1]^=1
        with self.assertRaises(ValueError):patch(bytes(raw))
if __name__=='__main__':unittest.main()
