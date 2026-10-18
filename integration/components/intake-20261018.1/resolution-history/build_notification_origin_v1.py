"""Give the legacy notification renderer the existing UI-canvas origin.

No global text-hook correction: both calls belong to renderer 494FC0.
The supplied rectangle is copied, translated once, and never modified in place.
"""
import hashlib
import json
import struct
import sys
from pathlib import Path
sys.path.insert(0,'E:/Codex/tools/pydeps')
import pefile

ROOT=Path(__file__).resolve().parent
BASE='a2abc1208cdf16bd797e568eb0dde60fd40c66cf77091514ec3e23ed28d09240'
UI_ORIGIN_X=0x5f7ee0
TEXT_HELPER=0x41e7f0
SITES=(0x495126,0x4951cb)


def call(src,dst): return b'\xe8'+struct.pack('<i',dst-src-5)


def build(raw):
    if hashlib.sha256(raw).hexdigest()!=BASE:
        raise ValueError('Exact non-observer fleet-middle image required')
    pe=pefile.PE(data=raw)
    section=pe.sections[-1]
    if section.Name.rstrip(b'\0')!=b'.dwui': raise ValueError('Unexpected code section')
    used=section.Misc_VirtualSize
    va=0x400000+section.VirtualAddress+used
    offset=section.PointerToRawData+used
    # cdecl(text, rectangle, lineHeight, alignment, firstLine).
    # Use a local eight-byte rectangle, keeping caller memory untouched.
    code=bytearray.fromhex('558bec83ec088b450c8b108955f88b50048955fc')
    code+=bytes.fromhex('0fbf05')+struct.pack('<I',UI_ORIGIN_X)
    code+=bytes.fromhex('660145f8ff7518ff7514ff75108d45f850ff7508')
    code+=call(va+len(code),TEXT_HELPER)
    code+=bytes.fromhex('8d65005dc3')
    if used+len(code)>section.SizeOfRawData or any(raw[offset:offset+len(code)]):
        raise ValueError('No unused code capacity')
    out=bytearray(raw)
    patches=[]
    for site in SITES:
        pos=pe.get_offset_from_rva(site-0x400000)
        before=call(site,TEXT_HELPER)
        if raw[pos:pos+5]!=before: raise ValueError('Notification call changed')
        after=call(site,va)
        out[pos:pos+5]=after
        patches.append(dict(va=hex(site),before=before.hex(),after=after.hex()))
    out[offset:offset+len(code)]=code
    struct.pack_into('<I',out,section.get_file_offset()+8,used+len(code))
    return bytes(out),dict(base_sha256=BASE,candidate_sha256=hashlib.sha256(out).hexdigest(),
        patches=patches,stub_va=hex(va),stub_bytes=code.hex(),
        ui_origin_x=hex(UI_ORIGIN_X),owner_renderer='0x494fc0',
        runtime_validated=False,
        scope='Notification renderer only; existing generic text helper unchanged',
        limitations=['Secondary notification path not captured by v4 top-90 filter.',
                     'Other direct-draw renderer contexts still require ownership mapping.',
                     'No existing offsets removed; no established overlap with these two calls.'])


if __name__=='__main__':
    candidate,report=build((ROOT/'game-fleet-middle-v1/ds9dw.exe').read_bytes())
    paths=(ROOT/'notification-origin-probe-v1.exe',ROOT/'notification-origin-patch-v1.json')
    if any(p.exists() for p in paths): raise FileExistsError('Preserve previous artifacts')
    with paths[0].open('xb') as f:f.write(candidate)
    with paths[1].open('x') as f:json.dump(report,f,indent=2)
    print(report['candidate_sha256'])
