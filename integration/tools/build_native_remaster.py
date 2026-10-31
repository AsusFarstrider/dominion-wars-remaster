"""Build the pinned Native1440/restored-v7 trial. Never install, launch or promote.

Player package reconstructs original audio from verified owner files. Complete
commercial inputs/remuxed movies and the generated EXE remain private.
"""
import argparse
from datetime import date
import importlib.util
import json
from pathlib import Path
import re
import shutil
import subprocess
import sys
import zipfile
import zlib

from binary_delta import bink_frames, make_bink_delta, make_delta, sha
from build_remaster import INTAKE as FOUNDATION, data_json, put, source_identity

ROOT = Path(__file__).resolve().parents[1]
NATIVE = ROOT / 'components/intake-native-20261028.1'
BUILD_ID = 'int-20261027.1'
FLAVOR = 'native1440-restored-v7'
DECISION = 'version-review-20261027-1.10'


def checked_file(path, digest):
    data = path.read_bytes()
    if sha(data) != digest:
        raise ValueError('Input identity mismatch: ' + str(path))
    return data


def rendering_settings(lock):
    profile = lock.get('rendering_profile', 'native-1440-v1')
    identities = {
        'native-1440-v1': ('de3e3ede7a9b4ce65a891201547f36413f3a880e4df3f28fb8a81e51f687abc6', (2560, 1440)),
        'native-display-v1': ('e81aacb185e755c6478c06516bf0d442b7b95edc1f3a3603bfc7ecda1c777023', (800, 600)),
    }
    if profile not in identities:
        raise ValueError('Unsupported frozen rendering capability')
    expected, dimensions = identities[profile]
    if lock['native_engine_sha256'] != expected or tuple(lock.get('engine_dimensions', dimensions)) != dimensions:
        raise ValueError('Profile/engine identity or recipe dimensions disagree')
    rendering = dict(profile=profile, source_manifest_sha256=lock['source_manifest_sha256'], recording='off')
    if profile == 'native-1440-v1':
        rendering.update(width=2560, height=1440)
    return rendering, dimensions


def native_engine(original, expected, dimensions=(2560, 1440)):
    spec = importlib.util.spec_from_file_location('frozen_native_engine', NATIVE / 'build_engine.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    # Only relocate the helper's workspace-root dependency. Its exact frozen
    # recipe and existing input-stub emitter remain unchanged and hash-pinned.
    module.ROOT = ROOT.parent
    engine, report = module.build(original, *dimensions)
    if sha(engine) != expected:
        raise ValueError('Native engine differs from the selected reviewed chain')
    return engine, report


def main():
    global NATIVE, BUILD_ID, FLAVOR, DECISION
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--original-game', type=Path, required=True)
    parser.add_argument('--asset-inputs', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--intake', type=Path, required=True)
    parser.add_argument('--decision', type=Path, required=True)
    args = parser.parse_args()
    if args.output.exists():
        raise ValueError('Output must be new; historical builds are immutable')
    NATIVE = args.intake.resolve()
    if NATIVE.parent != (ROOT / 'components').resolve():
        raise ValueError('Intake must be a frozen component in this source tree')
    decision_path = args.decision.resolve()
    if decision_path.parent != (ROOT.parent / 'versioning/decisions').resolve():
        raise ValueError('Decision must be committed in this source tree')
    decision_bytes = decision_path.read_bytes()
    decision = json.loads(decision_bytes)
    BUILD_ID, FLAVOR, DECISION = (decision[k] for k in ('integration_build_id', 'flavor', 'id'))
    if not re.fullmatch(r'int-[0-9]{8}\.[1-9][0-9]*(?:-[a-z0-9-]+)?', BUILD_ID):
        raise ValueError('Invalid integration build ID')
    version = decision['proposed_remaster_version']
    if (not isinstance(version, str) or not re.fullmatch(r'1\.(?:0[5-9]|[1-9][0-9])', version)
            or decision['release_stage'] != 'alpha' or decision['in_game_label'] != 'Remastered ' + version):
        raise ValueError('Expected an explicit two-digit pre-2.00 Alpha candidate decision')
    digits = BUILD_ID[4:12]
    build_date = date(int(digits[:4]), int(digits[4:6]), int(digits[6:])).isoformat()
    lock = json.loads((NATIVE / 'source-lock.json').read_text())
    for item in lock['files']:
        checked_file(NATIVE / item['path'], item['sha256'])
    base_lock = json.loads((FOUNDATION / 'source-lock.json').read_text())
    for item in base_lock['inputs']:
        checked_file(FOUNDATION / item['path'], item['sha256'])
    originals = json.loads((FOUNDATION / 'original-files.json').read_text())
    for record in originals:
        checked_file(args.original_game / record['path'], record['sha256'])
    for item in lock['assets']:
        checked_file(args.asset_inputs / item['input'], item['sha256'])
    git = shutil.which('git') or 'C:/Program Files/Git/cmd/git.exe'
    if subprocess.check_output([git, '-C', str(ROOT.parent), 'status', '--porcelain', '--', 'versioning']).strip():
        raise ValueError('Commit and freeze versioning inputs first')
    revision, tree_hash, dirty = source_identity(False)
    tree_hash = sha(data_json(dict(integration=tree_hash, version_decision=sha(decision_bytes),
                                  identity_schema=sha((ROOT.parent / 'versioning/identity.schema.json').read_bytes()))))
    rendering, dimensions = rendering_settings(lock)
    rendering_profile = rendering['profile']
    adaptive = rendering_profile == 'native-display-v1'
    engine, composition = native_engine((args.original_game / 'ds9dw.exe').read_bytes(), lock['native_engine_sha256'], dimensions)
    package = args.output / 'package'
    package.mkdir(parents=True)
    put(args.output / 'private/generated-ds9dw.exe', engine)
    patch = make_delta((args.original_game / 'ds9dw.exe').read_bytes(), engine)
    put(package / 'patches/engine.json', data_json(patch))
    for path in sorted((FOUNDATION / 'modernizer').rglob('*')):
        if path.is_file():
            put(package / 'foundation' / path.relative_to(FOUNDATION / 'modernizer'), path.read_bytes())
    runtime = []

    def runtime_put(name, data):
        put(package / 'runtime' / name, data)
        runtime.append(dict(source='runtime/' + name, target=name, size=len(data), sha256=sha(data)))

    for path in sorted((ROOT / 'runtime').iterdir()):
        if path.is_file():
            if path.name == 'Watch-RemasterFocus.ps1' and not adaptive:
                continue  # Legacy builds retain their original generated helper.
            runtime_put(path.name, path.read_bytes())
    for name in ('Launch-NoDesktopSwitch.ps1', 'DominionWarsLauncher.psm1', 'Restore-DominionWarsDesktop.ps1'):
        data = (FOUNDATION / 'launcher' / name).read_bytes()
        if name == 'Launch-NoDesktopSwitch.ps1':
            old = b'requested=${Width}x${Height}; diagnostics=$DiagnoseOnly'
            if data.count(old) != 1:
                raise ValueError('Unexpected launcher diagnostic anchor')
            data = data.replace(old, b'requested=unchanged-desktop; diagnostics=$DiagnoseOnly')
        runtime_put(name, data)
    helper = (FOUNDATION / 'launcher/Watch-NoResizeFocus-v2.ps1').read_text(encoding='utf-8-sig')
    old = "$expected='FB972D8FBABD6DCC929E78F74A8C3C158EA2F8A8A66DBC040291E643A9E91CD7'"
    if helper.count(old) != 1:
        raise ValueError('Unexpected focus helper anchor')
    if not adaptive:
        runtime_put('Watch-RemasterFocus.ps1', helper.replace(old, "$expected='" + sha(engine).upper() + "'").encode())
    runtime_put('ddraw.dll', (NATIVE / 'ddraw.dll').read_bytes())
    runtime_put('DDrawCompat-LICENSE.txt', (NATIVE / 'wrapper-source/LICENSE.txt').read_bytes())
    identity = dict(schema=1, product='Dominion Wars Remastered', original_game_version='1.04',
                    remaster_version=version, release_stage='alpha', integration_build_id=BUILD_ID,
                    source_revision=revision, flavor=FLAVOR, version_decision_id=DECISION,
                    in_game_label=decision['in_game_label'])
    identity_bytes = data_json(identity)
    runtime_put('version.json', identity_bytes)
    config = dict(schema=1, profile=rendering_profile, engine_sha256=sha(engine), version_sha256=sha(identity_bytes),
                  recording='off', paramount_hd=True, gizmo_hd=True, ss_hd=True, publisher_card=True,
                  title_card=True, main_background=True, main_buttons=True)
    runtime_put('Remaster.Rendering.json', data_json(config))
    asset_patches, audio_audit = [], []
    for item in lock['assets']:
        restored = (args.asset_inputs / item['input']).read_bytes()
        if item['kind'] == 'runtime':
            runtime_put(item['target'], restored)
            continue
        original = (args.original_game / item['target']).read_bytes()
        delta = make_bink_delta(original, restored)
        name = 'patches/' + Path(item['target']).stem + '.json'
        put(package / name, data_json(delta))
        asset_patches.append(dict(path=name, target=item['target'], input_sha256=delta['input_sha256'],
                                  output_sha256=delta['output_sha256'], method='BIKi-original-audio-COPY'))
        _, _, frames = bink_frames(original)
        audio_audit.append(dict(target=item['target'], input_sha256=sha(original), output_sha256=sha(restored),
                                frames=len(frames), audio_metadata_copy=[44, 12],
                                audio_packet_copies=[[start, audio_end-start] for start, audio_end, _ in frames],
                                literal_original_audio=False))
    put(package / 'patches/audio-provenance.json', data_json(audio_audit))
    for path in sorted((ROOT / 'installer').iterdir()):
        if path.is_file():
            put(package / path.name, path.read_bytes())
    put(package / 'Remaster.Core.psm1', (ROOT / 'runtime/Remaster.Core.psm1').read_bytes())
    profile = (FOUNDATION / 'wide960-displayfit.ini').read_text()
    profile = '\n'.join(line for line in profile.splitlines() if not line.startswith('#')) + '\n'
    for key, value in (('ResolutionScale', 'app(1)'), ('SupportedResolutions', 'native')):
        profile, count = re.subn(r'(?m)^' + key + r'\s*=.*$', key + ' = ' + value, profile)
        if count != 1:
            raise ValueError('Profile anchor changed: ' + key)
    profiles = []
    for name, text in (('d3d9on12', profile), ('native', profile.replace('ForceD3D9On12 = on', 'ForceD3D9On12 = off'))):
        filename = 'profiles/' + name + '.ini'
        put(package / filename, text.encode())
        profiles.append(dict(name=name, path=filename, sha256=sha(text.encode())))
    for path in sorted((NATIVE / 'wrapper-source').rglob('*')):
        if path.is_file():
            put(package / 'third_party/DDrawCompat-source' / path.relative_to(NATIVE / 'wrapper-source'), path.read_bytes())
    put(package / 'third_party/native-component-source-lock.json', (NATIVE / 'source-lock.json').read_bytes())
    put(package / 'third_party/runtime-contract.md', (NATIVE / 'tests/RUNTIME-CONTRACT.md').read_bytes())
    put(package / 'Read Me First.md', (ROOT / 'docs' / ('ADAPTIVE-ALPHA.md' if adaptive else 'NATIVE-1440-TRIAL.md')).read_bytes())
    put(package / 'version-decision.json', decision_bytes)
    files = [dict(path=p.relative_to(package).as_posix(), size=p.stat().st_size, sha256=sha(p.read_bytes()))
             for p in sorted(package.rglob('*')) if p.is_file()]
    manifest = dict(schema=1, product='Dominion Wars Remaster', build_id=BUILD_ID, build_date=build_date,
                    flavor=FLAVOR, status='integration-candidate', source_revision=revision,
                    source_tree_sha256=tree_hash, uncommitted_source=dirty, remaster_version=version,
                    release_stage='alpha', in_game_label=identity['in_game_label'], version_decision_id=DECISION,
                    version_identity=dict(target='version.json', sha256=sha(identity_bytes)),
                    rendering=dict(rendering, mode_policy=('current physical primary at0,0; one immutable size per launch' if adaptive else 'primary physical2560x1440 at0,0; refuse otherwise')),
                    modernizer_commit=base_lock['modernizer_commit'],
                    intake_lock_sha256=sha((NATIVE / 'source-lock.json').read_bytes()),
                    engine=dict(path='patches/engine.json', input_sha256=patch['input_sha256'], output_sha256=patch['output_sha256']),
                    asset=None, asset_patches=asset_patches, original_files=originals, runtime_files=runtime,
                    profiles=profiles, files=files, wrapper=dict(sha256=lock['wrapper_sha256'], telemetry_default=False, license='BSD-0-Clause'),
                    build_tools=dict(python=sys.version.split()[0], zlib=zlib.ZLIB_VERSION, zlib_runtime=zlib.ZLIB_RUNTIME_VERSION,
                                     player='Windows PowerShell5.1; no Git/Python/compiler/encoder needed'),
                    approvals=dict(internal_baseline='pending user acceptance', public_release='held'),
                    combined_validation=dict(automated='pending managed package tests', gameplay='pending', version_display='pending'),
                    known_limits=['One-system evidence only', ('Actual1080/1440 DPI96 only; other modes unvalidated; primary-origin0 only; no live resize' if adaptive else 'Native1440 pilot only'), 'Multiplayer unsupported',
                                  'v7 banks cache about231MiB; new combined live test pending', 'No universal Windows/GPU claims'])
    put(package / 'manifest.json', data_json(manifest))
    put(package / 'CHECKSUMS.json', data_json({p.relative_to(package).as_posix(): sha(p.read_bytes()) for p in sorted(package.rglob('*')) if p.is_file()}))
    archive_path = args.output / ('dominion-wars-remastered-' + version + '-alpha-' + BUILD_ID + '.zip')
    with zipfile.ZipFile(archive_path, 'x', zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        for path in sorted(package.rglob('*')):
            if path.is_file():
                info = zipfile.ZipInfo(path.relative_to(package).as_posix(), tuple(map(int, build_date.split('-'))) + (0, 0, 0))
                info.compress_type = zipfile.ZIP_DEFLATED
                info.external_attr = 0o644 << 16
                archive.writestr(info, path.read_bytes())
    with zipfile.ZipFile(archive_path) as archive:
        if archive.testzip() is not None:
            raise ValueError('ZIP CRC failure')
        for item in files + [dict(path='manifest.json', sha256=sha((package/'manifest.json').read_bytes()))]:
            if sha(archive.read(item['path'])) != item['sha256']:
                raise ValueError('Archived payload differs: ' + item['path'])
    record = dict(build_id=BUILD_ID, source_revision=revision, composition=composition, package=str(archive_path),
                  package_sha256=sha(archive_path.read_bytes()), package_size=archive_path.stat().st_size,
                  manifest_sha256=sha((package / 'manifest.json').read_bytes()),
                  identity_sha256=sha(identity_bytes), publish_allowed=False, promotion_allowed=False)
    put(args.output / 'build-record.json', data_json(record))
    print(json.dumps(record, indent=2))


if __name__ == '__main__':
    main()
