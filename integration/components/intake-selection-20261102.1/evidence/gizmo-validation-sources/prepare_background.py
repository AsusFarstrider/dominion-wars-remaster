"""Decode the supplied Main background without resizing; quantize once to565."""
import array,hashlib,json,shutil,struct,subprocess,sys
from pathlib import Path
src=Path('C:/Users/Example/Downloads/Dominion_Wars_Main_Menu_1440p_Restoration_v1/Dominion_Wars_Main_Menu_2560x1440_Faithful_Widescreen.png')
out=Path('E:/Codex/research/dominion-rendering/main-background-assets-v1')
ffmpeg=Path('C:/Program Files/Virtual Desktop Streamer/ffmpeg.exe')
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
assert not out.exists()
assert sha(src)=='a28f7f9df2c648dcfb81300cc5d4241d61c9a01a856d2ccbdc4e50f7749cfc1a'
assert sha(ffmpeg)=='b2495c9ab7bec4ba5f9e34fcabc70f778cdb70730ed32b627b9f51f6c7f384ad'
assert struct.unpack_from('>II',src.read_bytes(),16)==(2560,1440)
cmd=[str(ffmpeg),'-nostdin','-v','error','-i',str(src),'-frames:v','1','-f','rawvideo','-pix_fmt','rgba','pipe:1']
p=subprocess.run(cmd,capture_output=True,timeout=60);assert p.returncode==0,p.stderr
raw=p.stdout;assert len(raw)==2560*1440*4
alpha=raw[3::4];pixels=array.array('H',((((r*a+127)//255>>3)<<11)|(((g*a+127)//255>>2)<<5)|((b*a+127)//255>>3)
 for r,g,b,a in zip(raw[0::4],raw[1::4],raw[2::4],alpha)))
if sys.byteorder!='little':pixels.byteswap()
out.mkdir();shutil.copyfile(src,out/'main-source.png');dest=out/'main.dwcard'
dest.write_bytes(b'DWCARD1\0'+struct.pack('<II',2560,1440)+pixels.tobytes())
report=dict(schema='quantum-main-background-assets-1',resource='201',source=str(src),source_sha256=sha(src),width=2560,height=1440,
 alpha_range=[min(alpha),max(alpha)],payload=str(dest),payload_sha256=sha(dest),payload_bytes=dest.stat().st_size,
 rgba_sha256=hashlib.sha256(raw).hexdigest(),decode_command=cmd,ffmpeg_sha256=sha(ffmpeg),converter_sha256=sha(Path(__file__)),
 conversion='RGBA onblack thenR>>3/G>>2/B>>3, noresize/nodither',scope='Main background only; original foreground coverage/colors/layout retained separately')
(out/'assets.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))
