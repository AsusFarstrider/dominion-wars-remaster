"""Freeze the reviewed reference-free selection child, without building/promoting.

This is an exact intake recipe, not a generic importer for unreviewed experiments.
No legacy catalog, original game data or failure screenshots are copied.
"""
import argparse
import importlib.util
import json
from pathlib import Path, PurePosixPath

from binary_delta import sha
from build_remaster import data_json, put
from build_native_remaster import runtime_additions

BASE_LOCK = 'dded69659e37016882c042eec5b87c5a45a0bc549c6365cbdb37f988a832c0ce'
COMPONENT = '25c752ff9366de7d3eaf65b92a25b0646e95cc75f7f0e7d40d1fbe548cdda971'
INDEX = '364c40e25271b49327999cc560fd16bacf0d3f529a2e6adbec1b8c44a6d7d703'
PARENT = 'aba815d5642ae852f825c770c17af2f14d6ec1b0bfc464dd6f26c0134a2a11b2'


def relative(name):
    p = PurePosixPath(name)
    if (p.is_absolute() or p.as_posix() != name or '\\' in name or ':' in name or
            any(part in ('', '.', '..') for part in name.split('/'))):
        raise ValueError('Unsafe inventory path: ' + name)
    return p


def checked(path, digest, size=None):
    data = path.read_bytes()
    if sha(data) != digest or (size is not None and len(data) != size):
        raise ValueError('Frozen input changed: ' + str(path))
    return data


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--handoff', type=Path, required=True)
    p.add_argument('--previous-intake', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    args = p.parse_args()
    if args.output.exists():
        raise ValueError('Use a new immutable intake directory')
    previous_bytes = checked(args.previous_intake / 'source-lock.json', BASE_LOCK)
    previous = json.loads(previous_bytes)
    index_bytes = checked(args.handoff / 'CHECKSUMS.json', INDEX)
    index = json.loads(index_bytes)['files']
    # Verify everything before creating output. Paths are pinned, but still
    # checked structurally so a future recipe cannot introduce traversal.
    for name, digest in index.items():
        checked(args.handoff / relative(name), digest)
    component = json.loads(checked(args.handoff / 'component.json', COMPONENT))
    source_bytes = checked(args.handoff / 'source.json', component['source_receipt_sha256'])
    source = json.loads(source_bytes)
    if source['parent_receipt_sha256'] != PARENT or len(source['source_files']) != 472:
        raise ValueError('Unexpected component lineage/inventory')
    for item in previous['files']:
        checked(args.previous_intake / relative(item['path']), item['sha256'], item['size'])
    source_root = Path(source['source'])
    for name, digest in source['source_files'].items():
        checked(source_root / relative(name), digest)
    inventory = json.loads(checked(args.handoff / 'runtime-inventory.json', component['runtime_inventory_sha256']))
    if (inventory['source_reference_pixels_in_catalog'] or
            inventory['config']['profile'] != previous['rendering_profile'] or
            inventory['config']['config_delta'] is not None or len(inventory['files']) != 6):
        raise ValueError('Unsupported runtime inventory/config')
    wrapper = [item for item in inventory['files'] if item['target'] == 'ddraw.dll']
    if len(wrapper) != 1 or wrapper[0]['sha256'] != component['wrapper_sha256']:
        raise ValueError('Wrapper identity mismatch')
    for item in inventory['files']:
        checked(args.handoff / relative(item['source']), item['sha256'], item['size'])
    spec = importlib.util.spec_from_file_location('frozen_selection_validator', args.handoff / 'tools/validate_catalog.py')
    validator = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(validator)
    catalog = validator.validate((args.handoff / 'runtime/rs/Remaster/Selection/selection.dwselect').read_bytes())
    if catalog['sha256'] != component['catalog_sha256']:
        raise ValueError('Catalog structure/identity mismatch')

    additions = [dict(input='runtime-assets/' + item['target'], target=item['target'],
                      sha256=item['sha256'], size=item['size'])
                 for item in inventory['files'] if item['target'] != 'ddraw.dll']
    # Validate the same runtime contract the package builder will use.
    runtime_additions(dict(runtime_additions=additions, files=[dict(
        path=item['input'], sha256=item['sha256'], size=item['size']) for item in additions]))
    for item in previous['files']:
        if item['path'] in ('ddraw.dll', 'qfr-source.json') or item['path'].startswith('wrapper-source/'):
            continue  # Reconstructed below; put() deliberately refuses overwrite.
        put(args.output / item['path'], (args.previous_intake / item['path']).read_bytes())
    for name, digest in source['source_files'].items():
        put(args.output / 'wrapper-source' / name, checked(source_root / name, digest))
    put(args.output / 'ddraw.dll', checked(args.handoff / wrapper[0]['source'], component['wrapper_sha256']))
    for item in additions:
        put(args.output / item['input'], checked(args.handoff / ('runtime/' + item['target']), item['sha256']))
    # Retain new reference-free proof/source/tools, not private ancestor rasters.
    for name, digest in index.items():
        if not name.startswith('runtime/'):
            put(args.output / 'selection-handoff' / name, checked(args.handoff / name, digest))
    put(args.output / 'selection-handoff/CHECKSUMS.json', index_bytes)
    manifest = dict(schema='master-component-source-1', base_intake_lock_sha256=BASE_LOCK,
                    component_sha256=COMPONENT, source_receipt_sha256=sha(source_bytes),
                    parent_source_receipt_sha256=PARENT, production_files=source['source_files'],
                    wrapper_sha256=component['wrapper_sha256'], live_test='not performed on this child',
                    inherited_live_scope='selection03 plus paused briefing02, same one computer',
                    component_status='private locked intake; not a promoted or released build')
    manifest_bytes = data_json(manifest)
    put(args.output / 'qfr-source.json', manifest_bytes)
    lock = {k: v for k, v in previous.items() if k != 'files'}
    lock.update(source_manifest_sha256=sha(manifest_bytes), wrapper_sha256=component['wrapper_sha256'],
                previous_intake_sha256=BASE_LOCK, component_sha256=COMPONENT,
                runtime_additions=additions, live_acceptance='offline child; inherited live evidence only',
                selection_runtime_inventory_sha256=component['runtime_inventory_sha256'],
                public_artwork_review='Release must reconcile replacement-art permissions; no publication approval')
    lock['files'] = [dict(path=f.relative_to(args.output).as_posix(), sha256=sha(f.read_bytes()), size=f.stat().st_size)
                     for f in sorted(args.output.rglob('*')) if f.is_file()]
    put(args.output / 'source-lock.json', data_json(lock))
    print(json.dumps(dict(intake=str(args.output), production_files=472, runtime_additions=len(additions),
                          lock_sha256=sha((args.output / 'source-lock.json').read_bytes()),
                          promotion_allowed=False, publication_allowed=False), indent=2))


if __name__ == '__main__':
    main()
