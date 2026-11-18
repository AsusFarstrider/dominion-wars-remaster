"""Copy only pinned accepted production sources into a new HD research tree."""
import hashlib,json,shutil
from pathlib import Path
base=Path('E:/Codex/research/dominion-rendering')
parent=base/'hud-assembly-source-v2';work=base/'hd-video-work-01'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
assert not work.exists()
assert sha(parent/'qfr-source.json')=='af8001718c1116a666bb3d4d6b17e8e343d210b6ed1bd7ec130fbd70f1d6bc4d'
m=json.loads((parent/'qfr-source.json').read_text())
for r,h in m['opening_flow']['source_files'].items():
 assert sha(parent/r)==h,r
 d=work/r;d.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(parent/r,d)
shutil.copyfile(parent/'qfr-source.json',work/'parent-hud-source.json')
print(json.dumps(dict(work=str(work),verified=len(m['opening_flow']['source_files']))))
