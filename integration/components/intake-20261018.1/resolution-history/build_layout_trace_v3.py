"""Observe rectangle producers, not final font positions. No layout correction."""
import hashlib
import json
import struct
from pathlib import Path
from build_banner_trace_v1 import build as scaffold, rel
import pefile

ROOT = Path(__file__).resolve().parent


def build(raw, unique=False):
    image, metadata = scaffold(raw, broad=True)
    out = bytearray(image)
    pe = pefile.PE(data=image)
    stub = metadata['stub_file_offset']
    va = int(metadata['stub_va'], 16)
    data = int(metadata['data_va'], 16)
    # Remove the font-entry observation; observe the rectangle helper instead.
    out[metadata['hook_file_offset']:metadata['hook_file_offset']+8] = bytes.fromhex('83ec34a1789ba300')
    code = bytearray.fromhex('9c608b74242c85f6')
    branches = []
    def emit(h): code.extend(bytes.fromhex(h))
    def done_if(h):
        emit(h)
        branches.append(len(code))
        code.extend(b'\0'*4)
    done_if('0f84')
    # Filter supplied rectangle top, not the animated final glyph X.
    emit('66837e0200'); done_if('0f8c')
    emit('66837e025a'); done_if('0f8f')
    code.extend(b'\xa1' + struct.pack('<I', data+8))
    emit('3d00010000'); done_if('0f83')
    if unique:
        # Compare stable caller/rectangle values, not transient text pointers.
        emit('89c185c9')
        empty = len(code); emit('0f8400000000')
        emit('8b5424248b1e8b6e04')
        code.extend(b'\xbf'+struct.pack('<I', data+16))
        loop = len(code)
        mismatches = []
        for comparison in ('3917', '395f04'):
            emit(comparison)
            emit('0f85'); mismatches.append(len(code)); emit('00000000')
        emit('396f08'); done_if('0f84')
        advance = len(code)
        emit('83c72049')
        code.extend(b'\x0f\x85'+struct.pack('<i',loop-len(code)-6))
        allocate = len(code)
        struct.pack_into('<i',code,empty+2,allocate-empty-6)
        for pos in mismatches: struct.pack_into('<i',code,pos,advance-pos-4)
    emit('c1e005'); code.extend(b'\x8d\xb8'+struct.pack('<I', data+16))
    emit('8b4424248907')  # Caller return.
    emit('8b068947048b4604894708')  # Rectangle XY and WH, four signed WORDs.
    emit('8b44242889470c897710')  # Text pointer and rectangle pointer.
    code.extend(b'\xa1'+struct.pack('<I',0x4ec3b8)); emit('894714')
    emit('8b4424308947188b44243489471c')  # Line height and alignment.
    code.extend(b'\xff\x05'+struct.pack('<I',data+8))
    end = len(code)
    for pos in branches: struct.pack_into('<i',code,pos,end-pos-4)
    emit('619db80c000100')
    code.extend(rel(va+len(code),0x41e7f5))
    old_size = len(bytes.fromhex(metadata['stub_bytes']))
    section = pe.sections[-2]
    if stub+len(code)>section.PointerToRawData+section.SizeOfRawData:
        raise ValueError('Observer exceeds code section capacity')
    if len(code)>old_size:
        if any(out[stub+old_size:stub+len(code)]):
            raise ValueError('Observer extension is not unused zero space')
        struct.pack_into('<I',out,section.get_file_offset()+8,
                         stub-section.PointerToRawData+len(code))
    out[stub:stub+len(bytes.fromhex(metadata['stub_bytes']))] = b'\x90'*len(bytes.fromhex(metadata['stub_bytes']))
    out[stub:stub+len(code)] = code
    hook = pe.get_offset_from_rva(0x1e7f0)
    if out[hook:hook+5] != bytes.fromhex('b80c000100'):
        raise ValueError('Unexpected rectangle helper entry')
    out[hook:hook+5] = rel(0x41e7f0,va)
    struct.pack_into('<I',out,pe.sections[-1].PointerToRawData+4,4 if unique else 3)
    return bytes(out), dict(candidate_sha256=hashlib.sha256(out).hexdigest(),
        source_sha256=hashlib.sha256(raw).hexdigest(), data_va=hex(data),
        hook_va='0x41e7f0', stub_va=hex(va), stub_bytes=code.hex(),
        capacity=256, record_size=32, schema=['caller_return','rectangle_xy',
        'rectangle_wh','text_pointer','rectangle_pointer','font_state_pointer',
        'line_height','alignment'], runtime_validated=False,
        limitations=[('First 256 distinct caller/rectangle pairs with top 0..90.' if unique
                      else 'First 256 calls with rectangle top 0..90; no deduplication.'),
                     'Pointer contents may change after capture; no text copied.',
                     'No rendering, input, display, or layout correction.'])


if __name__ == '__main__':
    candidate, report = build((ROOT/'game-fleet-middle-v1/ds9dw.exe').read_bytes())
    if (ROOT/'layout-trace-probe-v3.exe').exists() or (ROOT/'layout-trace-patch-v3.json').exists():
        raise FileExistsError('Preserve previous artifacts')
    with (ROOT/'layout-trace-probe-v3.exe').open('xb') as f: f.write(candidate)
    with (ROOT/'layout-trace-patch-v3.json').open('x') as f: json.dump(report,f,indent=2)
    print(report['candidate_sha256'])
