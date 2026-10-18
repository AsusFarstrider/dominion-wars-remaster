"""Correct the shared confirmation dialog's local centering width only."""
import hashlib,json,sys
from pathlib import Path
sys.path.insert(0,'E:/Codex/tools/pydeps')
import pefile
ROOT=Path(__file__).resolve().parent
BASE='b5d4caa6763c6a0030aaa1c3c7d8e6177112b37a0cf90d28c1f252e6bb45a9b6'
def build(raw):
    if hashlib.sha256(raw).hexdigest()!=BASE:raise ValueError('Exact movie-centered base required')
    p=pefile.PE(data=raw);pos=p.get_offset_from_rva(0x237f9)
    before=bytes.fromhex('668b7810');after=bytes.fromhex('66bf2003')
    if raw[pos:pos+4]!=before:raise ValueError('Unexpected dialog instruction')
    out=raw[:pos]+after+raw[pos+4:]
    return out,dict(base_sha256=BASE,candidate_sha256=hashlib.sha256(out).hexdigest(),
        va='0x4237f9',file_offset=pos,before=before.hex(),after=after.hex(),runtime_validated=False)
if __name__=='__main__':
    out,r=build((ROOT/'game-movie-center-v1/ds9dw.exe').read_bytes())
    exe=ROOT/'dialog-center-probe-v1.exe';manifest=ROOT/'dialog-center-patch-v1.json'
    if exe.exists() or manifest.exists():raise FileExistsError('Preserve artifacts')
    with exe.open('xb') as f:f.write(out)
    with manifest.open('x') as f:json.dump(r,f,indent=2)
    print(r['candidate_sha256'])
