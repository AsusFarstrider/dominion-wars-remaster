"""Bounded fleet repaint/header probe on the passed dialog-centered image."""
import hashlib,json,struct,sys
from pathlib import Path
sys.path.insert(0,'E:/Codex/tools/pydeps')
import pefile
ROOT=Path(__file__).resolve().parent
BASE='20ccaf07a4652887d59d363c476b6f57effe7ff23d74ac2d577e37aebd33c05c'
def rel(op,src,dst):return bytes([op])+struct.pack('<i',dst-src-5)
def build(raw):
    if hashlib.sha256(raw).hexdigest()!=BASE:raise ValueError('Exact dialog base required')
    p=pefile.PE(data=raw);s=p.sections[-1];used=s.Misc_VirtualSize
    target=0x400000+s.VirtualAddress+used;offset=s.PointerToRawData+used
    # Save incoming flags/registers; existing fill helper takes (surface,0).
    stub=bytearray.fromhex('9c60a1649ba3008b006a0050')
    stub+=rel(0xe8,target+len(stub),0x4a1710)
    stub+=bytes.fromhex('83c408619d81ec88000000')
    stub+=rel(0xe9,target+len(stub),0x46f036)
    if s.Name.rstrip(b'\0')!=b'.dwui' or used+len(stub)>s.SizeOfRawData or any(raw[offset:offset+len(stub)]):raise ValueError('No verified stub capacity')
    out=bytearray(raw);patches=[]
    for va,before,after,label in [
        (0x46f030,bytes.fromhex('81ec88000000'),rel(0xe9,0x46f030,target)+b'\x90','fleet callback repaint'),
        (0x46f110,bytes.fromhex('66c7420ca802'),bytes.fromhex('66c7420cf802'),'fleet heading x680 to760')]:
        pos=p.get_offset_from_rva(va-0x400000)
        if raw[pos:pos+len(before)]!=before:raise ValueError('Unexpected instruction')
        out[pos:pos+len(before)]=after
        patches.append(dict(va=hex(va),file_offset=pos,before=before.hex(),after=after.hex(),label=label))
    out[offset:offset+len(stub)]=stub
    struct.pack_into('<I',out,s.get_file_offset()+8,used+len(stub))
    return bytes(out),dict(base_sha256=BASE,candidate_sha256=hashlib.sha256(out).hexdigest(),patches=patches,
        stub_va=hex(target),stub_file_offset=offset,stub_bytes=stub.hex(),virtual_size_field=s.get_file_offset()+8,runtime_validated=False)
if __name__=='__main__':
    out,r=build((ROOT/'game-dialog-center-v1/ds9dw.exe').read_bytes())
    exe=ROOT/'fleet-repaint-probe-v1.exe';manifest=ROOT/'fleet-repaint-patch-v1.json'
    if exe.exists() or manifest.exists():raise FileExistsError('Preserve artifacts')
    with exe.open('xb') as f:f.write(out)
    with manifest.open('x') as f:json.dump(r,f,indent=2)
    print(r['candidate_sha256'])
