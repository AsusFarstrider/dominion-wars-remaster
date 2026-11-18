#pragma once
#include "SelectionRuntime.h"
#include "CursorRaster.h"
#include <Research/MenuProbe/Bridge.h>
#include <Research/MenuCanvas/ScopeUnlock.h>
#include <Common/Hook.h>
#include <intrin.h>
#include <ddraw.h>
namespace CMainCursor {
struct Target {uint32_t holder=0,outer=0,native=0,surface=0,state=0;uint16_t index=0;unsigned w=0,h=0;};
inline bool nativeSame(const Target& a,const Target& b)noexcept{return a.holder==b.holder&&a.outer==b.outer&&a.native==b.native&&a.surface==b.surface&&a.index==b.index&&a.w==b.w&&a.h==b.h;}
inline bool same(const Target& a,const Target& b)noexcept{return nativeSame(a,b)&&a.state==b.state;}
inline std::atomic<bool> available{false};inline bool drawing=false;inline DWORD owner=0;
inline uint64_t epoch=1,frame=0,draws=0,restores=0,overwrites=0;
inline std::atomic<bool> foreign{false};inline std::shared_ptr<Art> artwork;
struct Fresh {bool valid=false;Target target{};uint64_t frame=0,epoch=0;};inline Fresh fresh;
struct Token {bool valid=false;Target target{};uint32_t source=0;uint64_t frame=0,epoch=0;};inline Token token;
inline std::set<std::string> reported;
inline bool foreground(){
#ifdef MENU_CANVAS_TEST
    return true; // Host-window boundary only; no computer input in mock fixtures.
#else
    DWORD process=0;GetWindowThreadProcessId(GetForegroundWindow(),&process);return process==GetCurrentProcessId();
#endif
}
inline void note(const char* kind,const std::string& detail={})noexcept{try{if(reported.size()<24&&reported.insert(std::string(kind)+detail).second)Remaster::note(kind,detail);}catch(...){}}
inline void note(const char* kind,const char* detail)noexcept{try{note(kind,std::string(detail));}catch(...){}}
inline bool context(Target& t){
    uint32_t draw=0,scene=1;uint16_t flags=0;QImageResearch image{};
    if(!SelectionRestoration::read(0x4D0A24,t.state)||(t.state!=7&&t.state!=10)||
        !SelectionRestoration::read(0x71383C,draw)||draw!=0x42A290||!SelectionRestoration::read(0x713840,scene)||scene||
        !SelectionRestoration::read(0xA39B64,t.holder)||!SelectionRestoration::read(t.holder,t.outer)||
        !SelectionRestoration::read(t.outer,flags)||!(flags&0x8000)||!SelectionRestoration::read(t.outer+8,t.index)||t.index>=4||
        !SelectionRestoration::read(t.outer+0x20,t.native)||!SelectionRestoration::read(t.native,image)||
        !(image.flags&0x200)||image.pixel_bits!=16||!image.surface||image.origin_x||image.origin_y||!MenuCanvas::validSize(image.width,image.height))return false;
    t.surface=image.surface;t.w=image.width;t.h=image.height;return true;
}
struct Lease {
    Target target;uint64_t epoch=0;Patch pixels;bool written=false,retired=false;
    bool probe=false;std::array<int32_t,4> bookBefore{},bookExpected{};
    IDirectDrawSurface4* surface=nullptr;
    explicit Lease(Target t):target(t),surface(reinterpret_cast<IDirectDrawSurface4*>(t.surface)){surface->lpVtbl->AddRef(surface);}
    ~Lease(){surface->lpVtbl->Release(surface);}
};
struct SurfaceHold {IDirectDrawSurface4* value;explicit SurfaceHold(IDirectDrawSurface4* s):value(s){value->lpVtbl->AddRef(value);}~SurfaceHold(){value->lpVtbl->Release(value);}};
inline std::array<std::unique_ptr<Lease>,4> leases;
inline unsigned probeSamples=0;
inline std::array<int,4> probeX{},probeY{};inline std::array<bool,4> probeSeen{};
inline void probeBeforeRestore(const Lease& lease,MenuCanvas::WriteView view)noexcept {
    if(!lease.probe)return;
    try{std::array<int32_t,4> actual{};uint32_t valid=0;
        if(!SelectionRestoration::read(0x718628+lease.target.index*16,actual)||!SelectionRestoration::read(0x71869C+lease.target.index*4,valid))return;
        bool after=true,before=true;const auto& p=lease.pixels;const auto w=size_t(p.rect.right-p.rect.left),h=size_t(p.rect.bottom-p.rect.top);
        for(size_t y=0;y<h;++y)for(size_t x=0;x<w;++x){uint16_t pixel=0;memcpy(&pixel,static_cast<uint8_t*>(view.pixels)+size_t(p.rect.top+y)*view.pitch+size_t(p.rect.left+x)*2,2);
            after=after&&pixel==p.after[y*w+x];before=before&&pixel==p.before[y*w+x];}
        const bool covers=actual[0]<=p.rect.left&&actual[1]<=p.rect.top&&actual[2]>=p.rect.right&&actual[3]>=p.rect.bottom;
        auto rect=[](const auto& a){return std::to_string(a[0])+","+std::to_string(a[1])+","+std::to_string(a[2])+","+std::to_string(a[3]);};
        Remaster::note("cursor_cleanup_probe","buffer="+std::to_string(lease.target.index)+"; frame="+std::to_string(frame)+"; recorded="+rect(actual)+"; expected_original_algorithm="+rect(lease.bookExpected)+
            "; recorded_matches_algorithm="+(actual==lease.bookExpected?"yes":"no")+"; valid="+std::to_string(valid)+"; covers_clipped44="+(covers?"yes":"no")+
            "; BEFORE_our_restore: matches_drawn_patch="+(after?"yes":"no")+"; matches_saved_underlay="+(before?"yes":"no")+"; no_eraser_causality_claim");
    }catch(...){}
}
inline void boundary()noexcept {
    if(!available)return;if(GetCurrentThreadId()!=owner){foreign=true;return;}
    ++epoch;fresh={};token={};for(auto& lease:leases)if(lease)lease->retired=true;
}
inline bool view(DDSURFACEDESC2& d,const Target& t,MenuCanvas::WriteView& v){
    if(!d.lpSurface||d.lPitch<=0||d.dwWidth!=t.w||d.dwHeight!=t.h||!(d.ddpfPixelFormat.dwFlags&DDPF_RGB)||d.ddpfPixelFormat.dwRGBBitCount!=16||
        d.ddpfPixelFormat.dwRBitMask!=0xF800||d.ddpfPixelFormat.dwGBitMask!=0x07E0||d.ddpfPixelFormat.dwBBitMask!=0x001F)return false;
    v={d.lpSurface,size_t(d.lPitch)*d.dwHeight,size_t(d.lPitch),d.dwWidth,d.dwHeight};return MenuCanvas::validView(v.pixels,v.bytes,v.pitch,v.width,v.height);
}
// Called from the already-installed421F70 entry BEFORE widget producers. Never
// restore a saved menu patch over the new frame or in another state/generation.
inline void frameBegin()noexcept {
    if(!available)return;if(GetCurrentThreadId()!=owner){foreign=true;return;}
    try{if(foreign.exchange(false))boundary();++frame;fresh={};token={};Target current{};if(!context(current))return;
        auto& slot=leases[current.index];if(!slot||!slot->written||slot->retired||slot->epoch!=epoch||!same(slot->target,current))return;
        auto lease=std::move(slot);SurfaceHold hold(lease->surface);DDSURFACEDESC2 desc{};desc.dwSize=sizeof(desc);
        const auto locked=lease->surface->lpVtbl->Lock(lease->surface,nullptr,&desc,DDLOCK_WAIT,nullptr);
        if(FAILED(locked)){lease->retired=true;slot=std::move(lease);note("cursor_restore_retired","Lock failed before menu producers");return;}
        MenuCanvas::ScopeUnlock unlock([s=lease->surface]{return s->lpVtbl->Unlock(s,nullptr);});
        MenuCanvas::WriteView target{};Target now{};
        const bool admitted=view(desc,current,target)&&context(now)&&same(now,current)&&lease->epoch==epoch;
        if(admitted)probeBeforeRestore(*lease,target);
        const bool restored=admitted&&lease->pixels.apply(target,true);
        const auto unlocked=unlock.close();
        if(!restored||FAILED(unlocked)){lease->retired=true;if(!slot)slot=std::move(lease);note("cursor_restore_retired","restore/Unlock not confirmed; fresh full repaint required");}
        else {++restores;note("cursor_underlay_restored","before421F70 producers; same retained native buffer/state/generation");}
    }catch(...){boundary();note("cursor_restore_exception");}
}
inline void committed(uint32_t outer,uint32_t native,uint32_t surface,unsigned width,unsigned height)noexcept {
    if(!available||GetCurrentThreadId()!=owner)return;
    try{fresh={};const auto entryEpoch=epoch,entryFrame=frame;Target t{};
        if(!context(t)||t.outer!=outer||t.native!=native||t.surface!=surface||t.w!=width||t.h!=height||foreign.load())return;
        auto& prior=leases[t.index];if(prior&&nativeSame(prior->target,t)){auto retired=std::move(prior);retired.reset();++overwrites;note("cursor_underlay_overwritten","fresh complete native menu paint; retained patch retired");}
        Target now{};if(entryEpoch!=epoch||entryFrame!=frame||foreign.load()||!context(now)||!same(t,now))return;
        fresh={true,t,entryFrame,entryEpoch};
    }catch(...){fresh={};}
}
inline void* originals[]={reinterpret_cast<void*>(0x40F990),reinterpret_cast<void*>(0x43B241)};
inline bool replace(uint32_t caller,uint32_t source,int x,int y,bool& nativeWritten){
    if(!available||GetCurrentThreadId()!=owner||drawing||caller!=0x43B22F||!fresh.valid||fresh.frame!=frame||fresh.epoch!=epoch)return false;
    Target t{};uint32_t visibility=0,state=1,sequence=0,ordinal=1,step=1,resolved=0,campaign=0,pending=0;int16_t hx=1,hy=1,mx=0,my=0;
    uint32_t ids[2]{},tag=0,width=0,height=0;
    if(!context(t)||!same(t,fresh.target)||foreign.load()||!SelectionRestoration::read(0x4D0A28,pending)||pending!=t.state||
        !SelectionRestoration::read(0x5F7E58,visibility)||!visibility||!SelectionRestoration::read(0x5F7E64,state)||state||
        !SelectionRestoration::read(0x5F7E80,sequence)||sequence!=0x4DC5C0||!SelectionRestoration::read(sequence,ids)||ids[0]!=0xAF87||ids[1]!=0xFFFFFFFF||
        !SelectionRestoration::read(0x5F7E74,ordinal)||ordinal||!SelectionRestoration::read(0x5F7E7C,step)||step||
        !SelectionRestoration::read(0x5F7E60,hx)||!SelectionRestoration::read(0x5F7E62,hy)||hx||hy||
        !SelectionRestoration::read(0x5F7E5C,mx)||!SelectionRestoration::read(0x5F7E5E,my)||x!=mx||y!=my||
        !SelectionRestoration::read(0x5F7E68,resolved)||resolved!=source||!SelectionRestoration::read(source,tag)||tag!=0x52545854||
        !SelectionRestoration::read(source+0x28,width)||width!=22||!SelectionRestoration::read(source+0x2C,height)||height!=22||
        !SelectionRestoration::read(0x71386C,campaign))return false;
    if(!foreground())return false;
    drawing=true;struct End {~End(){drawing=false;}} end;fresh.valid=false;
    const auto drawEpoch=epoch,drawFrame=frame;
    auto next=std::make_unique<Lease>(t);next->epoch=epoch;SurfaceHold hold(next->surface);DDSURFACEDESC2 desc{};desc.dwSize=sizeof(desc);
    const auto locked=next->surface->lpVtbl->Lock(next->surface,nullptr,&desc,DDLOCK_WAIT,nullptr);if(FAILED(locked)){note("cursor_original_retained","Lock failed before any replacement write");return false;}
    MenuCanvas::ScopeUnlock unlock([s=next->surface]{return s->lpVtbl->Unlock(s,nullptr);});
    MenuCanvas::WriteView target{};Target now{};
    if(!view(desc,t,target)||!next->pixels.prepare(target,artwork->frames[campaign?1:0],x,y)||!context(now)||!same(t,now)||next->epoch!=epoch||drawFrame!=frame||foreign.load())return false;
    auto& slot=leases[t.index];auto retired=std::move(slot);retired.reset();
    // Releasing an old COM lease can reenter resource cleanup. Validate after
    // that Release and before installing the new cleanup receipt/native write.
    if(drawFrame!=frame||drawEpoch!=epoch||foreign.load()||!context(now)||!same(t,now))return false;
    slot=std::move(next); // Empty slot: no destructor/callback between check and write.
    if(!slot->pixels.apply(target)){slot.reset();return false;}
    slot->written=true;nativeWritten=true;token={true,t,source,drawFrame,drawEpoch};
    const auto unlocked=unlock.close();if(drawFrame==frame&&drawEpoch==epoch)token={true,t,source,drawFrame,drawEpoch};else token={};++draws;
    if(FAILED(unlocked)){if(slot)slot->retired=true;note("cursor_native_written_unlock_failed","handled; no unsafe22px fallback after44px native write; cleanup retained until fresh overwrite");}
    else {try{note("cursor_replacement_draw",std::string(campaign?"C087":"AF87")+"; source88; physical44; native hotspot0,0; menu="+std::to_string(t.state));}catch(...){}}
    return true;
}
inline uint32_t __cdecl drawHook(void* source,int x,int y){
    const auto caller=uint32_t(reinterpret_cast<uintptr_t>(_ReturnAddress()));const DWORD error=GetLastError();
    if(!available||GetCurrentThreadId()!=owner){if(available)foreign=true;SetLastError(error);return reinterpret_cast<uint32_t(__cdecl*)(void*,int,int)>(originals[0])(source,x,y);}
    token={};bool nativeWritten=false;
    try{if(replace(caller,uint32_t(reinterpret_cast<uintptr_t>(source)),x,y,nativeWritten)){SetLastError(error);return 1;}}
    catch(...){if(nativeWritten){note("cursor_native_written_exception","handled; cleanup lease retained; no22px fallback");SetLastError(error);return 1;}note("cursor_original_retained","pre-write preparation exception");}
    SetLastError(error);return reinterpret_cast<uint32_t(__cdecl*)(void*,int,int)>(originals[0])(source,x,y);
}
inline void __stdcall sizeObserve(unsigned,const DWORD* saved)noexcept {
    const DWORD error=GetLastError();if(!available||GetCurrentThreadId()!=owner){if(available)foreign=true;SetLastError(error);return;}
    try{const auto used=token;token={};Target now{};uint32_t source=0;bool appliedDimensions=false;
        if(used.valid&&GetCurrentThreadId()==owner&&used.frame==frame&&used.epoch==epoch&&context(now)&&same(now,used.target)&&
            SelectionRestoration::read(0x5F7E68,source)&&source==used.source&&saved[6]==22&&saved[5]==22){
            auto* r=const_cast<DWORD*>(saved);r[6]=physicalSize;r[5]=physicalSize;appliedDimensions=true;}
        if(appliedDimensions){auto& lease=leases[now.index];
            if(lease&&same(lease->target,now)&&probeSamples<8){const int x=int(lease->pixels.rect.left),y=int(lease->pixels.rect.top);
                if(!probeSeen[now.index]||std::abs(x-probeX[now.index])+std::abs(y-probeY[now.index])>=16){
                    if(SelectionRestoration::read(0x718628+now.index*16,lease->bookBefore)){
                        int16_t mx=0,my=0;if(SelectionRestoration::read(0x5F7E5C,mx)&&SelectionRestoration::read(0x5F7E5E,my)){
                            lease->bookExpected={mx,my,lease->bookBefore[0]+int(physicalSize),lease->bookBefore[1]+int(physicalSize)};lease->probe=true;
                            probeSeen[now.index]=true;probeX[now.index]=x;probeY[now.index]=y;++probeSamples;
                        }
                    }
                }
            }
        }
    }catch(...){}SetLastError(error);
}
MENU_BRIDGE(sizeHook,1,4,originals,sizeObserve)
inline void install()noexcept {
    if(!Remaster::enabled())return;
    try{wchar_t exe[32768]{};if(!GetModuleFileNameW(nullptr,exe,32768))return;const auto dir=std::filesystem::path(exe).parent_path()/L"rs/Remaster/Selection";
        if(!std::filesystem::exists(dir/L"cursor.flags"))return;const auto option=SelectionRestoration::file(dir/L"cursor.flags",1);if(option.size()!=1||option[0]>1)throw std::runtime_error("cursor opt-in invalid");if(!option[0])return;
        auto art=Art::parse(SelectionRestoration::file(dir/L"cursor.dwcur",128*1024,assetHash));if(!art)throw std::runtime_error("cursor pixels invalid");
        constexpr uint8_t drawBytes[]={0x8B,0x44,0x24,0x04,0x80,0x38,0x54},sizeBytes[]={0xE9,0x88,0,0,0};uint8_t bytes[7]{};
        if(!SelectionRestoration::read(0x40F990,bytes,7)||memcmp(bytes,drawBytes,7)||!SelectionRestoration::read(0x43B241,bytes,5)||memcmp(bytes,sizeBytes,5))throw std::runtime_error("cursor hook source mismatch");
        const auto priorSize=originals[1];Compat::hookFunction(originals[1],reinterpret_cast<void*>(&sizeHook),"QUANTUM::CMain size receipt");if(priorSize==originals[1])throw std::runtime_error("cursor size seam failed");
        const auto priorDraw=originals[0];Compat::hookFunction(originals[0],reinterpret_cast<void*>(&drawHook),"QUANTUM::CMain native draw");if(priorDraw==originals[0])throw std::runtime_error("cursor draw seam failed");
        artwork=std::move(art);owner=GetCurrentThreadId();available=true;Remaster::note("cursor_replacement_ready",std::string(assetHash)+"; twoCMain resources; Main/Selection only; explicit44physical; native hotspot0,0; noWindows cursor settings changed");
    }catch(const std::exception& e){available=false;note("cursor_original_retained",e.what());}catch(...){available=false;}
}
}
