"""Briefing-only original-canvas positions and repaint probe."""
import hashlib,json,struct,sys
from pathlib import Path
sys.path.insert(0,'E:/Codex/tools/pydeps')
import pefile
ROOT=Path(__file__).resolve().parent
BASE='df4583a424892caf9a7053752e64111dd464e42deba19f0748bb5482afaa256c'
def rel(op,src,dst):return bytes([op])+struct.pack('<i',dst-src-5)
def build(raw):
    if hashlib.sha256(raw).hexdigest()!=BASE:raise ValueError('Exact fleet-repaint base required')
    p=pefile.PE(data=raw);s=p.sections[-1];used=s.Misc_VirtualSize
    start=0x400000+s.VirtualAddress+used;offset=s.PointerToRawData+used
    stubs=bytearray();out=bytearray(raw);patches=[]
    def patch(va,before,after,label):
        pos=p.get_offset_from_rva(va-0x400000);before=bytes.fromhex(before)
        if raw[pos:pos+len(before)]!=before or len(before)!=len(after):raise ValueError(hex(va))
        out[pos:pos+len(before)]=after
        patches.append(dict(va=hex(va),file_offset=pos,before=before.hex(),after=after.hex(),label=label))
    entry=start
    body=bytearray.fromhex('9c60a1649ba3008b006a0050')
    body+=rel(0xe8,entry+len(body),0x4a1710)
    body+=bytes.fromhex('83c408619d8b0d6c387100')
    body+=rel(0xe9,entry+len(body),0x415b16);stubs+=body
    patch(0x415b10,'8b0d6c387100',rel(0xe9,0x415b10,entry)+b'\x90','brief repaint')
    # Original direct draws, not widget-local coordinates.
    for va,before,after in [
        (0x415b47,'6a00','6a50'),
        (0x415bb7,'66c7410ca400','66c7410cf400'),
        (0x415bed,'66c7420ccc00','66c7420c1c01'),
        (0x415c1d,'66c7410c3e01','66c7410c8e01'),
        (0x415c55,'66c7400c8f02','66c7400cdf02'),
        (0x415ca7,'68d6010000','6826020000'),
        (0x415ce0,'6810020000','6860020000'),
        (0x415d2c,'66c7420c7900','66c7420cc900'),
        (0x4159d3,'68cd010000','681d020000'),
        (0x415a1b,'66c7400c8900','66c7400cd900'),
        (0x415ab2,'66c7410ce301','66c7410c3302'),
        (0x4c7f38,'7900','c900')]:
        patch(va,before,bytes.fromhex(after),'brief direct X +80')
    # PUSH imm8 121 cannot encode 201. Redirect just these two calls, leaving
    # the original argument setup and the alternate image-format branches intact.
    for site,dest,slot in [(0x41594b,0x40f630,12),(0x415957,0x49f050,16)]:
        target=start+len(stubs)
        # Save EAX; adjust this invocation's stack X, preserve flags; tailcall.
        code=bytes.fromhex('508b4424')+bytes([slot])+bytes.fromhex('8d4050894424')+bytes([slot])+b'\x58'
        code+=rel(0xe9,target+len(code),dest);stubs+=code
        patch(site,rel(0xe8,site,dest).hex(),rel(0xe8,site,target),'objective image X +80')
    if s.Name.rstrip(b'\0')!=b'.dwui' or used+len(stubs)>s.SizeOfRawData or any(raw[offset:offset+len(stubs)]):raise ValueError('No verified stub capacity')
    out[offset:offset+len(stubs)]=stubs
    struct.pack_into('<I',out,s.get_file_offset()+8,used+len(stubs))
    return bytes(out),dict(base_sha256=BASE,candidate_sha256=hashlib.sha256(out).hexdigest(),patches=patches,
        stub_va=hex(start),stub_file_offset=offset,stub_bytes=stubs.hex(),virtual_size_field=s.get_file_offset()+8,runtime_validated=False)
if __name__=='__main__':
    out,r=build((ROOT/'game-fleet-repaint-v1/ds9dw.exe').read_bytes())
    exe=ROOT/'brief-center-probe-v1.exe';manifest=ROOT/'brief-center-patch-v1.json'
    if exe.exists() or manifest.exists():raise FileExistsError('Preserve artifacts')
    with exe.open('xb') as f:f.write(out)
    with manifest.open('x') as f:json.dump(r,f,indent=2)
    print(r['candidate_sha256'])
