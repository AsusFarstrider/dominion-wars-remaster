import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from build_native_remaster import rendering_settings


class AdaptivePackageTests(unittest.TestCase):
    def adaptive(self):
        return dict(rendering_profile='native-display-v1', source_manifest_sha256='a' * 64,
                    native_engine_sha256='e81aacb185e755c6478c06516bf0d442b7b95edc1f3a3603bfc7ecda1c777023',
                    engine_dimensions=[800, 600])

    def test_adaptive_has_no_fixed_dimensions(self):
        rendering, recipe = rendering_settings(self.adaptive())
        self.assertNotIn('width', rendering)
        self.assertNotIn('height', rendering)
        self.assertEqual(recipe, (800, 600))
        self.assertEqual(rendering['recording'], 'off')

    def test_fixed_legacy_remains_explicit(self):
        rendering, recipe = rendering_settings(dict(source_manifest_sha256='a' * 64,
            native_engine_sha256='de3e3ede7a9b4ce65a891201547f36413f3a880e4df3f28fb8a81e51f687abc6'))
        self.assertEqual((rendering['width'], rendering['height']), (2560, 1440))
        self.assertEqual(recipe, (2560, 1440))

    def test_wrong_engine_dimensions_and_profile_refused(self):
        for field, bad in [('native_engine_sha256', 'b' * 64), ('engine_dimensions', [1920, 1080]),
                           ('rendering_profile', 'native-display'), ('rendering_profile', 'Native-Display-v1')]:
            lock = self.adaptive()
            lock[field] = bad
            with self.assertRaises(ValueError):
                rendering_settings(lock)


if __name__ == '__main__':
    unittest.main()
