"""Freeze the reviewed 1.20 campaign UI input, not the private test install."""
import argparse
import json
import struct
from pathlib import Path

from binary_delta import sha
from build_native_remaster import runtime_additions
from build_remaster import data_json, put
from snapshot_selection_component import checked, relative

SELECTION = '6232db997f18194f040740eea10da0c01b18d8926b0460d1e9554436ca872ac2'
INDEX = '8470b9124218c885df61ec3504ed8acb93d848cb34f70437d37423d0c062ebe0'
PARENT_SOURCE = 'a0428d6bc8037f83adae33676ed46d4c38fa54e1faef31a81e1cf8a6506b8883'
PARENT_RUNTIME = '58c51d506c37c0cfb9620905506aa6fdd8dd766da64b2bbd90827080dfa14974'
LEGACY = '08ae3f6f29bd31994d81c323cc8251429c32e25eb6a2613a880cd438145a495a'
BRIEFING = 'rs/Remaster/Briefing/briefing.dwbrief'
FLEET = 'rs/Remaster/Fleet/fleet.dwbank'
OVERLAY = {'DDrawCompat/Remaster/FleetArtwork.h', 'DDrawCompat/Remaster/FleetPins.h'}


def verify_bank(old, new):
    """Independently check the palette-only format change and full payload."""
    if (old[:16] != b'DWFLEET1' + struct.pack('<II', 139, 2) or
            new[:16] != b'DWFLEET2' + old[8:16] or new[88:] != old[1048:]):
        raise ValueError('Unexpected Fleet headers or changed replacement payload')
    keys = set()
    for i in range(2):
        op, np = 16 + i * 516, 16 + i * 36
        key = old[op:op + 4]
        keys.add(int.from_bytes(key, 'little'))
        ramp = old[op + 4:op + 516]
        if new[np:np + 36] != key + bytes.fromhex(sha(ramp)) or ramp in new:
            raise ValueError('Fleet palette identity mismatch or literal ramp present')
    if keys != {0xB200, 0xB272}:
        raise ValueError('Unexpected Fleet palette keys')


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for name in ('handoff', 'selection', 'fleet-parent', 'briefing-handoff', 'output'):
        p.add_argument('--' + name, type=Path, required=True)
    a = p.parse_args()
    if a.output.exists():
        raise ValueError('Use a new immutable intake directory')
    previous = json.loads(checked(a.selection / 'source-lock.json', SELECTION))
    for row in previous['files']:
        checked(a.selection / relative(row['path']), row['sha256'], row['size'])
    index = json.loads(checked(a.handoff / 'checksums.json', INDEX))
    if len(index) != 31:
        raise ValueError('Unexpected Fleet handoff inventory')
    for name, digest in index.items():
        checked(a.handoff / relative(name), digest)
    source = json.loads(checked(a.handoff / 'evidence/source.json', index['evidence/source.json']))
    candidate = json.loads(checked(a.handoff / 'evidence/candidate.json', index['evidence/candidate.json']))
    parent = json.loads(checked(a.fleet_parent / 'source.json', PARENT_SOURCE))
    parent_runtime = json.loads(checked(a.fleet_parent / 'candidate.json', PARENT_RUNTIME))
    pins = source['source_files']
    previous_pins = parent['source_files']
    if (source['parent_source_sha256'] != PARENT_SOURCE or
            source['parent_candidate_sha256'] != PARENT_RUNTIME or
            candidate['parent_candidate_sha256'] != PARENT_RUNTIME or
            candidate['source_receipt_sha256'] != index['evidence/source.json'] or
            len(pins) != 478 or pins.keys() != previous_pins.keys() or
            {name for name in pins if pins[name] != previous_pins[name]} != OVERLAY):
        raise ValueError('Unexpected Fleet child source lineage')
    expected_diff = {name: {'after': pins[name], 'before': previous_pins[name]} for name in OVERLAY}
    if source['changes'] != expected_diff:
        raise ValueError('Unexpected Fleet source overlay')
    for name in OVERLAY:
        checked(a.handoff / 'source-overlay' / name, pins[name])
    source_root = Path(source['source'])
    for name, digest in pins.items():
        checked(source_root / relative(name), digest)

    runtime = candidate['runtime_files']
    parent_files = parent_runtime['runtime_files']
    if (len(runtime) != 28 or runtime.keys() != parent_files.keys() or
            {name for name in runtime if runtime[name] != parent_files[name]} != {'ddraw.dll', FLEET} or
            candidate['changes'] != {name: runtime[name] for name in ('ddraw.dll', FLEET)}):
        raise ValueError('Unexpected Fleet child runtime lineage')
    for row in previous['runtime_additions']:
        if runtime[row['target']] != row['sha256']:
            raise ValueError('Inherited Selection assets do not match')
    for row in previous['assets']:
        if row['kind'] == 'runtime' and runtime[row['target']] != row['sha256']:
            raise ValueError('Inherited opening assets do not match')
    wrapper = checked(a.handoff / 'runtime/ddraw.dll', runtime['ddraw.dll'])
    new = checked(a.handoff / 'runtime' / FLEET, runtime[FLEET])
    old = checked(Path(parent_runtime['game']) / FLEET, LEGACY)
    verify_bank(old, new)
    briefing = checked(a.briefing_handoff / 'runtime' / BRIEFING, runtime[BRIEFING])
    additions = [*previous['runtime_additions'], *(
        dict(input='runtime-assets/' + target, target=target, sha256=runtime[target], size=len(data))
        for target, data in ((BRIEFING, briefing), (FLEET, new)))]
    runtime_additions(dict(runtime_additions=additions, files=[dict(
        path=row['input'], sha256=row['sha256'], size=row['size']) for row in additions]))

    for row in previous['files']:
        name = row['path']
        if name not in ('ddraw.dll', 'qfr-source.json') and not name.startswith('wrapper-source/'):
            put(a.output / name, checked(a.selection / name, row['sha256'], row['size']))
    for name, digest in pins.items():
        put(a.output / 'wrapper-source' / name, checked(source_root / name, digest))
    put(a.output / 'ddraw.dll', wrapper)
    put(a.output / 'runtime-assets' / FLEET, new)
    put(a.output / 'runtime-assets' / BRIEFING, briefing)
    for name in ('reproduction/convert_bank.py', 'evidence/conversion.json', 'evidence/test-result-02.txt'):
        put(a.output / 'fleet-validation' / Path(name).name, checked(a.handoff / name, index[name]))
    manifest = dict(schema='master-component-source-1', production_files=pins,
                    source_receipt_sha256=index['evidence/source.json'],
                    candidate_receipt_sha256=index['evidence/candidate.json'],
                    parent_source_receipt_sha256=PARENT_SOURCE, parent_candidate_receipt_sha256=PARENT_RUNTIME,
                    handoff_index_sha256=INDEX, wrapper_sha256=runtime['ddraw.dll'],
                    live_test='previous Fleet05 wrapper at 1440p; exact palette child offline only')
    manifest_bytes = data_json(manifest)
    put(a.output / 'qfr-source.json', manifest_bytes)
    lock = {k: v for k, v in previous.items() if k not in ('files', 'component_sha256')}
    lock.update(source_manifest_sha256=sha(manifest_bytes), wrapper_sha256=runtime['ddraw.dll'],
                previous_intake_sha256=SELECTION, fleet_handoff_index_sha256=INDEX,
                live_acceptance='private parent coverage only; packaged gameplay pending',
                runtime_additions=additions, player_readme='CAMPAIGN-ALPHA.md')
    lock['files'] = [dict(path=f.relative_to(a.output).as_posix(), sha256=sha(f.read_bytes()), size=f.stat().st_size)
                     for f in sorted(a.output.rglob('*')) if f.is_file()]
    put(a.output / 'source-lock.json', data_json(lock))
    print(json.dumps(dict(intake=str(a.output), source_pins=len(pins), runtime_additions=len(additions),
                          lock_sha256=sha((a.output / 'source-lock.json').read_bytes())), indent=2))


if __name__ == '__main__':
    main()
