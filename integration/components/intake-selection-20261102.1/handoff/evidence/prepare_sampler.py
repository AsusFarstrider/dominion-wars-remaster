"""Extend the existing read-only state sampler with physical client/engine sizes."""
import hashlib,json
from pathlib import Path
here=Path(__file__).resolve().parent
parent=here.parent/'esc-menu-fix/sample_state.py'
s=parent.read_text()
anchor='count=0;changes_after_limit=0;previous=None;started=time.monotonic()'
assert s.count(anchor)==1
s=s.replace(anchor,'''# Awareness belongs ONLY to this external sampler process. Never changes game DPI.
u=C.WinDLL('user32',use_last_error=True)
u.SetProcessDpiAwarenessContext.argtypes=[C.c_void_p];u.SetProcessDpiAwarenessContext.restype=W.BOOL
sampler_physical=bool(u.SetProcessDpiAwarenessContext(C.c_void_p(-4)))
u.GetClientRect.argtypes=[W.HWND,C.POINTER(W.RECT)];u.GetClientRect.restype=W.BOOL
u.ClientToScreen.argtypes=[W.HWND,C.POINTER(W.POINT)];u.ClientToScreen.restype=W.BOOL
u.GetDpiForWindow.argtypes=[W.HWND];u.GetDpiForWindow.restype=W.UINT
def screen():
 b=read(0xA38770,0x88)
 if not b:return None
 words=struct.unpack('<34I',b);context=words[12];mode=u32(context+0x14) if context else None
 modebytes=read(mode,24);hwnd=words[13];rect=W.RECT();origin=W.POINT()
 realized=bool(hwnd and u.GetClientRect(hwnd,C.byref(rect)) and u.ClientToScreen(hwnd,C.byref(origin)))
 holder=u32(0xA39B64);outer=u32(holder) if holder else None;native=u32(outer+0x20) if outer else None;image=read(native,24)
 return dict(screen_wh=list(words[2:4]),bpp=words[4],selected_mode_words=list(struct.unpack('<6I',modebytes)) if modebytes else None,hwnd=hwnd,primary_surface=words[16],active_draw_surface=words[17],draw0=words[18],active_depth=words[25],depth0=words[26],client_wh=[rect.right-rect.left,rect.bottom-rect.top] if realized else None,client_origin=[origin.x,origin.y] if realized else None,sampler_pmv2=sampler_physical,game_window_dpi=u.GetDpiForWindow(hwnd) if hwnd else None,native_image_wh=list(struct.unpack_from('<HH',image,10)) if image else None,logical_wh=list(struct.unpack('<2I',read(0x5FF670,8))) if read(0x5FF670,8) else None)
'''+anchor)
s=s.replace('   v={name:u32(address) for name,address in addresses.items()}','   v={name:u32(address) for name,address in addresses.items()};v["dimensions"]=screen()')
s=s.replace('hardware_or_pixel_evidence=False','pixel_evidence=False,actual_process_and_window_dimensions=True')
out=here/'sample_output.py';out.write_text(s)
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
(here/'sampler-provenance.json').write_text(json.dumps(dict(parent=str(parent),parent_sha256=sha(parent),sampler_sha256=sha(out),scope='Read-only process/window metadata; awareness change is sampler-only. No game writes, injected hooks, pixel capture, or display changes.'),indent=2)+'\n')
print(out)
