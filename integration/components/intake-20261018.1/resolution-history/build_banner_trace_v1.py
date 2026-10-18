"""Bounded, non-mutating font-call observation on latest fleet layout image."""
import hashlib,json,struct,sys
from pathlib import Path
sys.path.insert(0,'E:/Codex/tools/pydeps')
import pefile
ROOT=Path(__file__).resolve().parent
BASE='a2abc1208cdf16bd797e568eb0dde60fd40c66cf77091514ec3e23ed28d09240'
def align(x,a):return (x+a-1)//a*a
def rel(src,dst):return b'\xe9'+struct.pack('<i',dst-src-5)
def build(raw,broad=False):
    if hashlib.sha256(raw).hexdigest()!=BASE:raise ValueError('Exact fleet-middle image required')
    p=pefile.PE(data=raw);s=p.sections[-1];base=p.OPTIONAL_HEADER.ImageBase
    if base!=0x400000 or p.OPTIONAL_HEADER.DllCharacteristics&0x40:raise ValueError('Fixed-base image required')
    header=s.get_file_offset()+40
    if header+40>p.OPTIONAL_HEADER.SizeOfHeaders or any(raw[header:header+40]):raise ValueError('No header capacity')
    data_rva=align(p.OPTIONAL_HEADER.SizeOfImage,p.OPTIONAL_HEADER.SectionAlignment)
    data_va=base+data_rva;rawpos=align(len(raw),p.OPTIONAL_HEADER.FileAlignment)
    used=s.Misc_VirtualSize;stub_va=base+s.VirtualAddress+used;stub_offset=s.PointerToRawData+used
    code=bytearray.fromhex('9c608b35b8c34e0085f6');branches=[]
    def emit(b):code.extend(bytes.fromhex(b))
    def done_if(op):
        code.extend(bytes.fromhex(op));branches.append(len(code));code.extend(b'\0'*4)
    done_if('0f84')
    if not broad:
        emit('66817e0c9001');done_if('0f85') # Exact X400 lead, not a correction.
    emit('66837e0e00');done_if('0f8c')
    emit('66837e0e5a');done_if('0f8f') # Top90 logical pixels only.
    capacity=256 if broad else 64
    if broad:
        # Retain distinct caller/XY combinations instead of filling on repeated
        # menu frames. Never dereference text inside the game observer.
        code.extend(b'\x8b\x0d'+struct.pack('<I',data_va+8))
        emit('85c9');empty_pos=len(code);emit('0f8400000000')
        emit('8b5424248b5e0c');code.extend(b'\xbf'+struct.pack('<I',data_va+16))
        loop=len(code)
        emit('3917');different=len(code);emit('0f8500000000')
        emit('395f04');done_if('0f84')
        next_record=len(code)
        emit('83c72049');code.extend(b'\x0f\x85'+struct.pack('<i',loop-(len(code)+6)))
        allocate=len(code)
        struct.pack_into('<i',code,empty_pos+2,allocate-(empty_pos+6))
        struct.pack_into('<i',code,different+2,next_record-(different+6))
    code.extend(b'\xa1'+struct.pack('<I',data_va+8))
    if broad:emit('3d00010000')
    else:emit('83f840')
    done_if('0f83') # Fixed record cap, no unbounded logging.
    emit('c1e005');code.extend(b'\x8d\xb8'+struct.pack('<I',data_va+16))
    emit('8b4424248907') # Original caller return address, after PUSHFD/PUSHAD.
    emit('8b460c894704') # Packed signed16 X,Y.
    emit('8b06894708') # Font resource pointer.
    emit('8b44242c89470c') # Text argument pointer (not dereferenced in hook).
    emit('897710') # Font-state pointer.
    code.extend(b'\xa1'+struct.pack('<I',0xa39b78));emit('894714')
    emit('8b442428894718') # Surface argument pointer.
    code.extend(b'\xff\x05'+struct.pack('<I',data_va+8)) # Publish after writes.
    done=len(code)
    for pos in branches:struct.pack_into('<i',code,pos,done-(pos+4))
    emit('619d83ec34a1789ba300') # Restore state, replay exact displaced bytes.
    code.extend(rel(stub_va+len(code),0x4a2658))
    if used+len(code)>s.SizeOfRawData or any(raw[stub_offset:stub_offset+len(code)]):raise ValueError('No stub capacity')
    hook=p.get_offset_from_rva(0xa2650);before=bytes.fromhex('83ec34a1789ba300')
    if raw[hook:hook+8]!=before:raise ValueError('Unexpected font entry')
    out=bytearray(raw);out[hook:hook+8]=rel(0x4a2650,stub_va)+b'\x90'*3
    out[stub_offset:stub_offset+len(code)]=code
    struct.pack_into('<I',out,s.get_file_offset()+8,used+len(code))
    rawsize=align(16+capacity*32,p.OPTIONAL_HEADER.FileAlignment)
    out.extend(b'\0'*(rawpos+rawsize-len(out)))
    struct.pack_into('<IIII',out,rawpos,0x44575458,1,0,capacity)
    struct.pack_into('<8sIIIIIIHHI',out,header,b'.dwobs\0\0',16+capacity*32,data_rva,rawsize,rawpos,0,0,0,0,0xc0000040)
    struct.pack_into('<H',out,p.FILE_HEADER.get_field_absolute_offset('NumberOfSections'),p.FILE_HEADER.NumberOfSections+1)
    struct.pack_into('<I',out,p.OPTIONAL_HEADER.get_field_absolute_offset('SizeOfImage'),align(data_rva+16+capacity*32,p.OPTIONAL_HEADER.SectionAlignment))
    struct.pack_into('<I',out,p.OPTIONAL_HEADER.get_field_absolute_offset('SizeOfInitializedData'),p.OPTIONAL_HEADER.SizeOfInitializedData+rawsize)
    struct.pack_into('<I',out,p.OPTIONAL_HEADER.get_field_absolute_offset('CheckSum'),0)
    return bytes(out),dict(base_sha256=BASE,candidate_sha256=hashlib.sha256(out).hexdigest(),
        data_va=hex(data_va),capacity=capacity,record_size=32,hook_file_offset=hook,
        stub_va=hex(stub_va),stub_bytes=code.hex(),stub_file_offset=stub_offset,
        filter=('Any X, Y0..90; unique caller/XY' if broad else 'X400, Y0..90')+'; no rendering adjustment',runtime_validated=False)
if __name__=='__main__':
    out,r=build((ROOT/'game-fleet-middle-v1/ds9dw.exe').read_bytes())
    exe=ROOT/'banner-trace-probe-v1.exe';manifest=ROOT/'banner-trace-patch-v1.json'
    if exe.exists() or manifest.exists():raise FileExistsError('Preserve artifacts')
    with exe.open('xb') as f:f.write(out)
    with manifest.open('x') as f:json.dump(r,f,indent=2)
    print(r['candidate_sha256']);print(r['data_va'])
