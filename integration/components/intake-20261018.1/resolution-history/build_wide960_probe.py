"""Exact-NA-executable width-only probe. Never overwrites original or output."""
from pathlib import Path
import hashlib,json,struct,sys
sys.path.insert(0,'E:/Codex/tools/pydeps')
import pefile
ROOT=Path(__file__).resolve().parent
ORIGINAL='03c20a3535d0209111ff9f3b90eba403e059df2be7e784d2746abf1535c1be6a'
SITES=[
 (0x428c89,'6820030000',1,'initial computed layout width'),
 (0x42b5b5,'c705d037710020030000',6,'mode width global initializer'),
 (0x4a6480,'c7410820030000',3,'window-object default width'),
 (0x43349d,'6820030000',1,'window size/reset caller'),
 (0x4ad86a,'6820030000',1,'secondary window reset caller'),
 (0x429888,'6820030000',1,'initial SetRect right edge'),
 (0x40117a,'c70508034f0000000040',6,'projection horizontal scale'),
]

def patch(raw):
    if hashlib.sha256(raw).hexdigest()!=ORIGINAL:raise ValueError('Requires exact original NA 1.04 executable')
    pe=pefile.PE(data=raw);output=bytearray(raw);changes=[]
    imports={i.address:i.name.decode() for d in pe.DIRECTORY_ENTRY_IMPORT for i in d.imports if i.name}
    if imports.get(0x4bd218)!='SetRect':raise ValueError('Unexpected cursor-bound import')
    allowed=set()
    for va,expected,relative,label in SITES:
        offset=pe.get_offset_from_rva(va-pe.OPTIONAL_HEADER.ImageBase)
        if raw[offset:offset+len(bytes.fromhex(expected))]!=bytes.fromhex(expected):raise ValueError(f'Instruction differs: {va:x}')
        operand=offset+relative
        replacement=struct.pack('<f',2*800/960) if va==0x40117a else struct.pack('<I',960)
        output[operand:operand+4]=replacement;allowed.update(range(operand,operand+4))
        changes.append(dict(va=hex(va),file_offset=hex(operand),label=label,before=raw[operand:operand+4].hex(),after=replacement.hex()))
    changed=[i for i,(a,b) in enumerate(zip(raw,output)) if a!=b]
    if len(output)!=len(raw) or not set(changed)<=allowed:raise AssertionError('Unexpected binary delta')
    return bytes(output),changes,changed

if __name__=='__main__':
    raw=(ROOT/'game-test/ds9dw.exe').read_bytes();out,changes,changed=patch(raw)
    exe=ROOT/'wide960-probe.exe';manifest=ROOT/'wide960-patch-v1.json'
    if exe.exists() or manifest.exists():raise FileExistsError('Preserve existing experiment outputs')
    with exe.open('xb') as f:f.write(out)
    report=dict(original_sha256=ORIGINAL,candidate_sha256=hashlib.sha256(out).hexdigest(),
        logical_size=[960,600],scene_size=[960,446],m11=struct.unpack('<f',struct.pack('<f',2*800/960))[0],
        patch_sites=changes,changed_byte_offsets=[hex(i) for i in changed],
        runtime_validated=False,limits=['Native 16:10 exploratory mode, not final 16:9','HUD artwork not reanchored','No universal layout/input correctness claimed'])
    with manifest.open('x') as f:json.dump(report,f,indent=2)
    print(json.dumps(report,indent=2))
