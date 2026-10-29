"""Convert exact user v7 PNG states to lossless bounded RGBA banks; no art edits."""
import hashlib,json,struct,subprocess
from pathlib import Path
root=Path(__file__).resolve().parents[2]
intake=root/'cinematic-extraction/original-buttons-v7-intake'
source=intake/'DW_Menu_Hover_Gentle_Pulse_v7_Complete/assets'
out=Path('E:/Codex/research/dominion-rendering/main-button-assets-v7')
ffmpeg=Path('C:/Program Files/Virtual Desktop Streamer/ffmpeg.exe')
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
assert not out.exists();assert sha(intake/'DW_Menu_Hover_Gentle_Pulse_v7_Complete.zip')=='d4369bacfe8baa4aa92adaf9a67a700b880ad9b9e9b55b4c1589eb8a42d95b63'
assert sha(ffmpeg)=='b2495c9ab7bec4ba5f9e34fcabc70f778cdb70730ed32b627b9f51f6c7f384ad'
validation=json.loads((intake/'validation-independent.json').read_text());assert validation['final_sprite_count']==210 and validation['loop_boundary_pixels_equal']
expected={Path(r['path']).as_posix():r['sha256'] for r in validation['records']}
out.mkdir();banks=[];bytes_per_frame=720*400*4
rows=[('new_game',0x4E4850,0x202,12),('load_game',0x4E48E0,0x26B,12),('multiplayer',0x4E4970,0x225,11),('preferences',0x4E4A00,0x28E,12),('credits',0x4E4A90,0x248,12),('quit',0x4E4B20,0x2B1,12)]
for slug,node,base,restart in rows:
 files=[source/slug/'idle.png']+[source/slug/'hover'/f'hover_{n:02d}.png' for n in range(25)]+[source/slug/'activate'/f'activate_{n:02d}.png' for n in range(9)]
 for p in files:assert sha(p)==expected[p.relative_to(intake).as_posix()],p
 chunks=[];commands=[]
 for name,count,pattern in [('idle',1,str(source/slug/'idle.png')),('hover',25,str(source/slug/'hover/hover_%02d.png')),('activate',9,str(source/slug/'activate/activate_%02d.png'))]:
  command=[str(ffmpeg),'-nostdin','-v','error']
  if count>1:command+=['-start_number','0']
  command+=['-i',pattern,'-frames:v',str(count),'-f','rawvideo','-pix_fmt','rgba','pipe:1']
  result=subprocess.run(command,capture_output=True,timeout=90);assert result.returncode==0,result.stderr.decode(errors='replace')
  assert len(result.stdout)==count*bytes_per_frame,(slug,name,len(result.stdout));chunks.append(result.stdout);commands.append(command)
 data=b''.join(chunks);assert data[(1+restart)*bytes_per_frame:(2+restart)*bytes_per_frame]==data[25*bytes_per_frame:26*bytes_per_frame],slug
 payload=b'DWBANK1\0'+struct.pack('<III',720,400,35)+struct.pack('<35I',*range(base,base+35))+data
 dest=out/(slug+'.dwbank');dest.write_bytes(payload)
 banks.append(dict(name=slug,node=node,idle_resource=base,hover_wrap_start=restart,path=str(dest),sha256=sha(dest),bytes=len(payload),
  frames=[dict(resource=base+n,source=str(p),source_sha256=sha(p),rgba_sha256=hashlib.sha256(data[n*bytes_per_frame:(n+1)*bytes_per_frame]).hexdigest()) for n,p in enumerate(files)],commands=commands))
 print(json.dumps(dict(bank=slug,bytes=len(payload),sha256=sha(dest))),flush=True)
manifest=dict(schema='quantum-main-buttons-source-1',archive_sha256=sha(intake/'DW_Menu_Hover_Gentle_Pulse_v7_Complete.zip'),
 validation_sha256=sha(intake/'validation-independent.json'),converter_sha256=sha(Path(__file__)),decoder_sha256=sha(ffmpeg),
 format='DWBANK1NUL;U32LEwidth,height,count;count*U32LEoriginalResource;count*tightlypackedstraightRGBA8 frames',
 transform='PNG RGBA decode only; no resize/color quantization/dither/art regeneration; explicit alpha retained',banks=banks,
 original_layout=[180,100],source_density=[720,400],original_timers=True,original_hitboxes=True,game_tested=False)
(out/'assets.json').write_text(json.dumps(manifest,indent=2)+'\n')
print(json.dumps(dict(output=str(out),banks=len(banks),frames=sum(len(b['frames']) for b in banks),total_bytes=sum(b['bytes'] for b in banks))))
