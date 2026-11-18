"""Copy and verify the accepted opening source for an isolated HUD trial."""
import hashlib,json,shutil
from pathlib import Path
BASE=Path('E:/Codex/research/dominion-rendering')
PARENT=BASE/'opening-submenus-source-v5'
WORK=BASE/'hud-captains-work-01'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
if __name__=='__main__':
    if WORK.exists():raise ValueError('Use a new work directory')
    assert sha(PARENT/'qfr-source.json')=='04dfe4c49b7d8cac1414f6e96e06347c6f68c701a0531a840dbb1c0678fd0b6f'
    m=json.loads((PARENT/'qfr-source.json').read_text())
    for rel,digest in m['opening_flow']['source_files'].items():
        assert sha(PARENT/rel)==digest,rel
        dest=WORK/rel;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(PARENT/rel,dest)
    shutil.copyfile(PARENT/'qfr-source.json',WORK/'parent-opening-source.json')
    print(json.dumps(dict(work=str(WORK),verified_source_files=len(m['opening_flow']['source_files']),current_modified=False)))
