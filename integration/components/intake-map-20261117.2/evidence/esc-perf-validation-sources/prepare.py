"""New source child of accepted source-v2; no released file is modified."""
import hashlib,json,shutil
from pathlib import Path
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
base=Path('E:/Codex/research/dominion-rendering')
source=base/'production-runtime-source-v2';work=base/'esc-main-menu-work-01'
assert not work.exists()
assert sha(source/'qfr-source.json')=='c992873dcabf1047b196d4fbf708f3bcae046506c7413318ed9fea12c7944b84'
m=json.loads((source/'qfr-source.json').read_text())
for rel,h in m['opening_flow']['source_files'].items():
 assert sha(source/rel)==h,rel
 p=work/rel;p.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(source/rel,p)
shutil.copyfile(source/'qfr-source.json',work/'parent-production-runtime-v2.json')
(work/'prepare-receipt.json').write_text(json.dumps(dict(parent=str(source),parent_manifest_sha256=sha(source/'qfr-source.json'),production_pins_verified=len(m['opening_flow']['source_files']),baseline='int-20261029.1',game_modified=False),indent=2)+'\n')
print(work)
