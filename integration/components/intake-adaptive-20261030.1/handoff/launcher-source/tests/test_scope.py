from pathlib import Path
import hashlib,unittest
ROOT=Path(__file__).resolve().parents[1]
class ScopeTests(unittest.TestCase):
 def test_upstream_pins(self):
  expected={'Launch-Remaster.ps1':'46617a9b2d976e0c5bef1eba3f20b8b2c77e8bd927bf5a62b94a7fe71ebe3592',
            'Remaster.Core.psm1':'6575db2a607b97a62fec1a7a79c6f796f4b70b1baf8ee44565154f8c1c00980c',
            'Watch-RemasterFocus.ps1':'fd3f76945d7b731f881fdfff25bc4e641b932110ac59fbd157e58024d618e565'}
  for name,pin in expected.items():self.assertEqual(hashlib.sha256((ROOT/'upstream'/name).read_bytes()).hexdigest(),pin)
 def test_outer_launcher_unchanged(self):
  self.assertEqual((ROOT/'upstream/Launch-Remaster.ps1').read_bytes(),(ROOT/'runtime/Launch-Remaster.ps1').read_bytes())
 def test_helper_only_preflight_changed(self):
  a=(ROOT/'upstream/Watch-RemasterFocus.ps1').read_text();b=(ROOT/'runtime/Watch-RemasterFocus.ps1').read_text()
  self.assertEqual(a[a.index('if((Get-FileHash'):],b[b.index('if((Get-FileHash'):])
  self.assertEqual(b.count('$expected='),2)
 def test_core_only_identity_selector_added(self):
  a=(ROOT/'upstream/Remaster.Core.psm1').read_text();b=(ROOT/'runtime/Remaster.Core.psm1').read_text()
  start=b.index('function Get-RemasterFocusExpectedHash(');end=b.index('function Assert-RemasterOutput(',start)
  self.assertEqual(a,b[:start]+b[end:])
 def test_no_global_or_display_mutation_added(self):
  b=(ROOT/'runtime/Watch-RemasterFocus.ps1').read_text();prefix=b[:b.index('Add-Type')]
  for text in ('WriteProcessMemory','SendInput','ChangeDisplaySettings','Start-Process','SetEnvironmentVariable'):
   self.assertNotIn(text,prefix)

if __name__=='__main__':unittest.main()
