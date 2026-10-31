"""Bounded external read-only game-state sampler; no injected hooks or writes."""
import argparse,ctypes as C,hashlib,json,os,struct,time
from ctypes import wintypes as W
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--pid',type=int,required=True);p.add_argument('--game',required=True);p.add_argument('--out',required=True);a=p.parse_args()
k=C.WinDLL('kernel32',use_last_error=True)
k.OpenProcess.argtypes=[W.DWORD,W.BOOL,W.DWORD];k.OpenProcess.restype=W.HANDLE
k.ReadProcessMemory.argtypes=[W.HANDLE,C.c_void_p,C.c_void_p,C.c_size_t,C.POINTER(C.c_size_t)];k.ReadProcessMemory.restype=W.BOOL
k.QueryFullProcessImageNameW.argtypes=[W.HANDLE,W.DWORD,W.LPWSTR,C.POINTER(W.DWORD)];k.QueryFullProcessImageNameW.restype=W.BOOL
k.GetExitCodeProcess.argtypes=[W.HANDLE,C.POINTER(W.DWORD)];k.GetExitCodeProcess.restype=W.BOOL
k.CloseHandle.argtypes=[W.HANDLE]
h=k.OpenProcess(0x1000|0x10,False,a.pid)
if not h:raise OSError(C.get_last_error(),'OpenProcess read-only')
buf=C.create_unicode_buffer(32768);size=W.DWORD(len(buf));assert k.QueryFullProcessImageNameW(h,0,buf,C.byref(size))
game=Path(a.game).resolve();assert Path(buf.value).resolve()==game/'ds9dw.exe'
digest=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
out=Path(a.out);out.mkdir(parents=True,exist_ok=False)
def read(address,n):
 b=C.create_string_buffer(n);got=C.c_size_t()
 return b.raw if address and k.ReadProcessMemory(h,C.c_void_p(address),b,n,C.byref(got)) and got.value==n else None
def u32(address):
 b=read(address,4);return struct.unpack('<I',b)[0] if b else None
def node(address):
 b=read(address,0x90)
 if not b:return None
 u=lambda off:struct.unpack_from('<I',b,off)[0]
 return dict(address=address,id=u(0),flags=u(4),xywh=list(struct.unpack_from('<hhHH',b,8)),input=u(0x48),draw=u(0x4c),next=u(0x6b),previous=u(0x6f),child=u(0x73),parent=u(0x77),data=u(0x50),counter=u(0x54))
addresses={'state':0x4D0A24,'pending':0x4D0A28,'previous_state':0x4D0A20,'input':0x713834,'secondary':0x713838,'draw':0x71383c,'scene':0x713840,'pre':0x5F7ED4,'post':0x5F7ED0,'mode':0x713DE8,'holder':0xA39B64,'surface_alias':0xA387B4,'captured':0x5F7F6C,'consumed':0x5F7F94,'mission_mode':0x718690,'escape':0x7180B6}
count=0;changes_after_limit=0;previous=None;started=time.monotonic()
with (out/'events.ndjson').open('x',encoding='utf-8') as f:
 def emit(event):
  f.write(json.dumps(event,separators=(',',':'))+'\n');f.flush();os.fsync(f.fileno())
 emit(dict(kind='start',pid=a.pid,executable=str(game/'ds9dw.exe'),executable_sha256=digest(game/'ds9dw.exe'),wrapper_sha256=digest(game/'ddraw.dll'),version_sha256=digest(game/'version.json'),config_sha256=digest(game/'Remaster.Rendering.json'),read_only=True,period_ms=200,max_snapshots=256))
 try:
  while time.monotonic()-started<1800:
   exitcode=W.DWORD();assert k.GetExitCodeProcess(h,C.byref(exitcode))
   if exitcode.value!=259:emit(dict(kind='process_exit',exit_code=exitcode.value));break
   v={name:u32(address) for name,address in addresses.items()}
   global_node=node(0x5F7ED8);root=node(global_node['child']) if global_node else None
   outer=u32(v['holder']) if v['holder'] else None;ob=read(outer,0x50);native=u32(outer+0x20) if outer else None;nb=read(native,0x28)
   target=dict(outer=outer,native=native,outer_hex=ob.hex() if ob else None,native_hex=nb.hex() if nb else None)
   # Ignore cached pixels/pitch and animation counters for change detection;
   # each retained snapshot still includes the raw descriptors and full nodes.
   stable_node=lambda n:{name:n[name] for name in ('address','id','flags','xywh','input','draw','next','previous','child','parent')} if n else None
   key=json.dumps([v,stable_node(global_node),stable_node(root),outer,native,ob[0x10:0x14].hex() if ob else None,nb[:16].hex() if nb else None],sort_keys=True)
   if key!=previous:
    previous=key
    if count<256:
     count+=1;emit(dict(kind='snapshot',sequence=count,elapsed=round(time.monotonic()-started,3),values=v,global_node=global_node,root=root,target=target,main=node(0x4E47C0),buttons=[node(0x4E4850+i*0x90) for i in range(6)]))
     print(json.dumps(dict(sequence=count,state=v['state'],pending=v['pending'],pre=v['pre'],post=v['post'],root=root['address'] if root else None)),flush=True)
    else:changes_after_limit+=1
   time.sleep(.2)
  else:emit(dict(kind='time_limit',seconds=1800))
 except BaseException as e:
  emit(dict(kind='sampler_error',error=repr(e)));raise
 finally:
  emit(dict(kind='sampler_end',snapshots=count,changes_after_limit=changes_after_limit,hardware_or_pixel_evidence=False));k.CloseHandle(h)
