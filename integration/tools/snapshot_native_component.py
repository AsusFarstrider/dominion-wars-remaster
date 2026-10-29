"""Freeze the identified specialist component into a NEW Master intake.

Never launches, promotes or copies commercial game data into source. Converted
visual inputs/full remuxed movies remain in a separate PRIVATE input directory.
"""
import argparse
import json
from pathlib import Path

from binary_delta import sha

ROOT = Path(__file__).resolve().parents[1]


def read(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def put(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open('xb') as stream:
        stream.write(data)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--asset-output', type=Path, required=True)
    parser.add_argument('--expected-source-sha', required=True)
    parser.add_argument('--expected-wrapper-sha', required=True)
    parser.add_argument('--gizmo-report', type=Path, required=True)
    args = parser.parse_args()
    if args.output.exists() or args.asset_output.exists():
        raise ValueError('Both output directories must be NEW; historical inputs stay untouched')
    if args.asset_output.resolve().is_relative_to(ROOT.parent.resolve()):
        raise ValueError('Private media input cache must stay outside public source')
    source_bytes = (args.source / 'qfr-source.json').read_bytes()
    expected_source = args.expected_source_sha.lower()
    expected_dll = args.expected_wrapper_sha.lower()
    if sha(source_bytes) != expected_source:
        raise ValueError('Unreviewed specialist source identity')
    manifest = json.loads(source_bytes)
    baseline = ROOT / 'components/intake-20261018.1/wrapper-source'
    files = {p.relative_to(baseline).as_posix(): sha(p.read_bytes()) for p in baseline.rglob('*') if p.is_file()}
    if len(files) != 407:
        raise ValueError('Original frozen source inventory changed')
    for change in manifest['changes']:
        if files.get(change['path']) != change['before']:
            raise ValueError('Unexpected baseline-relative before hash: ' + change['path'])
        files[change['path']] = change['after']
    if files != manifest['opening_flow']['source_files']:
        raise ValueError('Incomplete current source inventory')
    for name, expected in files.items():
        if sha((args.source / name).read_bytes()) != expected:
            raise ValueError('Changed frozen source: ' + name)
    if sha((args.source / 'Release/ddraw.dll').read_bytes()) != expected_dll:
        raise ValueError('Changed frozen wrapper')
    tests_bytes = (args.source / 'validation/results.json').read_bytes()
    tests = json.loads(tests_bytes)
    repair = manifest.get('gpu_hook_fix')
    if repair:
        if (sha(tests_bytes) != repair['validation_sha256'] or repair['wrapper_sha256'] != expected_dll
                or not tests['clean_build'] or tests['wrapper_sha256'] != expected_dll):
            raise ValueError('Repair validation/build binding mismatch')
    elif tests.get('source_manifest_sha256') != expected_source:
        raise ValueError('Matching specialist preflight required')
    if not tests['passed']:
        raise ValueError('Specialist preflight failed')
    for result in tests['tests']:
        if not result['passed'] or sha(Path(result['binary']).read_bytes()) != result['binary_sha256']:
            raise ValueError('Missing/changed specialist test evidence: ' + result['name'])
    for name, expected in manifest['production_runtime']['test_sources'].items():
        if sha((args.source / 'production-validation-sources' / name).read_bytes()) != expected:
            raise ValueError('Changed test source: ' + name)
    if repair:
        for name, expected in repair['test_sources'].items():
            if sha((args.source / 'gpu-hook-validation-sources' / name).read_bytes()) != expected:
                raise ValueError('Changed real-visitor test source: ' + name)
    for name in files:
        put(args.output / 'wrapper-source' / name, (args.source / name).read_bytes())
    for name in ('qfr-source.json', 'validation/results.json'):
        put(args.output / name, (args.source / name).read_bytes())
    if repair:
        logs = [p for p in args.source.glob('build-*.log') if sha(p.read_bytes()) == tests['build_log_sha256']]
        if len(logs) != 1:
            raise ValueError('Missing exact current clean build log')
        put(args.output / 'native-build.log', logs[0].read_bytes())
        for path in (args.source / 'gpu-hook-validation-sources').rglob('*'):
            if path.is_file() and '__pycache__' not in path.parts:
                put(args.output / 'tests/gpu-hook' / path.relative_to(args.source / 'gpu-hook-validation-sources'), path.read_bytes())
        for name, expected in repair['contract_snapshots'].items():
            data = (args.source / 'gpu-hook-contract-snapshots' / name).read_bytes()
            if sha(data) != expected:
                raise ValueError('Changed repair contract snapshot')
            put(args.output / 'contracts/gpu-hook' / name, data)
    else:
        put(args.output / 'native-build.log', (args.source / 'build-production-06.log').read_bytes())
    for path in (args.source / 'production-validation-sources').rglob('*'):
        if path.is_file() and '__pycache__' not in path.parts:
            put(args.output / 'tests' / path.relative_to(args.source / 'production-validation-sources'), path.read_bytes())
    put(args.output / 'ddraw.dll', (args.source / 'Release/ddraw.dll').read_bytes())
    recipe = ROOT.parent / 'rendering-research/native-resolution/build_engine.py'
    if sha(recipe.read_bytes()) != 'd0c6b3275e877580d62ae402248544f3022848a69553392d0c1465cf6c5e5576':
        raise ValueError('Native engine recipe changed')
    put(args.output / 'build_engine.py', recipe.read_bytes())
    assets = []

    def asset(source, relative, target, expected, kind):
        data = source.read_bytes()
        if sha(data) != expected:
            raise ValueError('Unreviewed asset: ' + str(source))
        put(args.asset_output / relative, data)
        assets.append(dict(input=relative, target=target, sha256=expected, size=len(data), kind=kind))

    workspace = ROOT.parent
    asset(workspace / 'paramount-restoration/private-1440-v1/Paramount-1440p15-original-audio.bik',
          'movies/Paramount.bik', 'rs/Video/Paramount.bik',
          '5a3506e3f48190073591c1349f4e35dbfe8dde9d2ded19364fa865817deecff3', 'bink-delta')
    gizmo = read(args.gizmo_report)
    gizmo_sha = gizmo['sha256']
    for relative in ('DDrawCompat/Remaster/Runtime.cpp', 'DDrawCompat/Research/OpeningFlow/OpeningFlow.cpp'):
        if gizmo_sha not in (args.source / relative).read_text():
            raise ValueError('Gizmo input is not admitted by both frozen runtime gates')
    asset(args.gizmo_report.parent / gizmo['asset'], 'movies/GizmoLogo.bik',
          'rs/Video/GizmoLogo.bik', gizmo_sha, 'bink-delta')
    ss_report = read(workspace / 'cinematic-extraction/original-ss-hd-v1/asset-report.json')
    ss = workspace / 'cinematic-extraction/original-ss-hd-v1' / ss_report['asset']
    asset(ss, 'movies/SSLogo1.bik', 'rs/Video/SSLogo1.bik',
          manifest['production_runtime']['ss_asset_sha256'], 'bink-delta')
    art_base = args.source.parent
    for card in read(art_base / 'opening-card-assets-v1/assets.json')['assets']:
        asset(Path(card['payload']), 'art/' + card['name'] + '.dwcard',
              'rs/Remaster/OpeningCards/' + card['name'] + '.dwcard', card['payload_sha256'], 'runtime')
    main_art = read(art_base / 'main-background-assets-v1/assets.json')
    asset(Path(main_art['payload']), 'art/main.dwcard', 'rs/Remaster/OpeningCards/main.dwcard',
          main_art['payload_sha256'], 'runtime')
    button_bytes = (art_base / 'main-button-assets-v7/assets.json').read_bytes()
    if sha(button_bytes) != manifest['production_runtime']['buttons_manifest_sha256']:
        raise ValueError('Unreviewed button inventory')
    for bank in json.loads(button_bytes)['banks']:
        asset(Path(bank['path']), 'art/' + bank['name'] + '.dwbank',
              'rs/Remaster/MainButtons/' + bank['name'] + '.dwbank', bank['sha256'], 'runtime')
    lock = dict(schema=1, source_manifest_sha256=expected_source, wrapper_sha256=expected_dll,
                native_engine_sha256='de3e3ede7a9b4ce65a891201547f36413f3a880e4df3f28fb8a81e51f687abc6',
                assets=assets, live_acceptance='pending', version_display='pending package-bound trial',
                restored_visual_authorization='Owner confirmed October28: created by self/ChatGPT and may be released; original game/audio/trademark rights not transferred.',
                files=[dict(path=p.relative_to(args.output).as_posix(), sha256=sha(p.read_bytes()), size=p.stat().st_size)
                       for p in sorted(args.output.rglob('*')) if p.is_file()])
    put(args.output / 'source-lock.json', (json.dumps(lock, indent=2, sort_keys=True) + '\n').encode())
    print(json.dumps(dict(intake=str(args.output), private_asset_inputs=str(args.asset_output),
                          source_files=len(files), assets=len(assets), source_manifest_sha256=expected_source), indent=2))


if __name__ == '__main__':
    main()
