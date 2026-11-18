import hashlib
import struct
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from snapshot_campaign_component import verify_bank


class FleetBankTests(unittest.TestCase):
    def test_only_palette_storage_changes(self):
        counts = struct.pack('<II', 139, 2)
        tables = [(struct.pack('<I', key), bytes([i]) * 512)
                  for i, key in enumerate((0xB200, 0xB272), 1)]
        payload = b'replacement data'
        old = b'DWFLEET1' + counts + b''.join(key + ramp for key, ramp in tables) + payload
        new = b'DWFLEET2' + counts + b''.join(key + hashlib.sha256(ramp).digest()
                                             for key, ramp in tables) + payload
        verify_bank(old, new)
        for invalid in (old, new[:-1], new[:20] + b'\0' * 32 + new[52:]):
            with self.assertRaises(ValueError):
                verify_bank(old, invalid)
        with self.assertRaisesRegex(ValueError, 'literal ramp'):
            verify_bank(old + tables[0][1], new + tables[0][1])
