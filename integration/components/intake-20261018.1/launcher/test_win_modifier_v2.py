import hashlib,struct,sys,unittest
from pathlib import Path
sys.path.insert(0,'E:/Codex/tools/pydeps')
import pefile

class WinModifierTests(unittest.TestCase):
 def test_original_handler_maps_both_win_keys_to_shared_bit(self):
  raw=Path('E:/Codex/research/dominion-resolution/game-test/ds9dw.exe').read_bytes()
  self.assertEqual(hashlib.sha256(raw).hexdigest(),'03c20a3535d0209111ff9f3b90eba403e059df2be7e784d2746abf1535c1be6a')
  p=pefile.PE(data=raw)
  for key in (0x5b,0x5c):
   self.assertEqual(p.get_data(0x3b660+key-0x10,1),b'\x03')
   self.assertEqual(p.get_data(0x3b6c8+key-0x10,1),b'\x03')
  self.assertEqual(struct.unpack('<I',p.get_data(0x3b648+3*4,4))[0],0x43b5d3)
  self.assertEqual(struct.unpack('<I',p.get_data(0x3b6b0+3*4,4))[0],0x43b61d)
 def test_key_release_message_bits(self):
  self.assertEqual((-1051000831)&0xffffffff,0xc15b0001)
  self.assertEqual((-1050935295)&0xffffffff,0xc15c0001)
 def test_helper_is_scoped_and_read_only(self):
  s=Path(__file__).with_name('Watch-NoResizeFocus-v2.ps1').read_text()
  self.assertIn('OpenProcess(0x1010',s)
  self.assertIn('if(-not ($mask -band 0x80)){return}',s)
  self.assertNotIn('WriteProcessMemory',s)
  self.assertNotIn('SendInput(',s)
  self.assertNotIn('ChangeDisplaySettings',s)

if __name__=='__main__':unittest.main()
