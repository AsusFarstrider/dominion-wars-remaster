"""Targeted missed objective rectangle and large fleet-image origins."""
import hashlib,json,struct,sys
from pathlib import Path
sys.path.insert(0,'E:/Codex/tools/pydeps')
import pefile
ROOT=Path(__file__).resolve().parent
BASE='e0aa7cd71aed1d63fc2e945fadfe7fc3aafc6a77d94cec1c6e86f8593e52bc0c'
def rel(op,src,dst):return bytes([op])+struct.pack('<i',dst-src-5)
def build(raw, top_info=False):
    if hashlib.sha256(raw).hexdigest()!=BASE:raise ValueError('Exact brief-centered base required')
    p=pefile.PE(data=raw);s=p.sections[-1];used=s.Misc_VirtualSize
    start=0x400000+s.VirtualAddress+used;offset=s.PointerToRawData+used
    out=bytearray(raw);stubs=bytearray();patches=[]
    def patch(va,before,after,label):
        pos=p.get_offset_from_rva(va-0x400000);before=bytes.fromhex(before)
        if raw[pos:pos+len(before)]!=before or len(before)!=len(after):raise ValueError(hex(va))
        out[pos:pos+len(before)]=after
        patches.append(dict(va=hex(va),file_offset=pos,before=before.hex(),after=after.hex(),label=label))
    patch(0x415a5e,'66c744243c8500',bytes.fromhex('66c744243cd500'),'objective description rectangle X133 to213')
    if top_info:
        patch(0x46f044,'bd42000000',bytes.fromhex('bd92000000'),'upper-left info base X66 to146, including its text clip')
    for va in [0x470c5f,0x470cc5]:
        patch(va,'6873020000',bytes.fromhex('68c3020000'),'captain main image X627 to707')
    for site,dest,slot in [(0x470b16,0x40f630,12),(0x470b22,0x49f050,16)]:
        target=start+len(stubs)
        code=bytes.fromhex('508b4424')+bytes([slot])+bytes.fromhex('8d4050894424')+bytes([slot])+b'\x58'
        code+=rel(0xe9,target+len(code),dest);stubs+=code
        patch(site,rel(0xe8,site,dest).hex(),rel(0xe8,site,target),'ship main image X70 to150')
    if s.Name.rstrip(b'\0')!=b'.dwui' or used+len(stubs)>s.SizeOfRawData or any(raw[offset:offset+len(stubs)]):raise ValueError('No verified stub capacity')
    out[offset:offset+len(stubs)]=stubs
    struct.pack_into('<I',out,s.get_file_offset()+8,used+len(stubs))
    return bytes(out),dict(base_sha256=BASE,candidate_sha256=hashlib.sha256(out).hexdigest(),patches=patches,
        stub_va=hex(start),stub_file_offset=offset,stub_bytes=stubs.hex(),virtual_size_field=s.get_file_offset()+8,runtime_validated=False)
if __name__=='__main__':
    out,r=build((ROOT/'game-brief-center-v1/ds9dw.exe').read_bytes())
    exe=ROOT/'panel-details-probe-v1.exe';manifest=ROOT/'panel-details-patch-v1.json'
    if exe.exists() or manifest.exists():raise FileExistsError('Preserve artifacts')
    with exe.open('xb') as f:f.write(out)
    with manifest.open('x') as f:json.dump(r,f,indent=2)
    print(r['candidate_sha256'])
