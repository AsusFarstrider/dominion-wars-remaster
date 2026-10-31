"""Freeze the recorder-only follow-up without changing the passed game copy."""
import hashlib
import json
from pathlib import Path
import shutil
import subprocess

BASE = Path('E:/Codex/research/dominion-rendering')
PARENT = BASE/'opening-flow-source-v5'
WORK = BASE/'opening-recorder-work-01'
OUT = BASE/'opening-recorder-source-v1'
TESTS = BASE/'opening-recorder-tests-01'
HERE = Path(__file__).resolve().parent

def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def load(p): return json.loads(p.read_text(encoding='utf-8-sig'))

if __name__ == '__main__':
    if OUT.exists(): raise ValueError('Use a new frozen root')
    parent_hash = sha(PARENT/'qfr-source.json')
    assert parent_hash == '243c4d020449d53ad998f6f1e93d6c0896e904d959ec5d3a65fbee74a0bcbd08'
    m = load(PARENT/'qfr-source.json')
    pins = m['opening_flow']['source_files']
    for rel, digest in pins.items():
        assert sha(PARENT/rel) == digest, rel
    changed = sorted(rel for rel, digest in pins.items() if sha(WORK/rel) != digest)
    assert changed == [
        'DDrawCompat/Research/FlightRecorder/Queue.h',
        'DDrawCompat/Research/FlightRecorder/Recorder.cpp',
        'DDrawCompat/Research/MenuProbe/Probe.cpp',
        'DDrawCompat/Research/NativeProbe/Probe.cpp'], changed
    log = (WORK/'build.log').read_text(encoding='utf-8-sig')
    assert all(s in log for s in ('Build succeeded.', '0 Warning(s)', '0 Error(s)'))
    queue = TESTS/'queue/bin/QueuePressureTests.exe'
    q = subprocess.run([str(queue)], capture_output=True, text=True, timeout=20)
    assert q.returncode == 0 and q.stdout.startswith('PASS 12 '), q
    fixtures = {name: load(TESTS/name/'fixture-result.json') for name in ('normal', 'abrupt')}
    assert all(v['passed'] for v in fixtures.values())
    for rel in pins:
        dest = OUT/rel
        dest.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(WORK/rel, dest)
    for name in ('opening-validation-sources', 'opening-contract-snapshots', 'validation-sources', 'contract-snapshots', 'transition-validation-sources', 'transition-contract-snapshots'):
        if (PARENT/name).exists(): shutil.copytree(PARENT/name, OUT/name)
    (OUT/'Release').mkdir()
    shutil.copyfile(WORK/'Release/ddraw.dll', OUT/'Release/ddraw.dll')
    shutil.copyfile(WORK/'build.log', OUT/'build-recorder-01.log')
    shutil.copyfile(PARENT/'qfr-source.json', OUT/'parent-passed-opening.json')
    shutil.copyfile(PARENT/'parent-main-source.json', OUT/'parent-main-source.json')
    for name in ('QueuePressureTests.cpp', 'QueuePressureTests.vcxproj', 'freeze_recorder.py'):
        shutil.copyfile(HERE/name, OUT/'opening-validation-sources'/name)
    result = dict(schema='quantum-opening-recorder-checks-1', parent_manifest_sha256=parent_hash,
        changes=[dict(path=rel, before=pins[rel], after=sha(OUT/rel)) for rel in changed],
        wrapper_sha256=sha(OUT/'Release/ddraw.dll'), clean_build=True,
        queue=dict(stdout=q.stdout, exit=q.returncode, binary_sha256=sha(queue)),
        fixtures=fixtures, live_performance_validated=False,
        rendering_sources_unchanged=True,
        limitations=['Console fixtures do not prove live performance.',
            'Opening profile can reject events under pressure: lost counter and incomplete footer declare this.',
            'Full flight-recorder profiles retain their previous blocking policy.'])
    (OUT/'validation').mkdir()
    (OUT/'validation/recorder-checks.json').write_text(json.dumps(result, indent=2)+'\n')
    shutil.copytree(TESTS, OUT/'validation/fixtures')
    old = {x['path']: x for x in m['changes']}
    for rel in changed:
        old[rel]['after'] = sha(OUT/rel)
    m['changes'] = [old[rel] for rel in sorted(old)]
    m['opening_flow']['source_files'] = {rel: sha(OUT/rel) for rel in sorted(pins)}
    m['opening_flow']['wrapper_sha256'] = result['wrapper_sha256']
    m['opening_flow']['work_source'] = str(WORK)
    m['opening_flow']['validation_sources'] = {p.name:sha(p) for p in (OUT/'opening-validation-sources').iterdir() if p.is_file()}
    m['recorder_followup'] = dict(parent_manifest_sha256=parent_hash, checks_sha256=sha(OUT/'validation/recorder-checks.json'),
        scope='Recorder-only; preserves live-passed opening rendering implementation', live_performance_validated=False)
    (OUT/'qfr-source.json').write_text(json.dumps(m, indent=2)+'\n')
    print(json.dumps(dict(source=str(OUT), manifest_sha256=sha(OUT/'qfr-source.json'), wrapper_sha256=result['wrapper_sha256'], changed_files=len(changed), live_performance_validated=False)))
