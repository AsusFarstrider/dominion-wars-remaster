#include <Remaster/Runtime.h>
#include "Probe.h"
#include <Remaster/VersionLabel.h>
#include "TransitionProbe.h"
#include <Research/MenuCanvas/CanvasRuntime.h>
#include <Research/OpeningFlow/OpeningFlow.h>
#include <Research/HudCaptain/HudCaptain.h>
#include <Research/HudAssembly/HudAssembly.h>
#include <intrin.h>
#include "Core.h"
#include "Bridge.h"
#include "HookSites.h"
#include <Research/NativeProbe/CounterBank.h>
#include <Research/FlightRecorder/Recorder.h>
#include <Common/Hook.h>
#include <Windows.h>
#include <atomic>
#include <mutex>

namespace MenuProbe {
namespace {
struct LastError {DWORD value=GetLastError();~LastError(){SetLastError(value);}};
std::atomic<bool> ready{false};std::atomic<uint64_t> serial{0},dispatches{0},dispatchDropped{0},exceptions{0};
bool compact=false;std::atomic<uint64_t> dispatchSampledOut{0},emittedCallbacks{0};
auto& bank=*new NativeProbe::CounterBank;auto& mutex=*new std::mutex;
thread_local CallbackStack callbacks;
thread_local std::array<uint32_t,64> traversals{};thread_local size_t traversalDepth=0;
thread_local std::array<uint64_t,64> apiScopes{};thread_local size_t apiDepth=0;
std::string n(const char* k,int64_t v){return ",\""+std::string(k)+"\":"+std::to_string(v);}
std::string words(const uint32_t* p,size_t count){std::string out="[";for(size_t i=0;i<count;++i){if(i)out+=",";out+=std::to_string(p[i]);}return out+"]";}
bool read(uint32_t address,void* output,size_t bytes){
    if(address<0x10000||!bytes||bytes>1024||uint64_t(address)+bytes>0x100000000ull)return false;
    uintptr_t cursor=address;const uint64_t end=uint64_t(address)+bytes;
    while(cursor<end){MEMORY_BASIC_INFORMATION m{};if(!VirtualQuery(reinterpret_cast<void*>(cursor),&m,sizeof(m))||m.State!=MEM_COMMIT||(m.Protect&(PAGE_GUARD|PAGE_NOACCESS)))return false;
        const auto p=m.Protect&0xff;if(p!=PAGE_READONLY&&p!=PAGE_READWRITE&&p!=PAGE_WRITECOPY&&p!=PAGE_EXECUTE_READ&&p!=PAGE_EXECUTE_READWRITE&&p!=PAGE_EXECUTE_WRITECOPY)return false;
        const auto next=reinterpret_cast<uintptr_t>(m.BaseAddress)+m.RegionSize;if(next<=cursor)return false;cursor=next;}
    SIZE_T got=0;return ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(address),output,bytes,&got)&&got==bytes;
}
template<class T> bool read(uint32_t a,T& v){return read(a,&v,sizeof(v));}
const Reader reader=static_cast<bool(*)(uint32_t,void*,size_t)>(&read);
LONGLONG tick(){LARGE_INTEGER v{};QueryPerformanceCounter(&v);return v.QuadPart;}
void gap(const char* why){MenuCanvasRuntime::coverageLost(why);Qfr::event("native_ui_gap",",\"reason\":"+Qfr::quote(why));}
bool sample(const char* kind,const std::string& key){std::lock_guard lock(mutex);return bank.sample(std::string(kind)+"|"+key+"|transition_epoch|"+std::to_string(MenuTransition::epoch()),tick());}
void emit(const char* kind,const std::string& fields){if(sample(kind,fields+ownerKey()))Qfr::event(kind,fields);}
std::string nodeJson(const Node& v){return "{\"address\":"+std::to_string(v.address)+n("id",v.id)+n("flags",v.flags)+n("x",v.x)+n("y",v.y)+n("width",v.w)+n("height",v.h)+n("parent",v.parent)+n("next",v.next)+n("child",v.child)+n("draw_callback",v.draw)+n("input_callback",v.input)+n("palette",v.palette)+"}";}
void image(const char* role,uint32_t address){
    if(!address)return;
    uint32_t raw[24]{};if(!read(address,raw,24)){emit("native_ui_read_gap",",\"role\":"+Qfr::quote(role)+n("address",address));return;}
    uint16_t flags=0;memcpy(&flags,raw,2);uint32_t native=address;size_t count=6;
    if(flags&0x8000){if(!read(address,raw,sizeof(raw))){emit("native_ui_read_gap",",\"role\":"+Qfr::quote(role)+n("address",address));return;}count=24;memcpy(&native,reinterpret_cast<uint8_t*>(raw)+0x20,4);}
    uint8_t d[24]{};const bool ok=read(native,d,sizeof(d));uint32_t surface=0,pixels=0;uint16_t w=0,h=0;int16_t pitch=0;
    if(ok){memcpy(&surface,d+14,4);memcpy(&pixels,d+18,4);memcpy(&w,d+10,2);memcpy(&h,d+12,2);memcpy(&pitch,d+22,2);}
    emit("native_ui_image",",\"role\":"+Qfr::quote(role)+n("outer",address)+n("prefix_flags",flags)+",\"raw_outer_words\":"+words(raw,count)+n("native_image",native)+",\"native_read\":"+(ok?"true":"false")+n("surface_va",surface)+n("pixels_va",pixels)+n("width",w)+n("height",h)+n("cached_pitch",pitch));
}
void state(){
    uint32_t holder=0,outer=0,clip=0,alias=0,device=0,font=0,palette=0,archive=0,appState=0,priorState=0,uiRootId=0;
    const bool ok=read(0xA39B64,holder)&&read(0xA39B78,clip)&&read(0xA387B4,alias)&&read(0xA38D74,device)&&read(0x4EC3B8,font)&&read(0x4EC390,palette)&&read(0x7138EC,archive)&&read(0x4D0A24,appState)&&read(0x4E3134,priorState)&&read(0x5F7ED8,uiRootId);
    const bool outerRead=holder&&read(holder,outer);int16_t rect[4]{};uint32_t dimensions[2]{};
    const bool clipRead=clip&&read(clip,rect);read(0x5FF670,dimensions);
    const auto fields=",\"globals_read\":"+std::string(ok?"true":"false")+n("holder_slot",0xA39B64)+n("current_slot",holder)+n("outer_wrapper",outer)+",\"outer_slot_read\":"+(outerRead?"true":"false")+n("clip_pointer",clip)+",\"clip_read\":"+(clipRead?"true":"false")+n("clip_x",rect[0])+n("clip_y",rect[1])+n("clip_w",rect[2])+n("clip_h",rect[3])+n("active_draw_com",alias)+n("device_com",device)+n("font_state",font)+n("palette_table",palette)+n("archive_resource_handle",archive)+n("application_state",appState)+n("prior_return_state",priorState)+n("ui_root_address",0x5F7ED8)+n("ui_root_id",uiRootId)+n("canvas_width",dimensions[0])+n("canvas_height",dimensions[1]);
    emit("native_ui_state",fields);if(outerRead)image("current_holder_destination",outer);
}
void* originals[15]={};
void __stdcall entry(unsigned index,const DWORD* frame) noexcept {
    LastError preserve;if(!ready)return;
    try{
        uint32_t r[9]{},s[9]{};if(!read(reinterpret_cast<uint32_t>(frame),r)||!read(r[3]+4,s)){gap("hook_frame_unreadable");return;}
        const uint32_t sp=r[3]+4;
        if(index==12)Remaster::VersionLabel::substitute(s[0],s[1],s[2],sp+8,
            [](uint32_t a,void* p,size_t n){return read(a,p,n);},
            [](uint32_t a,const void* p,size_t n){SIZE_T got=0;return WriteProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(a),p,n,&got)&&got==n;});
        MenuCanvasRuntime::observe(index,r,s,sp,originals[10]);
        OpeningFlow::observe(index,r,s,sp,originals[10]);
        HudCaptain::observe(index,r,s,sp);
        HudAssembly::observe(index,r,s,sp);
        if(!Qfr::enabled())return;
        if(index==0){if(traversalDepth==traversals.size()){gap("traversal_depth_limit");return;}traversals[traversalDepth++]=sp;
            emit("native_ui_traversal",",\"phase\":\"entry\""+n("stack_va",sp)+n("depth",traversalDepth)+n("first_node",s[1])+n("ancestor_x",int32_t(s[2]))+n("ancestor_y",int32_t(s[3])));return;}
        if(index==3){if(!traversalDepth||traversals[traversalDepth-1]!=sp+12){gap("traversal_return_stack_mismatch");traversalDepth=0;}else --traversalDepth;
            emit("native_ui_traversal",",\"phase\":\"return\""+n("stack_va",sp)+n("depth",traversalDepth));return;}
        if(index==1){
            auto a=ancestry(reader,r[1]);if(!a.gap.empty())gap(a.gap.c_str());
            const char* role=a.role=="selection_root"?"selection_root":a.role=="selection_child"?"selection_child":a.role=="native_popup"?"native_popup":"unrelated";
            const auto observed=++dispatches;
            const bool selected=!compact||sample("callback_scope",std::to_string(r[1])+"|"+role);
            const bool budget=compact?emittedCallbacks.load()<100000:observed<=100000;
            Callback c{++serial,r[1],role,selected&&budget};if(!callbacks.push(c)){gap("callback_depth_limit");callbacks.clear();return;}
            if(!c.emitted){if(!selected)++dispatchSampledOut;else ++dispatchDropped;return;}++emittedCallbacks;
            Qfr::event("native_ui_callback_begin",n("serial",c.serial)+n("node",c.node)+",\"role\":"+Qfr::quote(role)+n("depth",callbacks.depth())+n("ancestor_x",int32_t(r[2]))+n("ancestor_y",int32_t(r[0])));
            std::string nodes="[";for(const auto& v:a.nodes){if(nodes.size()>1)nodes+=",";nodes+=nodeJson(v);}nodes+="]";
            emit("native_ui_ancestry",",\"nodes\":"+nodes+",\"classification_gap\":"+Qfr::quote(a.gap));state();return;
        }
        if(index==2){const auto c=callbacks.top();if(!c||c->node!=r[1]){gap("callback_return_mismatch");callbacks.clear();return;}
            if(c->emitted)Qfr::event("native_ui_callback_end",n("serial",c->serial)+n("node",c->node));callbacks.pop(r[1]);return;}
        const auto base=n("hook_index",index)+n("caller",s[0])+n("ecx",r[6])+",\"raw_stack_words\":"+words(s,9);
        const auto c=callbacks.top();const auto ownerKey=std::to_string(c?c->node:0);
        uint32_t cursorKey=0;if(index==7)read(0x5F7E5C,cursorKey);
        if(!sample("ui_entry",std::to_string(index)+"|"+ownerKey+"|"+words(s,5)+(index==7?"|cursor|"+std::to_string(cursorKey):"")))return;
        Qfr::event("native_ui_entry",base);state();
        if(index==4){Node v;if(node(reader,s[1],v))emit("native_ui_selection",",\"node\":"+nodeJson(v)+n("ancestor_x",int32_t(s[2]))+n("ancestor_y",int32_t(s[3])));}
        if(index==7){auto a=ancestry(reader,s[1]);if(!a.gap.empty())gap(a.gap.c_str());int16_t xy[2]{};read(0x5F7E5C,xy);
            emit("native_ui_hit_entry",n("node",s[1])+",\"role\":"+Qfr::quote(a.role)+n("ancestor_x",int32_t(s[2]))+n("ancestor_y",int32_t(s[3]))+n("mouse_x",xy[0])+n("mouse_y",xy[1])+",\"interpretation\":\"entry inputs only; hit result and callback outcomes not captured\"");}
        if(index==9)image("background_copy_wrapper",s[1]);
        if(index==10){image("software_source",s[1]);image("software_destination",s[2]);}
        if(index==11){uint32_t raw[20]{};if(read(s[1],raw))emit("native_ui_texture_raw",n("address",s[1])+",\"words\":"+words(raw,20)+",\"interpretation\":\"hardware source raw words; not decoded as a packed image or COM pointer\"");}
        if(index==12)image("font_destination",s[1]);
        if(index==14)image("fill_destination",s[1]);
    }catch(...){++exceptions;gap("observer_exception");}
}
MENU_BRIDGE(h0,0,0,originals,entry) MENU_BRIDGE(h1,1,4,originals,entry) MENU_BRIDGE(h2,2,8,originals,entry)
MENU_BRIDGE(h3,3,12,originals,entry) MENU_BRIDGE(h4,4,16,originals,entry) MENU_BRIDGE(h5,5,20,originals,entry)
MENU_BRIDGE(h6,6,24,originals,entry) MENU_BRIDGE(h7,7,28,originals,entry) MENU_BRIDGE(h8,8,32,originals,entry)
MENU_BRIDGE(h9,9,36,originals,entry) MENU_BRIDGE(h10,10,40,originals,entry) MENU_BRIDGE(h11,11,44,originals,entry)
MENU_BRIDGE(h12,12,48,originals,entry) MENU_BRIDGE(h13,13,52,originals,entry) MENU_BRIDGE(h14,14,56,originals,entry)
HudCaptain::Blit cpuOriginal=nullptr;
uint32_t __cdecl legacyCPU(QImageResearch* src,QImageResearch* dst,int x,int y){return HudCaptain::software(cpuOriginal,src,dst,x,y);}
uint32_t __cdecl cpuDispatch(QImageResearch* src,QImageResearch* dst,int x,int y) {
    try{return HudAssembly::software(legacyCPU,src,dst,x,y);}
    catch(...){HudAssembly::boundary("software_adapter_exception");HudCaptain::boundary("software_adapter_exception");return cpuOriginal(src,dst,x,y);}
}
using BinkCopy=INT(__stdcall*)(void*,void*,INT,UINT,UINT,UINT,UINT);BinkCopy originalBink=nullptr;
INT __stdcall binkCopy(void* handle,void* dest,INT pitch,UINT height,UINT x,UINT y,UINT flags){
    const DWORD before=GetLastError();uint32_t h[4]{};const bool ok=read(reinterpret_cast<uint32_t>(handle),h);
    SetLastError(before);const auto result=OpeningFlow::copy(originalBink,reinterpret_cast<uintptr_t>(_ReturnAddress()),handle,dest,pitch,height,x,y,flags);const DWORD after=GetLastError();
    try{if(Qfr::enabled())emit("native_ui_movie",n("handle",reinterpret_cast<uint32_t>(handle))+n("destination",reinterpret_cast<uint32_t>(dest))+n("pitch",pitch)+n("copy_height",height)+n("x",x)+n("y",y)+n("format_flags",flags)+n("result",result)+",\"handle_words_read\":"+(ok?"true":"false")+",\"handle_words\":"+words(h,4));}catch(...){++exceptions;}
    SetLastError(after);return result;
}
}
std::string annotate(const char* kind,const std::string& fields) noexcept {
    if(!ready)return fields;LastError preserve;
    try{const auto c=callbacks.top();return fields+n("ui_state_epoch",MenuTransition::epoch())+n("ui_owner_frame",MenuTransition::frame())+n("ui_callback_serial",c&&c->emitted?c->serial:0)+n("ui_node",c&&c->emitted?c->node:0)+",\"ui_role\":"+Qfr::quote(c&&c->emitted?c->role:"outside_callback")+n("ui_api_scope",apiDepth?apiScopes[apiDepth-1]:0)+n("ui_traversal_depth",traversalDepth)+",\"ui_event_kind\":"+Qfr::quote(kind);}catch(...){++exceptions;return fields;}
}
std::string ownerKey() noexcept {
    if(!ready)return {};LastError preserve;
    try{const auto c=callbacks.top();return "|ui_owner|"+std::to_string(c?c->node:0)+"|"+(c?c->role:"outside_callback")+"|state_epoch|"+std::to_string(MenuTransition::epoch());}catch(...){++exceptions;return {};}
}
bool apiBegin(uint64_t id,const char* name,uintptr_t object,const uintptr_t* args,size_t count) noexcept {
    if(!ready)return false;LastError preserve;bool entered=false;
    try{if(apiDepth==apiScopes.size()){gap("api_scope_depth_limit");return false;}apiScopes[apiDepth++]=id;entered=true;
        uint32_t a[8]{};for(size_t i=0;i<count&&i<8;++i)a[i]=static_cast<uint32_t>(args[i]);
        Qfr::event("native_ui_api",n("api_id",id)+",\"name\":"+Qfr::quote(name)+n("object",object)+",\"argument_words\":"+words(a,8));
    }catch(...){++exceptions;gap("api_observation_exception");}
    return entered;
}
void apiEnd(uint64_t id) noexcept {if(!ready)return;LastError preserve;if(!apiDepth||apiScopes[apiDepth-1]!=id){gap("api_scope_return_mismatch");apiDepth=0;}else --apiDepth;}
void install(const std::string& identity) noexcept {
    LastError preserve;if(!Remaster::feature(Remaster::Feature::ui,L"QFR_UI_PROBE"))return;
    try{
        compact=Remaster::feature(Remaster::Feature::opening,L"QFR_OPENING_FLOW");
        if(identity!="de3e3ede7a9b4ce65a891201547f36413f3a880e4df3f28fb8a81e51f687abc6"){gap("UI_probe_engine_not_native1440_allowlist");return;}
        for(size_t i=0;i<std::size(sites);++i){uint8_t bytes[16]{};if(!read(sites[i].va,bytes,sites[i].size)||memcmp(bytes,sites[i].bytes,sites[i].size)){gap("UI_hook_original_bytes_mismatch");return;}}
        void* hooks[]={reinterpret_cast<void*>(&h0),reinterpret_cast<void*>(&h1),reinterpret_cast<void*>(&h2),reinterpret_cast<void*>(&h3),reinterpret_cast<void*>(&h4),reinterpret_cast<void*>(&h5),reinterpret_cast<void*>(&h6),reinterpret_cast<void*>(&h7),reinterpret_cast<void*>(&h8),reinterpret_cast<void*>(&h9),reinterpret_cast<void*>(&h10),reinterpret_cast<void*>(&h11),reinterpret_cast<void*>(&h12),reinterpret_cast<void*>(&h13),reinterpret_cast<void*>(&h14)};
        for(size_t i=0;i<std::size(sites);++i){originals[i]=reinterpret_cast<void*>(sites[i].va);const auto prior=originals[i];Compat::hookFunction(originals[i],hooks[i],sites[i].name);if(prior==originals[i]){gap("UI_hook_installation_failed_observation_disabled");return;}}
        cpuOriginal=reinterpret_cast<HudCaptain::Blit>(originals[10]);originals[10]=reinterpret_cast<void*>(&cpuDispatch);
        originalBink=reinterpret_cast<BinkCopy>(Compat::hookIatFunction(GetModuleHandleW(nullptr),"_BinkCopyToBuffer@28",reinterpret_cast<void*>(&binkCopy)));
        MenuTransition::install();
        MenuCanvasRuntime::enable();
        OpeningFlow::install(originalBink!=nullptr);
        HudCaptain::install();
        HudAssembly::install();
        ready=true;Remaster::ready(Remaster::uiReady);Qfr::event("native_ui_mode",",\"schema\":\"quantum-ui-context-1\",\"capture_only\":false,\"hooks\":15,\"dispatch_budget\":100000,\"parent_limit\":32,\"depth_limit\":64,\"bink_hook\":"+std::string(originalBink?"true":"false")+",\"changes\":\"original UI metadata plus optional canvas composition/predicate control; transition extension is read-only\",\"omissions\":[\"pixel payloads\",\"hit results\",\"exclusive cursor save/restore identity\",\"early process startup\",\"callback events beyond explicit budget\"]");
        if(!originalBink)gap("Bink_import_unobserved");
    }catch(...){gap("UI_install_exception");}
}
void finish() noexcept {
    MenuTransition::finish();
    MenuCanvasRuntime::finish();
    OpeningFlow::finish();
    HudCaptain::finish();
    HudAssembly::finish();
    if(!ready)return;LastError preserve;
    try{NativeProbe::CounterBank copy;{std::lock_guard lock(mutex);copy=bank;}
        uint64_t missed=copy.unknownGroupOverflow;
        for(const auto& [category,g]:copy.groups){for(const auto& [key,c]:g.keys)Qfr::event("native_ui_counts",",\"key\":"+Qfr::quote(key)+n("observed",c.seen)+n("sampled",c.kept)+n("first_qpc",c.first)+n("last_qpc",c.last));missed+=g.overflow;}
        Qfr::event("native_ui_end",n("dispatches",dispatches)+n("dispatch_budget_dropped",dispatchDropped)+n("dispatch_sampled_out",dispatchSampledOut)+n("callbacks_emitted",emittedCallbacks)+n("counter_overflow",missed)+n("observer_exceptions",exceptions)+n("exit_thread_callback_depth",callbacks.depth())+n("exit_thread_api_depth",apiDepth));
    }catch(...){gap("UI_summary_exception");}
}
}
