"""Pin complete instruction spans for the bounded original/native hook contract."""
import hashlib,json,sys
from pathlib import Path
sys.path.insert(0,'E:/Codex/tools/pydeps')
import pefile,capstone
base=Path('E:/Codex/research/dominion-rendering')
binary=base/'native-engine-2560x1440-01/ds9dw.exe'
raw=binary.read_bytes()
assert hashlib.sha256(raw).hexdigest()=='de3e3ede7a9b4ce65a891201547f36413f3a880e4df3f28fb8a81e51f687abc6'
image=pefile.PE(data=raw)
decoder=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
specs=[(0x41DA93,'point_before',5),(0x41DCFC,'point_restore',6),
       (0x4590C0,'radar_pass',5),(0x4A5E00,'primitive_admission',5),
       (0x4A5F10,'indexed_admission',5),(0x4A5610,'queue_reset',5),
       (0x402880,'native_scene_pass',5),(0x45A06D,'radar_cx_x',7),
       (0x45A086,'radar_cx_y',7),(0x45A138,'radar_edx_x',7),
       (0x45A13F,'radar_ecx_y',7),(0x45A258,'radar_ax_x',6),
       (0x45A270,'radar_ax_y',6),(0x45A3A0,'radar_pair',14)]
rows=[]
for va,name,minimum in specs:
    offset=image.get_offset_from_rva(va-image.OPTIONAL_HEADER.ImageBase)
    span=bytearray();instructions=[]
    for ins in decoder.disasm(raw[offset:offset+24],va):
        span.extend(ins.bytes);instructions.append(dict(va=hex(ins.address),bytes=ins.bytes.hex(),instruction=ins.mnemonic+' '+ins.op_str))
        if len(span)>=minimum:break
    assert minimum<=len(span)<=16
    rows.append(dict(va=va,name=name,size=len(span),bytes=span.hex(),instructions=instructions))
folder=base/'hud-assembly-work-01/DDrawCompat/Research/HudAssembly'
lines=['#pragma once','namespace HudAssembly { struct HookSite {uint32_t va;const char* name;unsigned size;unsigned char bytes[16];};','inline constexpr HookSite sites[]={']
for r in rows:lines.append('{0x%X,"%s",%d,{%s}},'%(r['va'],r['name'],r['size'],','.join('0x'+r['bytes'][i:i+2] for i in range(0,2*r['size'],2))))
lines+=['};','}']
dest=folder/'Sites.h'
assert not dest.exists()
dest.write_text('\n'.join(lines)+'\n')
out=base/'hud-assembly-hook-sites-01.json'
assert not out.exists()
out.write_text(json.dumps(dict(binary=str(binary),sha256=hashlib.sha256(raw).hexdigest(),sites=rows),indent=2)+'\n')
print(json.dumps(dict(sites=len(rows),header=str(dest))))
