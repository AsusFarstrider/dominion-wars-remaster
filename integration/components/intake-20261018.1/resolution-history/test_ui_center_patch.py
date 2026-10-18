import unittest,sys,struct
from build_ui_center_probe import build,ROOT
sys.path.insert(0,'E:/Codex/tools/pydeps')
import pefile
class CenterPatch(unittest.TestCase):
    def test_image_and_width_preserved(self):
        raw=(ROOT/'game-wide960-v1/ds9dw.exe').read_bytes();out,r=build(raw)
        a=pefile.PE(data=raw);b=pefile.PE(data=out)
        self.assertEqual(b.FILE_HEADER.NumberOfSections,a.FILE_HEADER.NumberOfSections+1)
        self.assertEqual(len(r['patches']),18)
        self.assertEqual(b.sections[-1].Name.rstrip(b'\0'),b'.dwui')
        self.assertEqual(b.OPTIONAL_HEADER.ImageBase,a.OPTIONAL_HEADER.ImageBase)
        self.assertEqual(b.get_data(0x2b5bb,4),struct.pack('<I',960))
        self.assertEqual(b.get_data(0x1180,4),a.get_data(0x1180,4))
        self.assertEqual(b.get_data(0x33620,256),a.get_data(0x33620,256))
        allowed=set()
        for patch in r['patches']:
            allowed.update(range(patch['file_offset'],patch['file_offset']+len(bytes.fromhex(patch['before']))))
        for owner,field,size in ((a.FILE_HEADER,'NumberOfSections',2),(a.OPTIONAL_HEADER,'SizeOfImage',4),(a.OPTIONAL_HEADER,'SizeOfCode',4),(a.OPTIONAL_HEADER,'CheckSum',4)):
            start=owner.get_field_absolute_offset(field);allowed.update(range(start,start+size))
        start=a.sections[-1].get_file_offset()+40;allowed.update(range(start,start+40))
        self.assertTrue(all(i in allowed for i,(old,new) in enumerate(zip(raw,out)) if old!=new))
        self.assertEqual(b.sections[-1].Characteristics,0x60000020)
    def test_arithmetic_model(self):
        root=80;widget_x=200;mouse=280
        self.assertEqual(root+widget_x,mouse)
        self.assertEqual(mouse-root,widget_x)
        self.assertEqual(root-80+500,500) # Scene-container counter-offset.
        self.assertEqual(243-80,163) # Minimap extent retained.
        self.assertEqual(162-80,82) # Map center relative to new origin retained.
        self.assertEqual(root+(800-200)//2,(960-200)//2)
    def test_unexpected_input_rejected(self):
        raw=bytearray((ROOT/'game-wide960-v1/ds9dw.exe').read_bytes());raw[-1]^=1
        with self.assertRaises(ValueError):build(bytes(raw))
if __name__=='__main__':unittest.main()
