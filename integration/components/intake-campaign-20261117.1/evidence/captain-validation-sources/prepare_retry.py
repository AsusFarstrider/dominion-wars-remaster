"""Derive a new captain work tree from the preserved first live trial."""
import hashlib
import json
from pathlib import Path
import shutil

base = Path('E:/Codex/research/dominion-rendering')
parent = base / 'hud-captains-source-v1'
work = base / 'hud-captains-work-02'
sha = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
assert not work.exists(), 'Use a new work tree'
assert sha(parent / 'qfr-source.json') == '76e88f1d3b3500a04790ff5330f744efc73d8ab09e1b68076ea8ab149c22e315'
manifest = json.loads((parent / 'qfr-source.json').read_text())
for rel, digest in manifest['opening_flow']['source_files'].items():
    assert sha(parent / rel) == digest, rel
    dest = work / rel
    dest.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(parent / rel, dest)
shutil.copyfile(parent / 'qfr-source.json', work / 'parent-captain-source.json')
print(json.dumps(dict(work=str(work), verified_source_files=len(manifest['opening_flow']['source_files']))))
