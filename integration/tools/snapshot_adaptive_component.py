"""Freeze the reviewed adaptive component; never launch, promote or copy game media."""
import argparse
import json
from pathlib import Path

from binary_delta import sha
from build_remaster import data_json, put

COMPONENT_SHA = '26244aae31d835569da39cfadff694639d96747679e2fb56b8a1c61d7fe090af'
INDEX_SHA = '562f62ae1eadd8881078be3f6e72f6be364d53c3458d4686ae2eb1ac08486be5'


def checked(path, digest):
    data = path.read_bytes()
    if sha(data) != digest:
        raise ValueError('Frozen input changed: ' + str(path))
    return data


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--handoff', type=Path, required=True)
    p.add_argument('--previous-intake', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    args = p.parse_args()
    if args.output.exists():
        raise ValueError('A new intake directory is required')
    index = json.loads(checked(args.handoff / 'CHECKSUMS.json', INDEX_SHA))
    for name, digest in index.items():
        checked(args.handoff / name, digest)
    c = json.loads(checked(args.handoff / 'component.json', COMPONENT_SHA))
    source = Path(c['frozen_source'])
    manifest = checked(source / 'qfr-source.json', c['frozen_source_manifest_sha256'])
    if len(c['provider_production_pins']) != 467:
        raise ValueError('Unexpected production inventory')
    for name, digest in c['provider_production_pins'].items():
        put(args.output / 'wrapper-source' / name, checked(source / name, digest))
    put(args.output / 'qfr-source.json', manifest)
    put(args.output / 'ddraw.dll', checked(source / 'Release/ddraw.dll', c['wrapper_sha256']))
    # Keep current and inherited evidence distinct. No original EXE or compiled
    # fixture is copied; test source/logs and exact binary hashes are retained.
    suffixes = {'.py', '.cpp', '.h', '.vcxproj', '.json', '.md', '.ps1', '.log', '.txt'}
    for directory in source.iterdir():
        if directory.is_dir() and (directory.name.endswith('-validation-sources') or
                                   directory.name.endswith('-contract-snapshots') or
                                   directory.name == 'adaptive-validation'):
            for path in sorted(directory.rglob('*')):
                if path.is_file() and path.suffix.lower() in suffixes and '__pycache__' not in path.parts:
                    put(args.output / 'evidence' / directory.name / path.relative_to(directory), path.read_bytes())
    validation = json.loads((source / 'adaptive-validation/results.json').read_bytes())
    scope = json.loads(manifest)['adaptive_output']
    checked(source / 'adaptive-validation/results.json', scope['validation_sha256'])
    if validation['wrapper_sha256'] != c['wrapper_sha256'] or any(r['exit_code'] for r in validation['results']):
        raise ValueError('Adaptive validation/build receipt failed')
    for result in validation['results']:
        checked(source / 'adaptive-validation' / (result['name'] + '.log'), result['log_sha256'])
    for mode, digest in c['live_acceptance'].items():
        acceptance = json.loads(checked(args.handoff / 'evidence' / mode / 'acceptance.json', digest))
        if (acceptance['normal_process_exit'] != 0 or acceptance['normal_managed_launcher_exit'] != 0 or
                not acceptance['managed_files_verified'] or acceptance['fingerprint'] != c['same_fingerprint_both_live_runs']):
            raise ValueError('Two-mode acceptance/fingerprint mismatch')
    for name, digest in index.items():
        put(args.output / 'handoff' / name, checked(args.handoff / name, digest))
    put(args.output / 'handoff/CHECKSUMS.json', (args.handoff / 'CHECKSUMS.json').read_bytes())
    for name, digest in c['engine_reproduction_sources'].items():
        data = checked(args.handoff / 'engine-build-provenance' / name, digest)
        if name.endswith('/build_engine.py'):
            put(args.output / 'build_engine.py', data)
    previous_bytes = (args.previous_intake / 'source-lock.json').read_bytes()
    previous = json.loads(previous_bytes)
    # Asset versions are deliberately unchanged, including original-audio deltas.
    if len(previous['assets']) != 12:
        raise ValueError('Unexpected approved asset inventory')
    put(args.output / 'tests/RUNTIME-CONTRACT.md', (args.previous_intake / 'tests/RUNTIME-CONTRACT.md').read_bytes())
    lock = dict(schema=1, source_manifest_sha256=c['frozen_source_manifest_sha256'],
                wrapper_sha256=c['wrapper_sha256'], native_engine_sha256=c['engine_sha256'],
                rendering_profile='native-display-v1', engine_dimensions=[800, 600],
                assets=previous['assets'], previous_intake_sha256=sha(previous_bytes),
                component_sha256=COMPONENT_SHA, live_acceptance='two modes; one computer; external pinned receipts',
                restored_visual_authorization=previous['restored_visual_authorization'],
                files=[dict(path=f.relative_to(args.output).as_posix(), sha256=sha(f.read_bytes()), size=f.stat().st_size)
                       for f in sorted(args.output.rglob('*')) if f.is_file()])
    put(args.output / 'source-lock.json', data_json(lock))
    print(json.dumps(dict(intake=str(args.output), production_files=467, assets=12,
                          source_manifest_sha256=lock['source_manifest_sha256']), indent=2))


if __name__ == '__main__':
    main()
