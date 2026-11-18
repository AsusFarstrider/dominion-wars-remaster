"""Freeze the two source-backed corrections without modifying the first trial."""
import hashlib
import json
from pathlib import Path
import shutil
import subprocess

base = Path('E:/Codex/research/dominion-rendering')
parent = base / 'hud-captains-source-v1'
work = base / 'hud-captains-work-02'
out = base / 'hud-captains-source-v2'
here = Path(__file__).resolve().parent
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
assert not out.exists(), 'Use a new frozen source'
assert sha(parent / 'qfr-source.json') == '76e88f1d3b3500a04790ff5330f744efc73d8ab09e1b68076ea8ab149c22e315'
m = json.loads((parent / 'qfr-source.json').read_text())
pins = m['opening_flow']['source_files']
assert all(sha(parent / r) == h for r, h in pins.items())
changed = sorted(r for r, h in pins.items() if sha(work / r) != h)
assert changed == ['DDrawCompat/Common/VtableHookVisitor.h',
    'DDrawCompat/Research/HudCaptain/HudCaptain.cpp', 'DDrawCompat/Research/HudCaptain/HudCaptain.h'], changed
log = work / 'build-hud-01.log'
text = log.read_text(encoding='utf-8-sig')
assert all(s in text for s in ('Build succeeded.', '0 Warning(s)', '0 Error(s)'))
binary = base / 'hud-captains-tests-02/bin/HudCaptainTests.exe'
test = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
assert test.returncode == 0 and test.stdout.startswith('PASS 556 '), test
for rel in pins:
    dest = out / rel
    dest.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(work / rel, dest)
for name in ('validation-sources', 'contract-snapshots', 'transition-validation-sources',
             'transition-contract-snapshots', 'opening-validation-sources',
             'opening-contract-snapshots', 'captain-contract-snapshots'):
    shutil.copytree(parent / name, out / name)
for name in ('parent-main-source.json', 'parent-recorder-source.json', 'parent-opening-source.json'):
    shutil.copyfile(parent / name, out / name)
shutil.copyfile(parent / 'qfr-source.json', out / 'parent-captain-source.json')
contract = Path('E:/Codex/research/quantum-engine-kb/docs/HUD-ESCAPE-AND-STRIP-HANDOFF-01.md')
shutil.copyfile(contract, out / 'captain-contract-snapshots' / contract.name)
tests = out / 'captain-validation-sources'
tests.mkdir()
for p in here.iterdir():
    if p.is_file() and p.suffix in ('.py', '.ps1', '.h', '.cpp', '.vcxproj'):
        shutil.copyfile(p, tests / p.name)
shutil.copyfile(parent / 'captain-validation-sources/RuntimeTestMemory.h', tests / 'RuntimeTestMemory.h')
(out / 'Release').mkdir()
shutil.copyfile(work / 'Release/ddraw.dll', out / 'Release/ddraw.dll')
shutil.copyfile(log, out / log.name)
for entry in m['changes']:
    if entry['path'] in changed:
        entry['after'] = sha(out / entry['path'])
m['opening_flow']['source_files'] = {r: sha(out / r) for r in sorted(pins)}
m['opening_flow']['wrapper_sha256'] = sha(out / 'Release/ddraw.dll')
m['opening_flow']['work_source'] = str(work)
c = m['captain_trial']
c.update(parent_manifest_sha256=sha(parent / 'qfr-source.json'), expected_assertions=556,
         validation=dict(stdout=test.stdout, exit=test.returncode, binary_sha256=sha(binary)),
         validation_sources={p.name: sha(p) for p in tests.iterdir()}, live_validated=False,
         corrections=['Verified mission-pause graphics ownership with separate modal input guard.',
                      'Exact original strip Clear2 private rectangle extends only right edge; single original call.'],
         limitations=['First trial partial pass, preserved; two corrections require live validation.',
                      'Temporary captain layout and original-density art; remaining HUD unchanged.',
                      'Auxiliary states, paging, other displays and all hardware paths are not comprehensively validated.'])
c['contracts'].append(dict(path=str(contract), sha256=sha(contract)))
(out / 'qfr-source.json').write_text(json.dumps(m, indent=2) + '\n')
print(json.dumps(dict(source=str(out), manifest_sha256=sha(out / 'qfr-source.json'),
                      wrapper_sha256=m['opening_flow']['wrapper_sha256'], changed=changed,
                      test=test.stdout.strip(), game_launched=False)))
