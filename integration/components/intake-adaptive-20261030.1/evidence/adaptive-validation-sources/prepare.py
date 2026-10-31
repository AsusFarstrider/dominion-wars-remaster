"""Isolated adaptive-output child; accepted source and Current stay immutable."""
import hashlib,json,shutil
from pathlib import Path
base=Path('E:/Codex/research/dominion-rendering');parent=base/'esc-menu-perf-source-v1';work=base/'dynamic-output-work-01'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
assert not work.exists();assert sha(parent/'qfr-source.json')=='e0933cd5271c5f74963c65771e7ad9abf3866a1a0aa112889e48c5f53f5e20d4'
m=json.loads((parent/'qfr-source.json').read_text())
for r,h in m['opening_flow']['source_files'].items():
 assert sha(parent/r)==h,r
 p=work/r;p.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(parent/r,p)
shutil.copyfile(parent/'qfr-source.json',work/'parent-esc-menu-perf-source-v1.json')
print(work)
