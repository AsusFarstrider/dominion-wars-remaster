"""Create the combined HUD work tree from the preserved passing captain retry."""
import hashlib
import json
from pathlib import Path
import shutil

base = Path('E:/Codex/research/dominion-rendering')
parent = base / 'hud-captains-source-v2'
work = base / 'hud-assembly-work-01'
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
assert not work.exists(), 'Use a new work tree'
assert sha(parent / 'qfr-source.json') == '758cc077fd2739b7f0511eed4e58d936a8d4aa19a435c2ffd8f0c8c7e60ee1a2'
m = json.loads((parent / 'qfr-source.json').read_text())
for rel, digest in m['opening_flow']['source_files'].items():
    assert sha(parent / rel) == digest, rel
    dest = work / rel
    dest.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(parent / rel, dest)
shutil.copyfile(parent / 'qfr-source.json', work / 'parent-captain-source.json')
print(json.dumps(dict(work=str(work), files_verified=len(m['opening_flow']['source_files']))))
