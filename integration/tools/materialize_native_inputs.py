"""Recover PRIVATE developer visual inputs from a verified package and owned media.

This makes the native build reproducible without distributing full BIK files.
Never writes into the original game or player package.
"""
import argparse
import json
from pathlib import Path

from binary_delta import apply_delta, sha
from build_remaster import put

ROOT = Path(__file__).resolve().parents[1]


def child(root, relative):
    if not isinstance(relative, str) or ':' in relative or '\\' in relative:
        raise ValueError('Unsafe package relative path')
    path = (root / relative).resolve()
    if not path.is_relative_to(root.resolve()):
        raise ValueError('Package path escapes root')
    return path


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--package', type=Path, required=True)
    p.add_argument('--original-game', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    a = p.parse_args()
    if a.output.exists() or a.output.resolve().is_relative_to(ROOT.parent.resolve()):
        raise ValueError('Use a new private output outside source')
    manifest = json.loads((a.package / 'manifest.json').read_text())
    lock = json.loads((ROOT / 'components/intake-native-20261028.1/source-lock.json').read_text())
    if manifest['intake_lock_sha256'] != sha((ROOT / 'components/intake-native-20261028.1/source-lock.json').read_bytes()):
        raise ValueError('Package belongs to another source intake')
    for item in manifest['files']:
        if sha(child(a.package, item['path']).read_bytes()) != item['sha256']:
            raise ValueError('Changed package payload: ' + item['path'])
    for item in lock['assets']:
        if item['kind'] == 'bink-delta':
            patch = next(x for x in manifest['asset_patches'] if x['target'] == item['target'])
            data = apply_delta(child(a.original_game, item['target']).read_bytes(),
                               json.loads(child(a.package, patch['path']).read_text()))
        else:
            runtime = next(x for x in manifest['runtime_files'] if x['target'] == item['target'])
            data = child(a.package, runtime['source']).read_bytes()
        if sha(data) != item['sha256'] or len(data) != item['size']:
            raise ValueError('Reconstructed asset mismatch')
        put(child(a.output, item['input']), data)
    print('Reconstructed all12 hash-pinned PRIVATE inputs; original media/package unchanged.')


if __name__ == '__main__':
    main()
