import importlib.util
import json
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from binary_delta import sha
from build_native_remaster import runtime_additions
from snapshot_selection_component import relative


class SelectionIntakeTests(unittest.TestCase):
    def test_relative_inventory_paths(self):
        self.assertEqual(str(relative('source/file.h')), 'source/file.h')
        for name in ('../x', '/x', 'a//b', './x', 'a\\b', 'C:/x'):
            with self.subTest(name=name), self.assertRaises(ValueError):
                relative(name)

    def test_frozen_catalog_and_runtime_inventory(self):
        intake = ROOT / 'components/intake-selection-20261102.1'
        lock = json.loads((intake / 'source-lock.json').read_bytes())
        additions = runtime_additions(lock)
        self.assertEqual(len(additions), 5)
        for item in additions:
            data = (intake / item['input']).read_bytes()
            self.assertEqual((sha(data), len(data)), (item['sha256'], item['size']))
        spec = importlib.util.spec_from_file_location('selection_format_test',
            intake / 'selection-handoff/tools/validate_catalog.py')
        validator = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(validator)
        data = (intake / 'runtime-assets/rs/Remaster/Selection/selection.dwselect').read_bytes()
        report = validator.validate(data)
        self.assertEqual(report['record_count'], 114)
        self.assertFalse(report['contains_original_identity_pixel_field'])
        for malformed in (b'', b'DWSSEL1\0' + data[8:], data[:-1], data + b'x'):
            with self.assertRaises(ValueError):
                validator.validate(malformed)


if __name__ == '__main__':
    unittest.main()
