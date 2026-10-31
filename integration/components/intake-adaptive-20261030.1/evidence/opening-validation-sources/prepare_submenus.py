"""Make the isolated submenu worktree and pin original root/hook identities."""
import hashlib
import json
from pathlib import Path
import shutil
import struct
import sys
sys.path.insert(0, 'E:/Codex/tools/pydeps')
import pefile

BASE = Path('E:/Codex/research/dominion-rendering')
PARENT = BASE/'opening-recorder-source-v1'
WORK = BASE/'opening-submenus-work-01'

def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()

if __name__ == '__main__':
    if WORK.exists(): raise ValueError('Use a new isolated work root')
    assert sha(PARENT/'qfr-source.json') == '6fee5d37b89c105ee350a34bea4776171cfe5bfcd62d42989ce547bb87d569b5'
    m = json.loads((PARENT/'qfr-source.json').read_text())
    for rel, digest in m['opening_flow']['source_files'].items():
        assert sha(PARENT/rel) == digest, rel
        out = WORK/rel
        out.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(PARENT/rel, out)
    for name in ('parent-main-source.json',): shutil.copyfile(PARENT/name, WORK/name)
    shutil.copyfile(PARENT/'qfr-source.json', WORK/'parent-recorder-source.json')
    original = Path('E:/Codex/research/installed-1.04/ds9dw.exe')
    assert sha(original) == '03c20a3535d0209111ff9f3b90eba403e059df2be7e784d2746abf1535c1be6a'
    pe = pefile.PE(str(original))
    def data(va, n): return pe.get_data(va-pe.OPTIONAL_HEADER.ImageBase, n)
    rows = []
    for name, va in (('preferences',0x4DB6F8),('load',0x4DF100),('credits',0x4E57B0),('multiplayer20',0x4E05E8),('multiplayer21',0x4E0828),('multiplayer22',0x4E1908)):
        b = data(va,0x90)
        u = lambda o:struct.unpack_from('<I',b,o)[0]
        x,y,w,h = struct.unpack_from('<hhHH',b,8)
        rows.append(dict(name=name,address=va,id=u(0),flags=u(4),x=x,y=y,width=w,height=h,input=u(0x48),draw=u(0x4C),next=u(0x6B),child=u(0x73),parent=u(0x77)))
    result = dict(schema='quantum-submenu-source-identities-1',original=str(original),original_sha256=sha(original),
        parent_manifest_sha256=sha(PARENT/'qfr-source.json'),roots=rows,
        proposed_hooks=[dict(address=0x421005,bytes=data(0x421005,6).hex(),role='slider private EBX/EBP after original reads/ancestry')])
    (WORK/'original-identities.json').write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps(result,indent=2))
