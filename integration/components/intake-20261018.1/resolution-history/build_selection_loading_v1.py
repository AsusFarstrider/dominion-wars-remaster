"""Source-level repaint and loading update origins; preserve fleet consolidation."""
import hashlib,json,struct,sys
from pathlib import Path
sys.path.insert(0,'E:/Codex/tools/pydeps')
import pefile
ROOT=Path(__file__).resolve().parent
BASE='12779af0b0e41807d58eafec6d6724756f32d164268e3475de79e4ea7d401ebe'
ORIGIN=0x5f7ee0
def branch(op,src,dst):return bytes([op])+struct.pack('<i',dst-src-5)

def build(raw, corrected_repaint=False, include_loading=True):
    if hashlib.sha256(raw).hexdigest()!=BASE:raise ValueError('Exact fleet-origin image required')
    p=pefile.PE(data=raw);s=p.sections[-1];used=s.Misc_VirtualSize
    if s.Name.rstrip(b'\0')!=b'.dwui':raise ValueError('Unexpected code section')
    start=0x400000+s.VirtualAddress+used;offset=s.PointerToRawData+used
    out=bytearray(raw);stubs=bytearray();patches=[]
    def patch(va,expected,code,label):
        pos=p.get_offset_from_rva(va-0x400000);expected=bytes.fromhex(expected)
        if raw[pos:pos+len(expected)]!=expected:raise ValueError(hex(va))
        stub=start+len(stubs);stubs.extend(code(stub))
        out[pos:pos+len(expected)]=branch(0xe9,va,stub)+b'\x90'*(len(expected)-5)
        patches.append(dict(va=hex(va),before=expected.hex(),stub_va=hex(stub),label=label))
    def repaint(va):
        # Full target repaint before centered background, preserving all state.
        b=bytearray.fromhex('9c60a1649ba3008b006a0050' if corrected_repaint
                            else '9c606a00ff35649ba300')
        b+=branch(0xe8,va+len(b),0x4a1710)
        b+=bytes.fromhex('83c408619da1ec387100')
        return b+branch(0xe9,va+len(b),0x472635)
    patch(0x472630,'a1ec387100',repaint,'selection root full-surface repaint before artwork')
    def background(va):
        # Original preceding PUSH 0 is Y. Replace X=0 with signed root X,
        # preserving EAX/flags, and replay PUSH loading-widget.
        b=bytearray.fromhex('500fbf05')+struct.pack('<I',ORIGIN)
        b+=bytes.fromhex('8704246838a74d00')
        return b+branch(0xe9,va+len(b),0x434570)
    if include_loading:patch(0x434569,'6a006838a74d00',background,'loading background parent X from UI root')
    def text(va):
        b=bytearray.fromhex('8b1540a74d009c500fbf05')+struct.pack('<I',ORIGIN)
        b+=bytes.fromhex('01c2589d')
        return b+branch(0xe9,va+len(b),0x4345a9)
    if include_loading:patch(0x4345a3,'8b1540a74d00',text,'loading text absolute X from local widget plus root')
    def bar(va):
        b=bytearray.fromhex('0fbf3540a74d009c500fbf05')+struct.pack('<I',ORIGIN)
        b+=bytes.fromhex('01c6589d')
        return b+branch(0xe9,va+len(b),0x43462c)
    if include_loading:patch(0x434625,'0fbf3540a74d00',bar,'loading bar and clipping share absolute X')
    if used+len(stubs)>s.SizeOfRawData or any(raw[offset:offset+len(stubs)]):raise ValueError('No code capacity')
    out[offset:offset+len(stubs)]=stubs
    struct.pack_into('<I',out,s.get_file_offset()+8,used+len(stubs))
    return bytes(out),dict(base_sha256=BASE,candidate_sha256=hashlib.sha256(out).hexdigest(),
        patches=patches,stub_va=hex(start),stub_bytes=stubs.hex(),runtime_validated=False,
        corrected_repaint=corrected_repaint,includes_loading_changes=include_loading,
        provenance={'selection_root':'0x4e5050, callback 0x472630, resource 0x801',
                    'loading_widget':'0x4da738, normal callback 0x434700, direct update 0x434560'},
        limitations=['Normal loading callback remains parent-relative and unchanged.',
                     'Other menus may need margin repaint; not a global clear hook.',
                     'Source paths statically corroborated; user visual validation required.'])

if __name__=='__main__':
    data,r=build((ROOT/'game-fleet-origin-v1/ds9dw.exe').read_bytes())
    if (ROOT/'selection-loading-probe-v1.exe').exists() or (ROOT/'selection-loading-patch-v1.json').exists():raise FileExistsError('Preserve artifacts')
    with (ROOT/'selection-loading-probe-v1.exe').open('xb') as f:f.write(data)
    with (ROOT/'selection-loading-patch-v1.json').open('x') as f:json.dump(r,f,indent=2)
    print(r['candidate_sha256'])
