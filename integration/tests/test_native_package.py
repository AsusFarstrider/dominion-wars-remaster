import json
from pathlib import Path
import struct
import sys
import unittest

TOOLS = Path(__file__).resolve().parents[1] / 'tools'
sys.path.insert(0, str(TOOLS))
from binary_delta import apply_delta, bink_frames, make_bink_delta


def movie(videos, audio=(b'originalaudio1!!', b'originalaudio2!!'), width=800):
    frames = [struct.pack('<I', len(a)) + a + v for a, v in zip(audio, videos)]
    assert all(len(f) % 2 == 0 for f in frames)
    cursor = 56 + 4 * (len(frames) + 1)
    offsets = []
    for frame in frames:
        offsets.append(cursor)
        cursor += len(frame)
    offsets.append(cursor)
    header = bytearray(56)
    struct.pack_into('<4s10I', header, 0, b'BIKi', cursor - 8, len(frames), max(map(len, frames)),
                     0, width, 600, 15, 1, 0, 1)
    header[44:56] = b'audiometadat'
    return bytes(header) + struct.pack('<' + str(len(offsets)) + 'I', *offsets) + b''.join(frames)


class MovieDeltaTests(unittest.TestCase):
    def test_original_audio_is_source_copy_not_literal(self):
        old = movie((b'oldvideo', b'oldvideo'))
        new = movie((b'new HD video', b'new HD video'), width=2560)
        delta = make_bink_delta(old, new)
        self.assertEqual(apply_delta(old, delta), new)
        _, _, frames = bink_frames(old)
        copies = [(op['offset'], op['length']) for op in delta['operations'] if op['kind'] == 'copy']
        for start, audio_end, _ in frames:
            self.assertIn((start, audio_end - start), copies)
        self.assertTrue(any(start <= 44 and start + length >= 56 for start, length in copies))

    def test_changed_audio_refused(self):
        old = movie((b'oldvideo', b'oldvideo'))
        new = movie((b'newvideo', b'newvideo'), audio=(b'changed audio!', b'originalaudio2!!'))
        with self.assertRaises(ValueError):
            make_bink_delta(old, new)

    def test_changed_metadata_timing_and_bad_extent_refused(self):
        old = movie((b'oldvideo', b'oldvideo'))
        for offset in (44, 28, 56):
            new = bytearray(old)
            new[offset] ^= 2
            with self.assertRaises(ValueError):
                make_bink_delta(old, bytes(new))

    def test_wrong_magic_and_truncation_refused(self):
        old = movie((b'oldvideo', b'oldvideo'))
        for bad in (b'bad', old[:-1], b'BIKf' + old[4:]):
            with self.assertRaises(ValueError):
                bink_frames(bad)


if __name__ == '__main__':
    unittest.main()
