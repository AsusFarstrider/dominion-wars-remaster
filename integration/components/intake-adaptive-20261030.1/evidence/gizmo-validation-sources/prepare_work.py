"""Start the next isolated candidate from the tested cards/HD/HUD source."""
import hashlib,json,shutil
from pathlib import Path
base=Path('E:/Codex/research/dominion-rendering');parent=base/'opening-cards-source-v1';work=base/'gizmo-main-work-01'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
assert not work.exists()
assert sha(parent/'qfr-source.json')=='5f154f7d058b2795a9645577581aa190cb781e3d5ea195ded241f68be8d07443'
m=json.loads((parent/'qfr-source.json').read_text())
for r,h in m['opening_flow']['source_files'].items():
 assert sha(parent/r)==h,r
 d=work/r;d.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(parent/r,d)
shutil.copyfile(parent/'qfr-source.json',work/'parent-cards-source.json')
print(json.dumps(dict(work=str(work),files_verified=len(m['opening_flow']['source_files']))))
