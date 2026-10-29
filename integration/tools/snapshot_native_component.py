"""Freeze the identified specialist component into a NEW Master intake.

Never launches, promotes or copies commercial game data into source. Converted
visual inputs/full remuxed movies remain in a separate PRIVATE input directory.
"""
import argparse
import json
from pathlib import Path
import shutil

from binary_delta import sha

ROOT = Path(__file__).resolve().parents[1]
EXPECTED_SOURCE = '4812ccb9c32034f1d5129b7525a0e5c57010927761ca01f23124657233b9e23e'
EXPECTED_DLL = '85812305fcdfdfcb64d84dcb150fd7be0d399d909a224d228626d5885e391c94'


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
    args = parser.parse_args()
    if args.output.exists() or args.asset_output.exists():
        raise ValueError('Both output directories must be NEW; historical inputs stay untouched')
    if args.asset_output.resolve().is_relative_to(ROOT.parent.resolve()):
        raise ValueError('Private media input cache must stay outside public source')
    source_bytes = (args.source / 'qfr-source.json').read_bytes()
    if sha(source_bytes) != EXPECTED_SOURCE:
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
    if len(files) != 464 or files != manifest['opening_flow']['source_files']:
        raise ValueError('Incomplete current source inventory')
    for name, expected in files.items():
        if sha((args.source / name).read_bytes()) != expected:
            raise ValueError('Changed frozen source: ' + name)
    if sha((args.source / 'Release/ddraw.dll').read_bytes()) != EXPECTED_DLL:
        raise ValueError('Changed frozen wrapper')
    tests = read(args.source / 'validation/results.json')
    if not tests['passed'] or tests['source_manifest_sha256'] != EXPECTED_SOURCE:
        raise ValueError('Matching specialist preflight required')
    for result in tests['tests']:
        if not result['passed'] or sha(Path(result['binary']).read_bytes()) != result['binary_sha256']:
            raise ValueError('Missing/changed specialist test evidence: ' + result['name'])
    for name, expected in manifest['production_runtime']['test_sources'].items():
        if sha((args.source / 'production-validation-sources' / name).read_bytes()) != expected:
            raise ValueError('Changed test source: ' + name)
    for name in files:
        put(args.output / 'wrapper-source' / name, (args.source / name).read_bytes())
    for name in ('qfr-source.json', 'validation/results.json', 'build-production-06.log'):
        put(args.output / name, (args.source / name).read_bytes())
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
    asset(workspace / 'cinematic-extraction/original-gizmo-hd-v1/GizmoLogo-1440p-original-audio.bik',
          'movies/GizmoLogo.bik', 'rs/Video/GizmoLogo.bik',
          'c65bf6cb39b6838ab76f059152790bd0bf437d513c93b4c7b2bdd81d700dc851', 'bink-delta')
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
    lock = dict(schema=1, source_manifest_sha256=EXPECTED_SOURCE, wrapper_sha256=EXPECTED_DLL,
                native_engine_sha256='de3e3ede7a9b4ce65a891201547f36413f3a880e4df3f28fb8a81e51f687abc6',
                assets=assets, live_acceptance='pending', version_display='pending package-bound trial',
                restored_visual_authorization='Owner confirmed October28: created by self/ChatGPT and may be released; original game/audio/trademark rights not transferred.',
                files=[dict(path=p.relative_to(args.output).as_posix(), sha256=sha(p.read_bytes()), size=p.stat().st_size)
                       for p in sorted(args.output.rglob('*')) if p.is_file()])
    put(args.output / 'source-lock.json', (json.dumps(lock, indent=2, sort_keys=True) + '\n').encode())
    print(json.dumps(dict(intake=str(args.output), private_asset_inputs=str(args.asset_output),
                          source_files=len(files), assets=len(assets), source_manifest_sha256=EXPECTED_SOURCE), indent=2))


if __name__ == '__main__':
    main()
