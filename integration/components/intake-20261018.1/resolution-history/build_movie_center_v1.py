"""Exact movie-origin correction on the passed HUD/strip-clear image."""
import hashlib, json, struct, sys
from pathlib import Path
sys.path.insert(0, 'E:/Codex/tools/pydeps')
import pefile
ROOT=Path(__file__).resolve().parent
BASE='cc03aa48b6ca4e782f7067f8797a04179484fcfea884765048c2566740b01c6d'
def jump(src,dst): return b'\xe9'+struct.pack('<i',dst-src-5)
def build(raw):
    if hashlib.sha256(raw).hexdigest()!=BASE: raise ValueError('Exact passed strip-clear base required')
    p=pefile.PE(data=raw);s=p.sections[-1];base=p.OPTIONAL_HEADER.ImageBase
    if s.Name.rstrip(b'\0')!=b'.dwui': raise ValueError('Unexpected code section')
    used=s.Misc_VirtualSize;offset=s.PointerToRawData+used;target=base+s.VirtualAddress+used
    body=bytes.fromhex('8d4050a3a0e65d00')
    stub=body+jump(target+len(body),0x40f336)
    if used+len(stub)>s.SizeOfRawData or any(raw[offset:offset+len(stub)]):
        raise ValueError('No verified unused section capacity')
    out=bytearray(raw);patches=[]
    for va,before,after,label in [
        (0x40f01a,bytes.fromhex('c705a0e65d0000000000'),bytes.fromhex('c705a0e65d0050000000'),'default movie origin'),
        (0x40f331,bytes.fromhex('a3a0e65d00'),jump(0x40f331,target),'explicit movie origin')]:
        pos=p.get_offset_from_rva(va-base)
        if raw[pos:pos+len(before)]!=before: raise ValueError('Unexpected original instruction')
        out[pos:pos+len(before)]=after
        patches.append(dict(va=hex(va),file_offset=pos,before=before.hex(),after=after.hex(),label=label))
    out[offset:offset+len(stub)]=stub
    struct.pack_into('<I',out,s.get_file_offset()+8,used+len(stub))
    return bytes(out),dict(base_sha256=BASE,candidate_sha256=hashlib.sha256(out).hexdigest(),
        patches=patches,stub_file_offset=offset,stub_va=hex(target),stub_bytes=stub.hex(),
        virtual_size_field=s.get_file_offset()+8,ui_offset=80,runtime_validated=False)
if __name__=='__main__':
    out,r=build((ROOT/'game-strip-clear-v1/ds9dw.exe').read_bytes())
    exe=ROOT/'movie-center-probe-v1.exe';manifest=ROOT/'movie-center-patch-v1.json'
    if exe.exists() or manifest.exists():raise FileExistsError('Preserve previous artifacts')
    with exe.open('xb') as f:f.write(out)
    with manifest.open('x') as f:json.dump(r,f,indent=2)
    print(r['candidate_sha256'])
