"""Create a new work tree from the tested HD-video/HUD source."""
import hashlib,json,shutil
from pathlib import Path
base=Path('E:/Codex/research/dominion-rendering');parent=base/'hd-video-source-v1';work=base/'opening-cards-work-01'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
assert not work.exists()
assert sha(parent/'qfr-source.json')=='f49c0d39864bd461bfa43b459c3201ba68f855d1f08804b2e5d8957cae0bf1e3'
m=json.loads((parent/'qfr-source.json').read_text())
for r,h in m['opening_flow']['source_files'].items():
 assert sha(parent/r)==h,r
 d=work/r;d.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(parent/r,d)
shutil.copyfile(parent/'qfr-source.json',work/'parent-hd-source.json')
print(json.dumps(dict(work=str(work),files_verified=len(m['opening_flow']['source_files']))))
