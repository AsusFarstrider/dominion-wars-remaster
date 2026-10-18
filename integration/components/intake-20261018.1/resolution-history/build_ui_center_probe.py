"""Exact wide960 UI centering probe; private PE code section, no asset edits."""
import hashlib,json,struct,sys
from pathlib import Path
sys.path.insert(0,'E:/Codex/tools/pydeps')
import pefile
ROOT=Path(__file__).resolve().parent
BASE='fe60e52637300045433bad7002dafeee846e05d127095ff3e6b9dbe8f4fb0ab9'
POPUPS=[0x4343a0,0x434445,0x4344d5,0x435319,0x435388,0x4359df,0x435b4a,0x435bb6,0x435c8b,0x435cf7,0x43615d,0x4367ae]
def align(x,a):return (x+a-1)//a*a
def jump(src,dst):return b'\xe9'+struct.pack('<i',dst-src-5)
def build(raw, local_hud_origins=False):
    if hashlib.sha256(raw).hexdigest()!=BASE:raise ValueError('Exact wide960 base required')
    p=pefile.PE(data=raw);base=p.OPTIONAL_HEADER.ImageBase
    assert base==0x400000 and not p.OPTIONAL_HEADER.DllCharacteristics&0x40
    assert p.get_overlay_data_start_offset() is None
    header=p.sections[-1].get_file_offset()+40
    assert header+40<=p.OPTIONAL_HEADER.SizeOfHeaders and not any(raw[header:header+40])
    rva=align(max(s.VirtualAddress+max(s.Misc_VirtualSize,s.SizeOfRawData) for s in p.sections),p.OPTIONAL_HEADER.SectionAlignment)
    rawpos=align(len(raw),p.OPTIONAL_HEADER.FileAlignment)
    out=bytearray(raw);stubs=bytearray();patches=[]
    def replace(va,expected,replacement,label):
        offset=p.get_offset_from_rva(va-base);old=bytes.fromhex(expected)
        if raw[offset:offset+len(old)]!=old:raise ValueError(f'Unexpected instruction at {va:x}')
        assert len(replacement)==len(old)
        out[offset:offset+len(old)]=replacement
        patches.append(dict(va=hex(va),file_offset=offset,before=old.hex(),after=replacement.hex(),label=label))
    def trampoline(va,expected,body,label):
        target=base+rva+len(stubs);size=len(bytes.fromhex(expected))
        stubs.extend(body);stubs.extend(jump(target+len(body),va+size))
        replace(va,expected,jump(va,target)+b'\x90'*(size-5),label)
    # Root initialize: preserve original flags write, then set packed root X=80.
    trampoline(0x41d39a,'c705dc7e5f0004000000',bytes.fromhex('c705dc7e5f000400000066c705e07e5f005000'),'root origin +80')
    # Parent offsets are accumulated by traversal; do not pass root X a second time.
    replace(0x421fc5,'0fbf0de07e5f00',bytes.fromhex('b9000000009090'),'draw root parent X zero')
    # UI minimap/preview coordinate globals, not the scene/picking width globals.
    replace(0x40190b,'8d7aff',bytes.fromhex('8d7a4f'),'UI right edge and preview origin +80')
    replace(0x40195d,'c705c8f65f0000000000',bytes.fromhex('c705c8f65f0050000000'),'UI left edge +80')
    trampoline(0x4019a9,'8915c0f65f00',bytes.fromhex('8d52508915c0f65f00'),'minimap center +80')
    # Scene input widget is a child of the shifted root. Cancel the offset there.
    replace(0x439f04,'66c70538c54d000000',bytes.fromhex('66c70538c54d00b0ff'),'scene input local X -80')
    # These identified popup helpers center against the original UI canvas width.
    for va in POPUPS:replace(va,'a170f65f00',bytes.fromhex('b820030000'),'popup local canvas width 800')
    if local_hud_origins:
        # Absolute map/preview globals remain shifted for D3D and picking.
        # Only convert the identified widget-local consumers back to UI space.
        # PUSHFD/POPFD preserves flags across the 16-bit subtraction.
        trampoline(0x479ca5,'668b0dd0f65f00',
            bytes.fromhex('668b0dd0f65f009c6683e9509d'),
            'HUD container local X excludes root translation')
        trampoline(0x479ce4,'8b15e0f65f00',
            bytes.fromhex('8b15e0f65f008d52b0'),
            'preview widget local X excludes root translation')
        # EDX already contains center-left at this point. Preserve it; the
        # following instruction replaces EAX, so LEA cannot leak a changed X.
        trampoline(0x459f33,'66a380f64d00',
            bytes.fromhex('8d40b066a380f64d00'),
            'minimap and scanning parent local X excludes root translation')
    rawsize=align(len(stubs),p.OPTIONAL_HEADER.FileAlignment)
    out.extend(b'\0'*(rawpos+rawsize-len(out)))
    out[rawpos:rawpos+len(stubs)]=stubs
    struct.pack_into('<8sIIIIIIHHI',out,header,b'.dwui\0\0\0',len(stubs),rva,rawsize,rawpos,0,0,0,0,0x60000020)
    struct.pack_into('<H',out,p.FILE_HEADER.get_field_absolute_offset('NumberOfSections'),p.FILE_HEADER.NumberOfSections+1)
    struct.pack_into('<I',out,p.OPTIONAL_HEADER.get_field_absolute_offset('SizeOfImage'),align(rva+len(stubs),p.OPTIONAL_HEADER.SectionAlignment))
    struct.pack_into('<I',out,p.OPTIONAL_HEADER.get_field_absolute_offset('SizeOfCode'),p.OPTIONAL_HEADER.SizeOfCode+rawsize)
    struct.pack_into('<I',out,p.OPTIONAL_HEADER.get_field_absolute_offset('CheckSum'),0)
    q=pefile.PE(data=out);assert q.OPTIONAL_HEADER.AddressOfEntryPoint==p.OPTIONAL_HEADER.AddressOfEntryPoint
    assert q.FILE_HEADER.TimeDateStamp==p.FILE_HEADER.TimeDateStamp
    assert q.get_data(0x33620,256)==p.get_data(0x33620,256)
    return bytes(out),dict(base_sha256=BASE,candidate_sha256=hashlib.sha256(out).hexdigest(),
        patches=patches,ui_offset=80,ui_local_canvas_width=800,logical_size=[960,600],
        local_hud_origins=local_hud_origins,
        stub_rva=hex(rva),stub_bytes=stubs.hex(),section='.dwui',runtime_validated=False,
        limits=['Static hierarchy inference; custom callbacks require runtime testing','Objective text and all uncommon dialogs not proven','No blanket mouse-coordinate translation'])
if __name__=='__main__':
    out,report=build((ROOT/'game-wide960-v1/ds9dw.exe').read_bytes())
    exe=ROOT/'ui-center-probe-v1.exe';manifest=ROOT/'ui-center-patch-v1.json'
    if exe.exists() or manifest.exists():raise FileExistsError('Preserve previous experiment')
    with exe.open('xb') as f:f.write(out)
    with manifest.open('x') as f:json.dump(report,f,indent=2)
    print(json.dumps({k:v for k,v in report.items() if k not in ('patches','stub_bytes')},indent=2))
