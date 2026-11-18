"""Losslessly decode immutable PNGs, then quantize once to native RGB565.

No image resizing or semantic artwork editing. All sampling occurs at runtime
from the declared source dimensions. Private images are never source-bundled.
"""
import array,hashlib,json,struct,subprocess,shutil,sys
from pathlib import Path
base=Path('E:/Codex/research/dominion-rendering');out=base/'opening-card-assets-v1'
ffmpeg=Path('C:/Program Files/Virtual Desktop Streamer/ffmpeg.exe')
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
assert not out.exists();out.mkdir()
assert sha(ffmpeg)=='b2495c9ab7bec4ba5f9e34fcabc70f778cdb70730ed32b627b9f51f6c7f384ad'
assets=[('title',Path('C:/Users/Example/Downloads/Dominion_Wars_Title_Splash_2560x1440.png'),
 '9a54e4a22469b356ada3b40e56e76b9cd48226cb03ecfc31724737ffb7203ff2'),
 ('publisher',Path('C:/Users/Example/Downloads/Dominion_Wars_Publisher_Card_1440p_faithful.png'),
 '5f609b34886f8359831ceda0429a8baae135144ccda906f76834455a6b31592b')]
rows=[]
for name,src,expected in assets:
 assert sha(src)==expected
 png=src.read_bytes();assert png[:8]==b'\x89PNG\r\n\x1a\n';w,h=struct.unpack_from('>II',png,16)
 assert 0<w<=8192 and 0<h<=8192 and w*h<=16*1024*1024
 shutil.copyfile(src,out/(name+'-source.png'))
 cmd=[str(ffmpeg),'-nostdin','-v','error','-i',str(src),'-frames:v','1','-f','rawvideo','-pix_fmt','rgba','pipe:1']
 p=subprocess.run(cmd,capture_output=True,timeout=60);assert p.returncode==0,p.stderr
 raw=p.stdout;assert len(raw)==w*h*4
 alpha=raw[3::4];alpha_range=[min(alpha),max(alpha)]
 pixels=array.array('H',((((r*a+127)//255>>3)<<11)|(((g*a+127)//255>>2)<<5)|((b*a+127)//255>>3)
                    for r,g,b,a in zip(raw[0::4],raw[1::4],raw[2::4],alpha)))
 if sys.byteorder!='little':pixels.byteswap()
 dest=out/(name+'.dwcard');dest.write_bytes(b'DWCARD1\0'+struct.pack('<II',w,h)+pixels.tobytes())
 rows.append(dict(name=name,source=str(src),source_sha256=expected,width=w,height=h,alpha_range=alpha_range,
  payload=str(dest),payload_sha256=sha(dest),payload_bytes=dest.stat().st_size,
  rgba_sha256=hashlib.sha256(raw).hexdigest(),decode_command=cmd,format='DWCARD1/NUL,widthU32LE,heightU32LE,tightly packed RGB565LE',
  alpha='Composite on black with integer rounded(alpha*channel/255); opaque inputs unchanged',quantization='R>>3,G>>2,B>>3; no dither/no resizing'))
report=dict(schema='quantum-opening-card-assets-1',ffmpeg_sha256=sha(ffmpeg),converter_sha256=sha(Path(__file__)),assets=rows,
 state_resource_mapping={'5/0x101':'publisher','6/0x103':'title'},mapping_evidence='cinematic-extraction/remaining-opening-v1/inventory.json + original Engine constructor/root identities')
(out/'assets.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report,indent=2))
