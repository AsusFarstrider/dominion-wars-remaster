import copy
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from build_native_remaster import runtime_additions


class RuntimeAdditionTests(unittest.TestCase):
    def lock(self):
        target = 'rs/Remaster/Selection/selection.flags'
        item = dict(input='runtime-assets/' + target, target=target,
                    sha256='a' * 64, size=1)
        return dict(runtime_additions=[item], files=[dict(
            path=item['input'], sha256=item['sha256'], size=item['size'])])

    def test_legacy_inventory_unchanged(self):
        self.assertEqual(runtime_additions(dict(files=[])), [])

    def test_pinned_data_allowed(self):
        lock = self.lock()
        self.assertEqual(runtime_additions(lock), lock['runtime_additions'])

    def test_unsafe_targets_rejected(self):
        for target in ('../ds9dw.exe', '/rs/Remaster/x/y', 'C:/rs/Remaster/a',
                       'rs\\Remaster\\x', 'rs/Remaster/../x',
                       'rs/Remaster/./x', 'rs/Remaster//x',
                       'rs/Remaster/x:stream', 'rs/Remaster/', 'ddraw.dll',
                       'rs/Remaster/Selection/NUL.txt',
                       'rs/Remaster/Selection/item.', None):
            with self.subTest(target=target):
                lock = self.lock()
                lock['runtime_additions'][0]['target'] = target
                with self.assertRaises(ValueError):
                    runtime_additions(lock)

    def test_case_insensitive_collisions_rejected(self):
        lock = self.lock()
        item = copy.deepcopy(lock['runtime_additions'][0])
        item['target'] = item['target'].upper()
        lock['runtime_additions'].append(item)
        with self.assertRaises(ValueError):
            runtime_additions(lock)

    def test_missing_or_inconsistent_inventory_rejected(self):
        for field, value in [('input', '../outside'), ('size', 2),
                             ('sha256', 'b' * 64), ('sha256', 'bad')]:
            lock = self.lock()
            lock['runtime_additions'][0][field] = value
            with self.subTest(field=field, value=value), self.assertRaises(ValueError):
                runtime_additions(lock)
        lock = self.lock()
        lock['files'] = []
        with self.assertRaises(ValueError):
            runtime_additions(lock)


if __name__ == '__main__':
    unittest.main()
