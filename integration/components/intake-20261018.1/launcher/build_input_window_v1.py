"""Private, hash-gated mouse recenter patch. Never changes desktop metrics."""
import hashlib,json,struct,sys
from pathlib import Path
sys.path.insert(0,'E:/Codex/tools/pydeps')
import pefile
ROOT=Path(__file__).resolve().parent
BASE='827d3183e25c542dda3fccc0d2261e532065c155af4dceed0a46f59b2d377357'
SOURCE=Path('E:/Codex/research/dominion-resolution/game-no-desktop-switch-v1/ds9dw.exe')

def jump(src,dst):return b'\xe9'+struct.pack('<i',dst-src-5)

def build(raw, hwnd_address=0x713de8):
 if hashlib.sha256(raw).hexdigest()!=BASE:raise ValueError('Exact no-switch control required')
 pe=pefile.PE(data=raw)
 imports={i.address:i.name for d in pe.DIRECTORY_ENTRY_IMPORT for i in d.imports if i.name}
 if imports.get(0x4bd278)!=b'GetClientRect':raise ValueError('Unexpected import')
 section=next(s for s in pe.sections if s.Name.rstrip(b'\0')==b'.dwui')
 used=section.Misc_VirtualSize;offset=section.PointerToRawData+used
 va=0x400000+section.VirtualAddress+used
 site=0x49e5ee;before=bytes.fromhex('66a1e49eab00668b0de69eab00')
 hook=pe.get_offset_from_rva(site-0x400000)
 if raw[hook:hook+len(before)]!=before:raise ValueError('Unexpected recenter load')
 # Preserve all registers/flags; API writes only a private stack RECT.
 code=bytearray.fromhex('9c6083ec108d042450ff35e83d7100ff1578d24b0085c0')
 struct.pack_into('<I',code,11,hwnd_address)
 branches=[]
 def done_if(h):
  code.extend(bytes.fromhex(h));branches.append(len(code));code.extend(b'\0'*4)
 done_if('0f84')
 code.extend(bytes.fromhex('8b4424082b04248b54240c2b54240483f801'));done_if('0f8c')
 code.extend(bytes.fromhex('3dff7f0000'));done_if('0f8f')
 code.extend(bytes.fromhex('83fa01'));done_if('0f8c')
 code.extend(bytes.fromhex('81faff7f0000'));done_if('0f8f')
 code.extend(bytes.fromhex('66a3e49eab00668915e69eab00'))
 done=len(code)
 for pos in branches:struct.pack_into('<i',code,pos,done-pos-4)
 code.extend(bytes.fromhex('83c410619d'));code.extend(before)
 code.extend(jump(va+len(code),site+len(before)))
 if used+len(code)>section.SizeOfRawData or any(raw[offset:offset+len(code)]):raise ValueError('No code capacity')
 out=bytearray(raw);out[hook:hook+len(before)]=jump(site,va)+b'\x90'*(len(before)-5)
 out[offset:offset+len(code)]=code
 struct.pack_into('<I',out,section.get_file_offset()+8,used+len(code))
 return bytes(out),dict(base_sha256=BASE,candidate_sha256=hashlib.sha256(out).hexdigest(),
  hook_va=hex(site),displaced_bytes=before.hex(),stub_va=hex(va),stub_bytes=code.hex(),hwnd_address=hex(hwnd_address),
  runtime_validated=False,scope='Refresh mouse-recenter cache from GetClientRect at recenter use',
  limitations=['Original screen-coordinate warp assumes client origin at desktop zero; nonzero-origin windows not handled here.',
               'Invalid HWND/API failure retains original cache; no desktop or rendering dimensions changed.'])

if __name__=='__main__':
 out,report=build(SOURCE.read_bytes())
 paths=(ROOT/'input-window-probe-v1.exe',ROOT/'input-window-patch-v1.json')
 if any(p.exists() for p in paths):raise FileExistsError('Preserve artifacts')
 with paths[0].open('xb') as f:f.write(out)
 with paths[1].open('x') as f:json.dump(report,f,indent=2)
 print(report['candidate_sha256'])
