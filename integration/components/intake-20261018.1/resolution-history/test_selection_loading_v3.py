import unittest
from build_selection_loading_v1 import ROOT,build
import pefile
class LoadingTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.base=(ROOT/'game-fleet-origin-v1/ds9dw.exe').read_bytes()
        cls.menu,cls.mr=build(cls.base,True,False)
        cls.out,cls.report=build(cls.base,True,True)
    def test_passed_menu_stub_identical(self):
        a=pefile.PE(data=self.menu);b=pefile.PE(data=self.out)
        va=int(self.mr['stub_va'],16)
        count=len(bytes.fromhex(self.mr['stub_bytes']))
        self.assertEqual(a.get_data(va-0x400000,count),b.get_data(va-0x400000,count))
        self.assertEqual(a.get_data(0x72630,5),b.get_data(0x72630,5))
    def test_loading_changes_are_three_source_anchors(self):
        self.assertEqual([p['va'] for p in self.report['patches']][1:],
                         ['0x434569','0x4345a3','0x434625'])
        self.assertTrue(self.report['corrected_repaint'])
        self.assertTrue(self.report['includes_loading_changes'])
    def test_normal_loading_widget_path_unchanged(self):
        a=pefile.PE(data=self.base);b=pefile.PE(data=self.out)
        self.assertEqual(a.get_data(0x34700,0x193),b.get_data(0x34700,0x193))
    def test_root_origin_used_for_three_loading_anchors(self):
        code=bytes.fromhex(self.report['stub_bytes'])
        self.assertEqual(code.count(bytes.fromhex('0fbf05e07e5f00')),3)
if __name__=='__main__':unittest.main()
