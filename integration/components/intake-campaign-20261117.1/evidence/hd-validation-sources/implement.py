"""Apply the scoped HD provider to its disposable work tree, once."""
from pathlib import Path
p=Path('E:/Codex/research/dominion-rendering/hd-video-work-01/DDrawCompat')
f=p/'Research/OpeningFlow/OpeningFlow.cpp';s=f.read_text()
assert 'HdHistory.h' not in s
s=s.replace('#include "OpeningCore.h"','#include "OpeningCore.h"\n#include "HdHistory.h"\n#include <mutex>')
s=s.replace('struct Movie {uint64_t','bool hdRequested=false,safetyInstalled=false;std::recursive_mutex movieMutex;\nstruct Movie {std::unique_ptr<HdHistory> hd;uint64_t')
a=s.index('void* __stdcall binkOpen(');b=s.index('bool movieOwner(',a)
s=s[:a]+'''using Goto=void(__stdcall*)(void*,unsigned,unsigned);Goto originalGoto=nullptr;
bool header(uint32_t p,MovieGeometry& g) {
    return read(p,g.width)&&read(p+4,g.height)&&read(p+0x20,g.handleFlags)&&
        read(p+0x1E4,g.internalWidth)&&read(p+0x1E8,g.internalHeight);
}
bool hdProfile(uint32_t p,const MovieGeometry& g) {
    return g.width==2560&&g.height==1440&&g.internalWidth==g.width&&g.internalHeight==g.height&&
        !(g.handleFlags&(0x70000000|0x80000))&&value(p+8)==225&&value(p+0x14)==15&&value(p+0x18)==1;
}
bool large(const MovieGeometry& g){return g.width>800||g.height>600||g.internalWidth>800||g.internalHeight>600;}
void* __stdcall binkOpen(const void* data,uint32_t flags) {
    const DWORD before=GetLastError();SetLastError(before);void* handle=originalOpen(data,flags);const DWORD after=GetLastError();
    if(!handle){SetLastError(after);return nullptr;}
    const auto pointer=static_cast<uint32_t>(reinterpret_cast<uintptr_t>(handle));
    bool reject=false;
    try {
        std::lock_guard<std::recursive_mutex> lock(movieMutex);
        MovieGeometry g{};const bool known=header(pointer,g);
        // The early gate is independent of UI installation. Until the safe
        // Copy provider is enabled, no HD handle can escape to native80,60.
        const bool hd=hdRequested&&(!known||large(g));
        if(hd&&(!known||!sameThread()||!safetyInstalled||!hdProfile(pointer,g)))reject=true;
        else if(sameThread()) {
            auto m=std::make_shared<Movie>();m->lifetime=++lifetimes;m->geometry=g;
            if(hd)m->hd=std::make_unique<HdHistory>(g.width,g.height);
            movies[pointer]=std::move(m);
            trace(hd?"native_hd_open":"native_opening_video_open",n("handle",pointer)+n("lifetime",lifetimes)+
                n("open_flags",flags)+n("source_width",g.width)+n("source_height",g.height));
        }
    }catch(...){reject=hdRequested;error("video_open_observation_failed");}
    if(reject){originalClose(handle);handle=nullptr;error("HD_open_refused_closed_once");}
    SetLastError(after);return handle;
}
void __stdcall binkClose(void* handle) {
    const DWORD before=GetLastError();
    try {std::lock_guard<std::recursive_mutex> lock(movieMutex);
        const auto pointer=static_cast<uint32_t>(reinterpret_cast<uintptr_t>(handle));const auto i=movies.find(pointer);
        if(i!=movies.end()) {
            if(sameThread()){trace(i->second->hd?"native_hd_close":"native_opening_video_close",n("handle",pointer)+n("lifetime",i->second->lifetime));moviePublication={};}
            movies.erase(i);
        }
        SetLastError(before);originalClose(handle);
    }catch(...){error("video_close_observation_failed");}
}
void __stdcall binkGoto(void* handle,unsigned frame,unsigned flags) {
    const DWORD before=GetLastError();
    std::lock_guard<std::recursive_mutex> lock(movieMutex);
    auto i=movies.find(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(handle)));
    if(i!=movies.end()&&i->second->hd){i->second->hd->reset();moviePublication={};
        trace("native_hd_seek",n("frame",frame)+n("flags",flags)+n("lifetime",i->second->lifetime));}
    SetLastError(before);originalGoto(handle,frame,flags);
}
''' +s[b:]
# Exact frozen file identities are checked before the first media Open.
a=s.index('\n}\nvoid install() noexcept',s.index('bool dllIdentity()'))
s=s[:a]+'''
bool fileIdentity(const std::filesystem::path& path,const char* expected) {
    std::ifstream file(path,std::ios::binary);std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(file)),{});
    unsigned char digest[32]{};if(bytes.empty()||bytes.size()>128*1024*1024||
        BCryptHash(BCRYPT_SHA256_ALG_HANDLE,nullptr,0,bytes.data(),ULONG(bytes.size()),digest,32)<0)return false;
    const char* hex="0123456789abcdef";std::string actual;for(auto b:digest){actual+=hex[b>>4];actual+=hex[b&15];}
    return actual==expected;
}
''' +s[a:]
s=s.replace('void install() noexcept {','''bool installSafety() noexcept {
    LastError preserve;wchar_t v[2]{};
    if(GetEnvironmentVariableW(L"QFR_HD_MOVIE",v,2)!=1||v[0]!=L'1')return true;
    hdRequested=true;
    try {
        wchar_t path[32768]{};if(!GetModuleFileNameW(nullptr,path,32768)||!dllIdentity())return false;
        if(!fileIdentity(path,"de3e3ede7a9b4ce65a891201547f36413f3a880e4df3f28fb8a81e51f687abc6")||
           !fileIdentity(std::filesystem::path(path).parent_path()/L"rs/Video/Paramount.bik",
               "5a3506e3f48190073591c1349f4e35dbfe8dde9d2ded19364fa865817deecff3"))return false;
        originalClose=reinterpret_cast<Close>(Compat::hookIatFunction(GetModuleHandleW(nullptr),"_BinkClose@4",reinterpret_cast<void*>(&binkClose)));
        originalGoto=reinterpret_cast<Goto>(Compat::hookIatFunction(GetModuleHandleW(nullptr),"_BinkGoto@12",reinterpret_cast<void*>(&binkGoto)));
        originalOpen=reinterpret_cast<Open>(Compat::hookIatFunction(GetModuleHandleW(nullptr),"_BinkOpen@8",reinterpret_cast<void*>(&binkOpen)));
        safetyInstalled=originalOpen&&originalClose&&originalGoto;
        return safetyInstalled;
    }catch(...){return false;}
}
void install(bool copyReady) noexcept {''')
s=s.replace('if(!dllIdentity()){error("shipped_bink_identity_mismatch");return;}','if(!copyReady){error("video_copy_hook_missing");return;}\n        if(!dllIdentity()){error("shipped_bink_identity_mismatch");return;}\n        if(!safetyInstalled) {')
s=s.replace('if(!originalOpen||!originalClose){error("video_lifetime_import_hook_failed");return;}','}\n        if(!originalOpen||!originalClose||(hdRequested&&!safetyInstalled)){error("video_lifetime_import_hook_failed");return;}')
# Add the HD Copy provider in the same owner/lease controller; it is executed
# before every old early-return/catch/fallback. Unknown HD can never reach it.
insert=s.index('\nint copy(BinkCopy original,')
s=s[:insert]+'''
namespace {
int copyHd(BinkCopy original,uintptr_t caller,void* handle,void* destination,int pitch,unsigned height,unsigned x,unsigned y,unsigned flags) noexcept {
    const DWORD before=GetLastError();DWORD after=before;bool called=false;int result=-1;
    std::shared_ptr<Movie> owned;
    try {
        std::lock_guard<std::recursive_mutex> lock(movieMutex);
        const auto pointer=static_cast<uint32_t>(reinterpret_cast<uintptr_t>(handle));auto i=movies.find(pointer);
        moviePublication.interactive=false;
        if(!sameThread()||!safetyInstalled||copyingVideo||i==movies.end()||!i->second->hd) {
            error("HD_copy_refused_no_safe_owner");SetLastError(before);return -1;
        }
        owned=i->second;auto& history=*owned->hd;MovieGeometry g{};
        if(history.quarantined||!header(pointer,g)||!hdProfile(pointer,g)||height!=g.height||flags!=10||
           caller!=0x40F2ED||x!=80||y!=60||!history.intact()) {
            history.quarantined=true;error("HD_copy_quarantined_source_or_call");SetLastError(before);return -1;
        }
        const auto* c=context();uint32_t slot=0,media=0;Target t{};
        const auto savedEpoch=epoch,token=lease.token;
        const bool owner=movieOwner(c,pointer,slot,media)&&c==&specs[0]&&media==value(0x749020+4*0x5C5);
        const bool output=owner&&lease.active&&lease.epoch==epoch&&target(t)&&lease.surface==t.surface&&
            lease.d.lpSurface==destination&&lease.d.lPitch==pitch&&format(lease.d,t)&&
            t.w==history.width&&t.h==history.height;
        // A transient lost output cannot redirect decoding into native memory.
        // The known source buffer remains valid independently of that lease.
        if(!owner){history.quarantined=true;error("HD_copy_quarantined_resource_identity");SetLastError(before);return -1;}
        const auto copyFlags=history.copyFlags();Busy busy(copyingVideo);
        SetLastError(before);called=true;result=original(handle,history.data(),int(history.pitch),height,0,0,copyFlags);after=GetLastError();++movieCopies;
        const auto live=movies.find(pointer);
        const bool source=history.converted(result)&&live!=movies.end()&&live->second==owned;
        Target current{};const bool publish=source&&output&&epoch==savedEpoch&&lease.active&&lease.token==token&&
            movieOwner(c,pointer,slot,media)&&target(current)&&current.surface==t.surface&&current.w==t.w&&current.h==t.h;
        bool written=false;
        if(publish) {
            // Exact1:1 rows: source history never traverses800x600 or global menu fit.
            written=history.identity(view(lease.d));
            if(written){surfaces[t.surface].scaled=true;moviePublication={++serial,epoch,c,t.surface,false};lease.written=true;}
        }
        if(!output)error("HD_output_withheld_invalid_native_lease");
        if(history.quarantined)error("HD_copy_result_or_guard_failed");
        trace("native_hd_copy",n("handle",pointer)+n("lifetime",owned->lifetime)+n("slot",slot)+n("media_slot_value",media)+n("frame",value(pointer+12))+
            n("token",token)+n("source_width",g.width)+n("source_height",g.height)+n("internal_width",g.internalWidth)+n("internal_height",g.internalHeight)+
            n("source_pitch",history.pitch)+n("source_x",0)+n("source_y",0)+n("original_x",x)+n("original_y",y)+n("copy_flags",copyFlags)+n("result",result)+
            n("target_width",t.w)+n("target_height",t.h)+n("target_pitch",lease.d.lPitch)+n("surface",lease.surface)+
            ",\\"history_valid\\":"+(history.initialized?"true":"false")+",\\"guards_intact\\":"+(history.intact()?"true":"false")+
            ",\\"identity_written\\":"+(written?"true":"false")+",\\"native_writes_withheld\\":"+(!written?"true":"false"));
        SetLastError(after);return result;
    }catch(...) {
        if(owned&&owned->hd)owned->hd->quarantined=true;
        moviePublication.interactive=false;error("HD_provider_exception_no_legacy_fallback");SetLastError(called?after:before);return called?result:-1;
    }
}
}
''' +s[insert:]
s=s.replace('    if(!sameThread())return original(handle,destination,pitch,height,x,y,flags);','''    if(hdRequested) {
        // Header probing is bounded even on a foreign thread. Failure refuses
        // rather than guessing that this is safe legacy artwork.
        MovieGeometry g{};const auto pointer=static_cast<uint32_t>(reinterpret_cast<uintptr_t>(handle));
        const DWORD before=GetLastError();const bool known=header(pointer,g);SetLastError(before);
        if(!known||large(g))return copyHd(original,caller,handle,destination,pitch,height,x,y,flags);
        std::lock_guard<std::recursive_mutex> lock(movieMutex);
        const auto i=movies.find(pointer);
        if(i!=movies.end()&&i->second->hd)return copyHd(original,caller,handle,destination,pitch,height,x,y,flags);
    }
    if(!sameThread())return original(handle,destination,pitch,height,x,y,flags);''')
# Exact root-only HD skip mapping. Original dispatcher masks/callback are not
# changed, and the global pointer remains in native coordinates.
s=s.replace('        bool admitted=known&&publication.interactive', '''        bool hd=false;
        if(hdRequested&&s==&specs[0]&&address==s->root) {
            const auto slot=value(0x4E47B8);const auto i=slot<4?movies.find(value(0x74AA0C+slot*0x24)):movies.end();
            hd=i!=movies.end()&&i->second->hd&&!i->second->hd->quarantined&&i->second->hd->initialized;
        }
        bool admitted=known&&publication.interactive''')
s=s.replace('t.surface==publication.surface&&plan&&planW==t.w&&planH==t.h;', 't.surface==publication.surface&&(hd?(t.w==2560&&t.h==1440):(plan&&planW==t.w&&planH==t.h));')
s=s.replace('if(admitted) {const auto q=plan->geometry().inverse_pixel({int64_t(x)+px,int64_t(y)+py});', '''if(admitted&&hd)hit=dw::ui::contains({0,0,t.w,t.h},{int64_t(x)+px,int64_t(y)+py});
        else if(admitted) {const auto q=plan->geometry().inverse_pixel({int64_t(x)+px,int64_t(y)+py});''')
f.write_text(s)
f=p/'Research/OpeningFlow/OpeningFlow.h';s=f.read_text().replace('void install() noexcept;','bool installSafety() noexcept; // Must precede all media Open calls for the opt-in HD profile.\nvoid install(bool copyReady=true) noexcept;');f.write_text(s)
f=p/'Research/MenuProbe/Probe.cpp';s=f.read_text().replace('OpeningFlow::install();','OpeningFlow::install(originalBink!=nullptr);');f.write_text(s)
f=p/'Dll/DllMain.cpp';s=f.read_text().replace('#include <Research/FlightRecorder/Driver.h>','#include <Research/FlightRecorder/Driver.h>\n#include <Research/OpeningFlow/OpeningFlow.h>');s=s.replace('\t\tif (!Config::enableDDrawCompat.get())','\t\t// Fail process initialization rather than exposing an HD asset to legacy Copy.\n\t\tif (!OpeningFlow::installSafety())return FALSE;\n\n\t\tif (!Config::enableDDrawCompat.get())');f.write_text(s)
print('HD provider applied to isolated work tree')
