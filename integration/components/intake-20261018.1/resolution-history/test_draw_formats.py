import unittest
from analyze_draw_formats import analyze

BASE = 'DWDRAW v=1 tick=100 device=&123 kind=1 primitive=4 fvf=274 position=2 vertices=128 indices=300 hr=0 matrix_hr=0 m11=2 m22=3.5 viewport_hr=0 viewport=800x446 origin=0,0 vb_desc_hr=-2147467263'

class DrawFormats(unittest.TestCase):
    def test_xyz_scene(self):
        r=analyze(BASE)
        self.assertEqual(r['main_scene_position_classes'],{'XYZ':1})
    def test_transformed(self):
        r=analyze(BASE.replace('fvf=274 position=2','fvf=452 position=4'))
        self.assertEqual(r['main_scene_position_classes'],{'XYZRHW':1})
    def test_failed_draw_not_evidence(self):
        self.assertEqual(analyze(BASE.replace(' hr=0 ',' hr=-1 '))['accepted_records'],0)
    def test_failed_viewport_not_scene(self):
        self.assertEqual(analyze(BASE.replace('viewport_hr=0','viewport_hr=-1'))['main_scene_candidate_records'],0)
    def test_empty(self):
        self.assertEqual(analyze('ordinary log')['records'],0)
    def test_bad_mask_rejected(self):
        with self.assertRaises(AssertionError): analyze(BASE.replace('position=2','position=4'))
    def test_cap(self):
        with self.assertRaises(AssertionError): analyze('\n'.join([BASE]*97))
    def test_missing_field(self):
        with self.assertRaises(KeyError): analyze('DWDRAW v=1 kind=1')

if __name__=='__main__': unittest.main()
