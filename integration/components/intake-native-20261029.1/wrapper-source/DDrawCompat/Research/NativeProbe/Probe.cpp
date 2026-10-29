#include <Remaster/Runtime.h>
#include <Research/MenuProbe/Probe.h>
#include <Research/MenuCanvas/CanvasRuntime.h>
// Bounded, metadata-only native-mode observations. No drawing or mode changes.
#include "Probe.h"
#include "CounterBank.h"
#include "EngineIdentity.h"
#include <Research/FlightRecorder/Recorder.h>
#include <Common/Hook.h>
#include <D3dDdi/Resource.h>
#include <Windows.h>
#include <bcrypt.h>
#include <ddraw.h>
#include <d3d.h>
#include <atomic>
#include <fstream>
#include <iterator>
#include <map>
#include <mutex>
#include <vector>

namespace NativeProbe {
namespace {
struct LastError {DWORD old=GetLastError();~LastError(){SetLastError(old);}};
auto& counters=*new CounterBank;
auto& counterMutex=*new std::mutex;
std::atomic<uint64_t> epoch{0},apiSerial{0},overflow{0};
const Identity::Engine* selected=nullptr;
LONGLONG tick(){LARGE_INTEGER v{};QueryPerformanceCounter(&v);return v.QuadPart;}
std::string n(const char* k,int64_t v){return ",\""+std::string(k)+"\":"+std::to_string(v);}
std::string arr(const uint32_t* v,size_t size){std::string r="[";for(size_t i=0;i<size;++i){if(i)r+=",";r+=std::to_string(v[i]);}return r+"]";}
bool read(uintptr_t address,void* out,size_t bytes){
    if(address<0x10000||!bytes||bytes>1024||uint64_t(address)+bytes>0x100000000ull)return false;
    uintptr_t cursor=address;
    while(cursor<address+bytes){MEMORY_BASIC_INFORMATION m{};
        if(!VirtualQuery(reinterpret_cast<void*>(cursor),&m,sizeof(m))||m.State!=MEM_COMMIT||(m.Protect&(PAGE_GUARD|PAGE_NOACCESS)))return false;
        const auto p=m.Protect&0xff;
        if(p!=PAGE_READONLY&&p!=PAGE_READWRITE&&p!=PAGE_WRITECOPY&&p!=PAGE_EXECUTE_READ&&p!=PAGE_EXECUTE_READWRITE&&p!=PAGE_EXECUTE_WRITECOPY)return false;
        const auto next=reinterpret_cast<uintptr_t>(m.BaseAddress)+m.RegionSize;if(next<=cursor)return false;cursor=next;
    }
    SIZE_T got=0;return ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(address),out,bytes,&got)&&got==bytes;
}
template<class T> bool read(uintptr_t a,T& v){return read(a,&v,sizeof(v));}
void gap(const char* why){Qfr::event("native_gap",",\"reason\":"+Qfr::quote(why)+n("epoch",epoch));}
bool sample(const std::string& key){
    std::lock_guard lock(counterMutex);
    return counters.sample(key,tick());
}
std::string screen(){
    uint32_t values[34]{};if(!read(0xA38770,values,sizeof(values))){gap("screen_object_unreadable");return {};}
    uint32_t mode=0,record[6]{};const auto context=values[0x30/4];
    const bool mr=context&&read(context+0x14,mode)&&mode&&read(mode,record,sizeof(record));
    return n("screen_object",0xA38770)+n("screen_flags",values[1])+n("width",values[2])+n("height",values[3])+n("bits",values[4])+
        n("enum_context",context)+n("selected_mode_pointer",mode)+",\"selected_mode_read\":"+(mr?"true":"false")+
        ",\"selected_mode_words\":"+arr(record,6)+n("window",values[0x34/4])+n("primary_surface",values[0x40/4])+
        n("active_draw_surface",values[0x44/4])+n("draw0",values[0x48/4])+n("active_depth_surface",values[0x64/4])+
        n("depth0",values[0x68/4])+n("viewport_count",values[0x84/4]);
}
void layout(const char* stage){
    uint32_t dimensions[2]{},scene[2]{},mission[3]{},views[44]{},projection[16]{};
    const bool ok=read(0x5FF670,dimensions)&&read(0x5FF6A4,scene)&&read(0x7137D0,mission)&&
        read(0x4F01A8,views)&&read(0x4F0308,projection);
    if(!ok){gap("layout_snapshot_unreadable");return;}
    uint32_t cursor=0;read(0x5F7E5C,cursor);
    Qfr::event("native_layout",",\"stage\":"+Qfr::quote(stage)+n("epoch",epoch)+",\"logical_wh\":"+arr(dimensions,2)+
        ",\"scene_wh\":"+arr(scene,2)+",\"mission_whbpp\":"+arr(mission,3)+",\"viewport_words\":"+arr(views,44)+
        ",\"projection_float_bits\":"+arr(projection,16)+n("cursor_xy_packed",cursor)+screen());
}
std::string desc(const DDSURFACEDESC2& d){
    return n("width",d.dwWidth)+n("height",d.dwHeight)+n("pitch",d.lPitch)+n("flags",d.dwFlags)+n("caps",d.ddsCaps.dwCaps)+
        n("bits",d.ddpfPixelFormat.dwRGBBitCount)+n("pixel_flags",d.ddpfPixelFormat.dwFlags)+n("red_mask",d.ddpfPixelFormat.dwRBitMask)+
        n("green_mask",d.ddpfPixelFormat.dwGBitMask)+n("blue_mask",d.ddpfPixelFormat.dwBBitMask)+n("refresh",d.dwRefreshRate)+n("surface_buffer_va",reinterpret_cast<uintptr_t>(d.lpSurface));
}
void descriptor(uintptr_t address,const char* stage,uint64_t id){
    DDSURFACEDESC2 d{};
    if(!read(address,d)||d.dwSize!=sizeof(d)){gap("surface4_descriptor_unreadable_or_wrong_size");return;}
    Qfr::event("native_surface_desc",n("api_id",id)+n("descriptor_address",address)+",\"stage\":"+Qfr::quote(stage)+desc(d));
}
const char* names[]={"screen_initialize","mode_select","mode_apply","draw_allocate","depth_allocate","viewport_create","layout_initialize",
    "application_layout","mission_setup","mission_defaults","application_transition","mode_change","screen_release","surface_restore",
    "scene_initialize","scene_frame","picker","mode_callback","window_rect_update","focus_gain","focus_loss"};
void* originals[]={reinterpret_cast<void*>(0x4A6510),reinterpret_cast<void*>(0x4A72A0),reinterpret_cast<void*>(0x4A6880),
    reinterpret_cast<void*>(0x4A6CD0),reinterpret_cast<void*>(0x4A6F10),reinterpret_cast<void*>(0x4A7010),reinterpret_cast<void*>(0x401860),
    reinterpret_cast<void*>(0x429030),reinterpret_cast<void*>(0x42B040),reinterpret_cast<void*>(0x42B5B0),reinterpret_cast<void*>(0x4292B0),
    reinterpret_cast<void*>(0x4A7B50),reinterpret_cast<void*>(0x4A6790),reinterpret_cast<void*>(0x4A8C70),reinterpret_cast<void*>(0x401070),
    reinterpret_cast<void*>(0x4022A0),reinterpret_cast<void*>(0x4070C0),reinterpret_cast<void*>(0x4A8140),reinterpret_cast<void*>(0x4A7AC0),
    reinterpret_cast<void*>(0x433FF0),reinterpret_cast<void*>(0x434040)};
void __stdcall entry(unsigned index,const DWORD* frame) noexcept {
    LastError preserve;
    try{
        if(index!=15 && index!=16 && index!=17 && index<std::size(names))MenuCanvasRuntime::resourceBoundary(names[index]);
        if(index==2&&selected){uint32_t context=0,mode=0,m[3]{};
            if(!read(0xA387A0,context)||!read(context+0x14,mode)||!read(mode,m)||m[0]!=selected->width||m[1]!=selected->height){
                Remaster::note("native_mode_refused","selected mode differs from package profile");Qfr::event("native_mode_rejected",",\"reason\":\"selected dimensions differ from pinned launch request or cannot be validated\"");ExitProcess(0x4E01);}}
        if(!Qfr::enabled())return;
        uint32_t r[9]{},stack[9]{};
        if(index>=std::size(names)||!read(reinterpret_cast<uintptr_t>(frame),r)||!read(r[3]+4,stack)){gap("entry_stack_or_registers_unreadable");return;}
        if(index<15||index>=18)++epoch;
        const std::string fields=n("function_index",index)+",\"name\":"+Qfr::quote(names[index])+n("epoch",epoch)+
            n("caller",stack[0])+n("ecx",r[6])+",\"raw_stack_words\":"+arr(stack,9)+screen();
        if(index==15||index==16){
            // Scene/picking contexts recur; retain samples plus exact counters.
            uint32_t dims[2]{},proj[16]{},cursor=0;read(0x5FF670,dims);read(0x4F0308,proj);read(0x5F7E5C,cursor);
            const auto key="entry|"+std::to_string(index)+"|"+std::to_string(epoch)+"|"+arr(dims,2)+"|"+arr(proj,16)+
                (index==16?"|"+std::to_string(stack[1])+"|"+std::to_string(stack[2]):"");
            if(!sample(key))return;
        }
        Qfr::event("native_engine_entry",fields);
        if(index==16){uint32_t matrix[16]{};
            if(read(stack[3],matrix))Qfr::event("native_picker",n("epoch",epoch)+n("x_float_bits",stack[1])+n("y_float_bits",stack[2])+
                n("projection_pointer",stack[3])+",\"projection_float_bits\":"+arr(matrix,16));
            else gap("picker_projection_unreadable");}
        if(index==17){DDSURFACEDESC2 d{};if(read(stack[1],d)&&d.dwSize==sizeof(d))Qfr::event("native_enumerated_mode",desc(d));else gap("mode_callback_descriptor_unreadable");}
        if(index!=17)layout(names[index]);

    }catch(...){gap("engine_observation_exception");}
}
// Proven QFR x86 entry bridge: all GPRs, flags, x87 and SSE state restored.
// No return-address substitution, guessed C ABI or callee-cleanup modification.
#define HOOK(symbol,index,offset) \
__declspec(naked) void symbol(){ \
    __asm pushfd \
    __asm pushad \
    __asm mov eax,esp \
    __asm sub esp,528 \
    __asm and esp,0FFFFFFF0h \
    __asm fxsave [esp] \
    __asm mov [esp+512],eax \
    __asm fninit \
    __asm push eax \
    __asm push index \
    __asm call entry \
    __asm fxrstor [esp] \
    __asm mov esp,[esp+512] \
    __asm popad \
    __asm popfd \
    __asm jmp dword ptr [originals+offset] \
}
HOOK(h0,0,0) HOOK(h1,1,4) HOOK(h2,2,8) HOOK(h3,3,12) HOOK(h4,4,16)
HOOK(h5,5,20) HOOK(h6,6,24) HOOK(h7,7,28) HOOK(h8,8,32) HOOK(h9,9,36)
HOOK(h10,10,40) HOOK(h11,11,44) HOOK(h12,12,48) HOOK(h13,13,52) HOOK(h14,14,56)
HOOK(h15,15,60) HOOK(h16,16,64) HOOK(h17,17,68) HOOK(h18,18,72) HOOK(h19,19,76) HOOK(h20,20,80)
#undef HOOK
}
bool active() noexcept {if(Remaster::managed)return Remaster::enabled();static const bool value=[](){LastError preserve;return Remaster::env(L"QFR_NATIVE_PROBE");}();return value;}
bool allowCall(const char* layer,const char* name) noexcept {
    if(!active())return true;
    return !strcmp(layer,"driver")&&(!strcmp(name,"pfnPresent")||!strcmp(name,"pfnPresent1"));
}
bool allowEvent(const char* kind,const std::string& fields) noexcept {
    if(!active())return true;
    if(!strncmp(kind,"native_",7))return true;
    if(!strcmp(kind,"device_create")||!strcmp(kind,"device_destroy")||!strcmp(kind,"display_focus"))++epoch;
    if(!strcmp(kind,"draw")||!strcmp(kind,"copy")||!strcmp(kind,"ownership")||!strcmp(kind,"write")||
       !strcmp(kind,"lock")||!strcmp(kind,"unlock")||!strcmp(kind,"cpu_copy_end")||!strcmp(kind,"boundary")||!strcmp(kind,"coverage_gap")){
        try{return sample(std::string(kind)+"|"+std::to_string(epoch)+"|"+fields+MenuProbe::ownerKey());}catch(...){++overflow;return false;}
    }
    return !strcmp(kind,"session")||!strcmp(kind,"build")||!strcmp(kind,"file")||!strcmp(kind,"os")||!strcmp(kind,"dpi")||
        !strcmp(kind,"monitor")||!strcmp(kind,"device_create")||!strcmp(kind,"device_destroy")||!strcmp(kind,"resource_create")||
        !strcmp(kind,"resource_create_failed")||!strcmp(kind,"resource_destroy")||!strcmp(kind,"subresource")||!strcmp(kind,"display_focus")||
        !strcmp(kind,"call_begin")||!strcmp(kind,"call_result")||!strcmp(kind,"call_end")||!strcmp(kind,"process_exit");
}
bool apiName(const char* name) noexcept {
    return strstr(name,"IDirectDraw4Vtbl::CreateSurface")||strstr(name,"IDirectDraw4Vtbl::SetDisplayMode")||
        strstr(name,"IDirectDraw4Vtbl::SetCooperativeLevel")||strstr(name,"IDirect3D3Vtbl::CreateDevice")||
        strstr(name,"IDirect3DViewport3Vtbl::SetViewport2")||strstr(name,"IDirect3DDevice3Vtbl::SetTransform")||
        strstr(name,"IDirect3DDevice3Vtbl::SetRenderTarget")||strstr(name,"IDirect3DDevice3Vtbl::SetCurrentViewport")||
        strstr(name,"IDirectDrawSurface4Vtbl::GetSurfaceDesc")||strstr(name,"IDirectDrawSurface4Vtbl::Lock")||strstr(name,"IDirectDrawSurface4Vtbl::Unlock")||
        strstr(name,"IDirectDrawSurface4Vtbl::Restore")||strstr(name,"IDirectDrawSurface4Vtbl::IsLost");
}
void ApiScope::begin() noexcept {
    if(!Qfr::enabled())return;
    LastError preserve;
    try{
        // Sample repeat states; all operation counts remain in native_counts.
        const auto key=std::string("api|")+name+"|"+std::to_string(epoch)+"|"+std::to_string(object)+"|"+std::to_string(args[0])+MenuProbe::ownerKey();
        const bool frequent=strstr(name,"SetTransform")||strstr(name,"SetViewport2")||strstr(name,"SetCurrentViewport")||strstr(name,"Lock")||strstr(name,"Unlock");
        if(frequent&&!sample(key))return;
        id=++apiSerial;
        menuScope=MenuProbe::apiBegin(id,name,object,args.data(),args.size());
        Qfr::event("native_api_begin",n("api_id",id)+",\"name\":"+Qfr::quote(name)+n("object",object)+n("epoch",epoch));
        if(strstr(name,"CreateSurface"))descriptor(args[0],"create_request",id);
        else if(strstr(name,"SetDisplayMode"))Qfr::event("native_mode_request",n("api_id",id)+n("width",args[0])+n("height",args[1])+n("bits",args[2])+n("refresh",args[3])+n("flags",args[4]));
        else if(strstr(name,"SetViewport2")){uint32_t v[11]{};if(read(args[0],v))Qfr::event("native_viewport",n("api_id",id)+",\"words\":"+arr(v,11));else gap("API_viewport_unreadable");}
        else if(strstr(name,"SetTransform")&&args[0]==D3DTRANSFORMSTATE_PROJECTION){uint32_t m[16]{};if(read(args[1],m))Qfr::event("native_projection",n("api_id",id)+",\"float_bits\":"+arr(m,16));else gap("API_projection_unreadable");}
        else if(strstr(name,"CreateDevice"))Qfr::event("native_device_target",n("api_id",id)+n("target_surface",args[1]));
        else if(strstr(name,"SetRenderTarget")||strstr(name,"SetCurrentViewport"))Qfr::event("native_bind",n("api_id",id)+n("argument_object",args[0]));
    }catch(...){gap("API_entry_observation_exception");}
}
void ApiScope::result(int64_t value) noexcept {
    LastError preserve;
    OpeningFlow::apiResult(name,object,caller,args.data(),argumentCount,value);
    if(!Qfr::enabled())return;
    try{
        if(value<0&&sample(std::string("api_failure|")+name+"|"+std::to_string(object)+"|"+std::to_string(value)+"|"+std::to_string(epoch)+MenuProbe::ownerKey()))
            Qfr::event("native_api_failure",",\"name\":"+Qfr::quote(name)+n("object",object)+n("hr",value)+n("epoch",epoch));
        if(!id)return;
        Qfr::event("native_api_result",n("api_id",id)+n("hr",value));
        if(value<0)return;
        if(strstr(name,"CreateSurface")||strstr(name,"CreateDevice")){uint32_t output=0;
            const auto slot=strstr(name,"CreateSurface")?args[1]:args[2];
            if(read(slot,output))Qfr::event("native_api_output",n("api_id",id)+n("interface_va",output));else gap("API_output_interface_unreadable");}
        else if(strstr(name,"GetSurfaceDesc"))descriptor(args[0],"original_descriptor_result",id);
        else if(strstr(name,"Lock"))descriptor(args[1],"original_lock_result",id);
    }catch(...){++overflow;}
}
ApiScope::~ApiScope(){if(id){LastError preserve;Qfr::event("native_api_end",n("api_id",id));if(menuScope)MenuProbe::apiEnd(id);}}
void presentation(uintptr_t device,const D3dDdi::Resource& dst,unsigned dstSub,const void* dstRect,
    const D3dDdi::Resource& src,unsigned srcSub,const void* srcRect,unsigned filter) noexcept {
    if(!active())return;LastError preserve;
    try{const auto& s=src.getFixedDesc().pSurfList[srcSub];const auto& d=dst.getFixedDesc().pSurfList[dstSub];
        const auto& sr=*static_cast<const RECT*>(srcRect);const auto& dr=*static_cast<const RECT*>(dstRect);
        const uint32_t rs[]={uint32_t(sr.left),uint32_t(sr.top),uint32_t(sr.right),uint32_t(sr.bottom)};
        const uint32_t rd[]={uint32_t(dr.left),uint32_t(dr.top),uint32_t(dr.right),uint32_t(dr.bottom)};
        const auto fields=n("epoch",epoch)+n("device",device)+n("source",Qfr::resource(device,reinterpret_cast<uintptr_t>(static_cast<HANDLE>(src))))+
            n("target",Qfr::resource(device,reinterpret_cast<uintptr_t>(static_cast<HANDLE>(dst))))+n("source_width",s.Width)+n("source_height",s.Height)+
            n("target_width",d.Width)+n("target_height",d.Height)+",\"source_rect\":"+arr(rs,4)+",\"target_rect\":"+arr(rd,4)+n("filter",filter);
        if(sample("presentation|"+fields))Qfr::event("native_presentation",fields);
    }catch(...){gap("presentation_observation_exception");}
}
void finish() noexcept {
    if(!active())return;LastError preserve;
    MenuProbe::finish();
    try{CounterBank copy;{std::lock_guard lock(counterMutex);copy=counters;}
        uint64_t keys=0,unrepresented=overflow+copy.unknownGroupOverflow;
        for(const auto& [category,g]:copy.groups){
            for(const auto& [key,c]:g.keys)Qfr::event("native_counts",",\"key\":"+Qfr::quote(key)+n("observed",c.seen)+n("sampled",c.kept)+n("first_qpc",c.first)+n("last_qpc",c.last));
            keys+=g.keys.size();unrepresented+=g.overflow;
            Qfr::event("native_counter_category",",\"category\":"+Qfr::quote(category)+n("keys",g.keys.size())+n("unrepresented_occurrences",g.overflow));
        }
        Qfr::event("native_counts_end",n("keys",keys)+n("unrepresented_occurrences",unrepresented));
    }catch(...){gap("counter_summary_exception");}
}
void install() noexcept {
    LastError preserve;
    try{
        if(reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr))!=0x400000){gap("executable_base_mismatch");return;}
        wchar_t path[32768]{};GetModuleFileNameW(nullptr,path,32768);std::ifstream file(path,std::ios::binary);
        std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(file)),{});unsigned char digest[32]{};
        if(bytes.empty()||bytes.size()>4*1024*1024||BCryptHash(BCRYPT_SHA256_ALG_HANDLE,nullptr,0,bytes.data(),ULONG(bytes.size()),digest,32)<0){gap("executable_identity_read_failed");return;}
        const char* hex="0123456789abcdef";std::string identity;for(auto b:digest){identity+=hex[b>>4];identity+=hex[b&15];}
        for(const auto& e:Identity::engines)if(identity==e.sha)selected=&e;
        if(!selected){gap("executable_identity_not_in_immutable_allowlist");return;}
        Qfr::event("native_probe_mode",",\"schema\":\"quantum-native-observation-1\",\"exe_sha256\":"+Qfr::quote(identity)+n("request_width",selected->width)+
            n("request_height",selected->height)+",\"hybrid_comparison\":"+(selected->hybrid?"true":"false")+
            ",\"metadata_only\":true,\"samples_per_key\":4,\"counter_key_limit_per_category\":32768,\"counter_category_limit\":16,\"omissions\":[\"CPU/GPU pixel payloads\",\"unselected API/driver calls\",\"most repeated metadata after four samples; counters retained subject to explicit category overflow\",\"engine initialization before first DirectDraw export\"],\"mode_mismatch_policy\":\"abort before original mode application; exit0x4E01\"");
        Qfr::event("native_request_preselection_snapshot",",\"basis\":\"screen-object state observed at first DirectDraw export, not the missed initial function entry\""+screen());
        void* hooks[]={reinterpret_cast<void*>(&h0),reinterpret_cast<void*>(&h1),reinterpret_cast<void*>(&h2),reinterpret_cast<void*>(&h3),reinterpret_cast<void*>(&h4),
            reinterpret_cast<void*>(&h5),reinterpret_cast<void*>(&h6),reinterpret_cast<void*>(&h7),reinterpret_cast<void*>(&h8),reinterpret_cast<void*>(&h9),
            reinterpret_cast<void*>(&h10),reinterpret_cast<void*>(&h11),reinterpret_cast<void*>(&h12),reinterpret_cast<void*>(&h13),reinterpret_cast<void*>(&h14),
            reinterpret_cast<void*>(&h15),reinterpret_cast<void*>(&h16),reinterpret_cast<void*>(&h17),reinterpret_cast<void*>(&h18),reinterpret_cast<void*>(&h19),reinterpret_cast<void*>(&h20)};
        for(unsigned i=0;i<std::size(hooks);++i){const auto before=originals[i];Compat::hookFunction(originals[i],hooks[i],names[i]);
            if(originals[i]==before){gap("engine_hook_did_not_install");return;}}
        MenuProbe::install(identity);
        Remaster::ready(Remaster::nativeReady);
        Qfr::event("native_hooks_installed",n("count",std::size(hooks))+",\"entry_only\":true,\"return_address_substitution\":false");
    }catch(...){gap("hook_installation_exception");}
}
}
