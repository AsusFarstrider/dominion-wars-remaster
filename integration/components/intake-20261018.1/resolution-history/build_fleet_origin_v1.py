"""Consolidate fleet local draw/clip coordinates around the existing root origin."""
import hashlib,json,struct,sys
from pathlib import Path
sys.path.insert(0,'E:/Codex/tools/pydeps')
import pefile
from capstone import Cs,CS_ARCH_X86,CS_MODE_32
ROOT=Path(__file__).resolve().parent
BASE='e39518b9e89debb6fb7a2d330b3f84ef73398a8f0f90fd7f1804fd4aeaf3c5aa'
ORIGINAL='03c20a3535d0209111ff9f3b90eba403e059df2be7e784d2746abf1535c1be6a'
ORIGIN=0x5f7ee0
CLIPS={0x46faa0:(0x20,'imm',258),0x46fc1a:(0x20,'imm',258),
       0x46fdfa:(0x20,'imm',258),0x46ff86:(0x3c,'bx',0),
       0x47126d:(0x14,'bx',0),0x471304:(0x14,'bx',0),
       0x47139b:(0x14,'bx',0),0x47164f:(0x14,'imm',524),
       0x4716ed:(0x14,'bx',0),0x471784:(0x14,'bx',0)}

def branch(op,src,dst):return bytes([op])+struct.pack('<i',dst-src-5)

def build(raw,original):
    if hashlib.sha256(raw).hexdigest()!=BASE or hashlib.sha256(original).hexdigest()!=ORIGINAL:
        raise ValueError('Exact notification candidate and original required')
    p=pefile.PE(data=raw);op=pefile.PE(data=original);s=p.sections[-1]
    if s.Name.rstrip(b'\0')!=b'.dwui':raise ValueError('Unexpected section')
    used=s.Misc_VirtualSize;start=0x400000+s.VirtualAddress+used;offset=s.PointerToRawData+used
    out=bytearray(raw);code=bytearray();changes=[];wrappers={}
    def emit_stub(blob):
        va=start+len(code);code.extend(blob);return va
    def patch(va,before,after,label):
        pos=p.get_offset_from_rva(va-0x400000)
        if raw[pos:pos+len(before)]!=before or len(before)!=len(after):raise ValueError(hex(va))
        out[pos:pos+len(after)]=after
        changes.append(dict(va=hex(va),before=before.hex(),after=after.hex(),label=label))
    # Tail adapters translate one local image X, preserving incoming registers/flags.
    for dest,slot in ((0x40f630,0x14),(0x49f050,0x18)):
        va=start+len(code)
        b=bytearray.fromhex('9c50528b4424')+bytes([slot])+bytes.fromhex('0fbf15')+struct.pack('<I',ORIGIN)
        b+=bytes.fromhex('01d0894424')+bytes([slot])+bytes.fromhex('5a589d')
        b+=branch(0xe9,va+len(b),dest);wrappers[dest]=emit_stub(b)
    # Save exact font pointer and origin locally; restore local X after the call.
    va=start+len(code)
    b=bytearray.fromhex('558becff35b8c34e000fbf05')+struct.pack('<I',ORIGIN)+b'\x50'
    before=bytes.fromhex('9c50528b45fc8b55f86601500c5a589d')
    after=bytes.fromhex('9c50528b45fc8b55f86629500c5a589d')
    b+=before+bytes.fromhex('ff750cff7508')
    b+=branch(0xe8,va+len(b),0x4a2650)
    b+=bytes.fromhex('8d642408')+after+bytes.fromhex('8d65005dc3')
    wrappers[0x4a2650]=emit_stub(b)
    # Wrapped text accepts a local rectangle; translate a copy, not caller memory.
    va=start+len(code)
    b=bytearray.fromhex('558bec83ec088b450c8b108955f88b50048955fc0fbf05')+struct.pack('<I',ORIGIN)
    b+=bytes.fromhex('660145f8ff7518ff7514ff75108d45f850ff7508')
    b+=branch(0xe8,va+len(b),0x41e7f0)+bytes.fromhex('8d65005dc3')
    wrappers[0x41e7f0]=emit_stub(b)
    decoder=Cs(CS_ARCH_X86,CS_MODE_32)
    rows=list(decoder.disasm(op.get_data(0x6f030,0x3239),0x46f030))
    sites=[]
    # The first image pair draws a widget-relative background, already absolute.
    excluded={0x46f0ce,0x46f0da}
    for i in rows:
        if i.mnemonic!='call' or i.size!=5 or i.address in excluded:continue
        try:dest=int(i.op_str,16)
        except ValueError:continue
        if dest not in wrappers:continue
        pos=p.get_offset_from_rva(i.address-0x400000)
        patch(i.address,raw[pos:pos+5],branch(0xe8,i.address,wrappers[dest]),'fleet local drawing via root origin')
        sites.append(dict(va=hex(i.address),destination=hex(dest)))
    # Remove prior literal origin corrections; all these positions are now local.
    resets=[(0x46f044,'bd92000000','bd42000000'),
            (0x46f110,'66c7420cf802','66c7420ca802'),
            (0x470c5f,'68c3020000','6873020000'),(0x470cc5,'68c3020000','6873020000'),
            (0x46fc86,'684a010000','68fa000000'),(0x46fe66,'684a010000','68fa000000'),
            (0x46fffe,'684c010000','68fc000000')]
    for va,b,a in resets:patch(va,bytes.fromhex(b),bytes.fromhex(a),'restore legacy local coordinate; adapter owns translation')
    # Local clipping rectangle stores use the same root origin. Width/Y untouched.
    clip_helpers={}
    for site,(slot,kind,value) in CLIPS.items():
        key=(slot,kind,value)
        if key not in clip_helpers:
            b=bytearray.fromhex('9c50')
            b+=bytes.fromhex('6689d8') if kind=='bx' else b'\x66\xb8'+struct.pack('<H',value)
            b+=bytes.fromhex('660305')+struct.pack('<I',ORIGIN)
            b+=bytes.fromhex('66894424')+bytes([slot+12])+bytes.fromhex('589dc3')
            clip_helpers[key]=emit_stub(b)
        pos=op.get_offset_from_rva(site-0x400000)
        expected=original[pos:pos+(5 if kind=='bx' else 7)]
        patch(site,expected,branch(0xe8,site,clip_helpers[key])+b'\x90'*(len(expected)-5),'fleet local clip left via root origin')
    if used+len(code)>s.SizeOfRawData or any(raw[offset:offset+len(code)]):raise ValueError('No stub capacity')
    out[offset:offset+len(code)]=code
    struct.pack_into('<I',out,s.get_file_offset()+8,used+len(code))
    return bytes(out),dict(base_sha256=BASE,candidate_sha256=hashlib.sha256(out).hexdigest(),
        patches=changes,call_sites=sites,wrappers={hex(k):hex(v) for k,v in wrappers.items()},
        excluded_background_sites=[hex(x) for x in sorted(excluded)],restored_literal_sites=[hex(x[0]) for x in resets],
        clip_sites=[hex(x) for x in CLIPS],stub_va=hex(start),stub_bytes=code.hex(),runtime_validated=False,
        limitations=['Fleet tab branches need visual validation. No global renderer hooks.',
                     'Old private adapter bytes retained but old fleet call redirects superseded.',
                     'Loading bar is outside this renderer and unchanged.'])

if __name__=='__main__':
    data,report=build((ROOT/'game-notification-origin-v1/ds9dw.exe').read_bytes(),(ROOT/'game-test/ds9dw.exe').read_bytes())
    if (ROOT/'fleet-origin-probe-v1.exe').exists() or (ROOT/'fleet-origin-patch-v1.json').exists():raise FileExistsError('Preserve artifacts')
    with (ROOT/'fleet-origin-probe-v1.exe').open('xb') as f:f.write(data)
    with (ROOT/'fleet-origin-patch-v1.json').open('x') as f:json.dump(report,f,indent=2)
    print(report['candidate_sha256']);print(len(report['call_sites']))
