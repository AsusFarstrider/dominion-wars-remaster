"""Scoped fleet-middle draw-call correction; no global rendering hooks."""
import hashlib,json,struct,sys
from pathlib import Path
sys.path.insert(0,'E:/Codex/tools/pydeps')
import pefile
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
ROOT=Path(__file__).resolve().parent
BASE='27240be8df70f76c8884d6500612e924232a146c80a057c6d43c1cac63183235'
def rel(op,src,dst):return bytes([op])+struct.pack('<i',dst-src-5)
def build(raw):
    if hashlib.sha256(raw).hexdigest()!=BASE:raise ValueError('Exact panel-details v2 base required')
    p=pefile.PE(data=raw);s=p.sections[-1];used=s.Misc_VirtualSize
    start=0x400000+s.VirtualAddress+used;offset=s.PointerToRawData+used
    out=bytearray(raw);stubs=bytearray();patches=[]
    def patch(va,before,after,label):
        pos=p.get_offset_from_rva(va-0x400000)
        if raw[pos:pos+len(before)]!=before or len(before)!=len(after):raise ValueError(hex(va))
        out[pos:pos+len(before)]=after
        patches.append(dict(va=hex(va),file_offset=pos,before=before.hex(),after=after.hex(),label=label))
    wrappers={}
    for dest,slot in [(0x40f630,12),(0x49f050,16)]:
        target=start+len(stubs);wrappers[dest]=target
        code=bytes.fromhex('508b4424')+bytes([slot])+bytes.fromhex('8d4050894424')+bytes([slot])+b'\x58'
        stubs+=code+rel(0xe9,target+len(code),dest)
    # Font call wrapper: save the exact font-state pointer locally; shift X for
    # the call and subtract afterward, retaining any original text advancement.
    # EAX/result and flags around the original return are preserved.
    target=start+len(stubs);wrappers[0x4a2650]=target
    code=bytearray.fromhex('558becff35b8c34e009c508b45fc6683400c50589dff750cff7508')
    code+=rel(0xe8,target+len(code),0x4a2650)
    code+=bytes.fromhex('8d6424089c508b45fc6683680c50589d8d65005dc3');stubs+=code
    # Use the confirmed callback start for instruction alignment. Stop at its
    # observed epilogue; no jump-table bytes are accepted as CALL instructions.
    decoder=Cs(CS_ARCH_X86,CS_MODE_32);decoder.skipdata=True
    rows=list(decoder.disasm(p.get_data(0x6f030,0x3239),0x46f030))
    excluded={0x470c6a,0x470c76,0x470cd0,0x470cdc}
    sites=[]
    for ins in rows:
        if not 0x47070d<=ins.address<0x472269 or ins.mnemonic!='call' or ins.size!=5:continue
        try:dest=int(ins.op_str,16)
        except ValueError:continue
        if dest not in wrappers or ins.address in excluded:continue
        # Only exact relative CALL bytes; never inferred indirect-call targets.
        if bytes(ins.bytes)!=rel(0xe8,ins.address,dest):raise ValueError('Unexpected CALL')
        patch(ins.address,bytes(ins.bytes),rel(0xe8,ins.address,wrappers[dest]),'fleet middle direct draw X +80')
        sites.append(dict(va=hex(ins.address),destination=hex(dest)))
    if not sites or sites[0]['va']!='0x4708ba':raise ValueError('Unexpected middle-call scan')
    for va,before,after in [
        (0x46fc86,'68fa000000','684a010000'),
        (0x46fe66,'68fa000000','684a010000'),
        (0x46fffe,'68fc000000','684c010000')]:
        patch(va,bytes.fromhex(before),bytes.fromhex(after),'selected upper-left item image X +80')
    if s.Name.rstrip(b'\0')!=b'.dwui' or used+len(stubs)>s.SizeOfRawData or any(raw[offset:offset+len(stubs)]):raise ValueError('No verified stub capacity')
    out[offset:offset+len(stubs)]=stubs
    struct.pack_into('<I',out,s.get_file_offset()+8,used+len(stubs))
    return bytes(out),dict(base_sha256=BASE,candidate_sha256=hashlib.sha256(out).hexdigest(),patches=patches,
        call_sites=sites,wrappers={hex(k):hex(v) for k,v in wrappers.items()},
        stub_va=hex(start),stub_file_offset=offset,stub_bytes=stubs.hex(),virtual_size_field=s.get_file_offset()+8,runtime_validated=False)
if __name__=='__main__':
    out,r=build((ROOT/'game-panel-details-v2/ds9dw.exe').read_bytes())
    exe=ROOT/'fleet-middle-probe-v1.exe';manifest=ROOT/'fleet-middle-patch-v1.json'
    if exe.exists() or manifest.exists():raise FileExistsError('Preserve artifacts')
    with exe.open('xb') as f:f.write(out)
    with manifest.open('x') as f:json.dump(r,f,indent=2)
    print(r['candidate_sha256']);print('Scoped calls:',len(r['call_sites']))
