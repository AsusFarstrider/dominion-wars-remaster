import hashlib,json,shutil
from pathlib import Path
base=Path('E:/Codex/research/dominion-rendering');parent=base/'gizmo-main-source-v5';work=base/'production-runtime-work-01'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
assert not work.exists()
assert sha(parent/'qfr-source.json')=='5f375031db2b6c190c9f4bb4485997bbcb2bf90bc1a379582c29b3adf4ba7c5f'
m=json.loads((parent/'qfr-source.json').read_text())
for r,h in m['opening_flow']['source_files'].items():
 assert sha(parent/r)==h,r
 d=work/r;d.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(parent/r,d)
shutil.copyfile(parent/'qfr-source.json',work/'parent-gizmo-main-source.json')
print(json.dumps(dict(work=str(work),parent_files=len(m['opening_flow']['source_files']))))
