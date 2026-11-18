import hashlib,json,shutil
from pathlib import Path
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
base=Path('E:/Codex/research/dominion-rendering');parent=base/'esc-main-menu-source-v1';work=base/'esc-menu-perf-work-01'
assert not work.exists();assert sha(parent/'qfr-source.json')=='05da2a8831a634e45bfc21b9c65d7e8845b1a327bfaaccfcaace363dafc301bb'
m=json.loads((parent/'qfr-source.json').read_text())
for r,h in m['opening_flow']['source_files'].items():
 assert sha(parent/r)==h,r
 p=work/r;p.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(parent/r,p)
shutil.copyfile(parent/'qfr-source.json',work/'parent-esc-menu-source-v1.json')
print(work)
