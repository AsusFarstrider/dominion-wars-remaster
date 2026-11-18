#include <Remaster/Runtime.h>
#include "TransitionProbe.h"
#include <Research/OpeningFlow/OpeningFlow.h>
#include <Research/HudAssembly/HudAssembly.h>
#include "TransitionState.h"
#include "Bridge.h"
#include "TransitionSites.h"
#include <Common/Hook.h>
#include <Research/FlightRecorder/Recorder.h>
#include <Windows.h>
#include <atomic>
#include <string>

namespace MenuTransition {
namespace {
struct LastError {DWORD value=GetLastError();~LastError(){SetLastError(value);}};
bool enabled=false;
thread_local Counter counter;
thread_local Frames ownerFrames;
std::atomic<uint64_t> snapshots{0},gaps{0},exceptions{0},entries{0},returns{0},prepaints{0},outsideInstallers{0};
std::atomic<DWORD> owner{0};
void* original[6]{};
bool read(uint32_t address,void* output,size_t bytes) {
    if(address<0x10000||!bytes||bytes>1024||uint64_t(address)+bytes>0x100000000ull)return false;
    uintptr_t cursor=address;const uint64_t end=uint64_t(address)+bytes;
    while(cursor<end) {
        MEMORY_BASIC_INFORMATION m{};
        if(!VirtualQuery(reinterpret_cast<void*>(cursor),&m,sizeof(m))||m.State!=MEM_COMMIT||
            (m.Protect&(PAGE_GUARD|PAGE_NOACCESS)))return false;
        const auto p=m.Protect&0xFF;
        if(p!=PAGE_READONLY&&p!=PAGE_READWRITE&&p!=PAGE_WRITECOPY&&p!=PAGE_EXECUTE_READ&&
            p!=PAGE_EXECUTE_READWRITE&&p!=PAGE_EXECUTE_WRITECOPY)return false;
        const auto next=reinterpret_cast<uintptr_t>(m.BaseAddress)+m.RegionSize;
        if(next<=cursor)return false;cursor=next;
    }
    SIZE_T got=0;return ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(address),output,bytes,&got)&&got==bytes;
}
std::string n(const char* key,uint64_t value){return ",\""+std::string(key)+"\":"+std::to_string(value);}
template<size_t N> std::string words(const std::array<uint32_t,N>& a) {
    std::string s="[";for(size_t i=0;i<N;++i){if(i)s+=",";s+=std::to_string(a[i]);}return s+"]";
}
void traceGap(const char* reason) noexcept {
    ++gaps;ownerFrames.invalidate();
    try{Qfr::event("native_transition_gap",",\"reason\":"+Qfr::quote(reason));}catch(...){++exceptions;}
}
void __stdcall observe(unsigned index,const DWORD* saved) noexcept {
    if(!enabled)return;LastError preserve;
    auto thread=owner.load();
    if(!thread){owner.compare_exchange_strong(thread,GetCurrentThreadId());thread=owner.load();}
    if(thread!=GetCurrentThreadId()) {
        traceGap("foreign frame thread");return;
    }
    try {
        // These two actions are functional; recording must never control them.
        if(index==0)HudAssembly::prepareFrame();
        else if(index==4)OpeningFlow::resourceBoundary("state_install");
        if(!Qfr::enabled())return;
        const auto sp=saved[3]+4;uint32_t caller=0,argument=0;
        if(index==0) {
            uint8_t transitions=0;
            if(!read(sp,&caller,4)||!read(sp+4,&transitions,1)||!ownerFrames.enter(sp,transitions)) {
                traceGap("frame entry unreadable or nesting/serial limit");return;
            }
            ++entries;
        }else if(index==4) {
            if(!read(sp,&caller,4)||!read(sp+4,&argument,4)){traceGap("installer stack unreadable");return;}
            if(!ownerFrames.top())++outsideInstallers;
            // The bounded normal-frame call pushes the next-state argument.
            else if(caller!=0x4294D1||!ownerFrames.matches(sp,16)) {
                traceGap("nested or unexpected installer caller");
            }
        }else if(!ownerFrames.matches(sp,index==5?0:8)) {
            traceGap("frame phase/return stack mismatch");return;
        }
        Snapshot s;
        if(!capture(read,s)||!counter.observe(s)) {
            traceGap("snapshot unreadable or counter overflow");return;
        }
        ++snapshots;if(index==1)++prepaints;
        const char* kinds[]={"native_transition_frame_entry","native_transition_prepaint","native_transition_postdraw",
            "native_transition_precleanup","native_transition_install_entry","native_transition_frame_end"};
        const auto* frame=ownerFrames.top();
        Qfr::event(kinds[index],n("visit",counter.frame)+n("frame",frame?frame->id:0)+n("depth",ownerFrames.depth())+
            n("transition_enabled",frame?frame->transitions:0)+n("state_epoch",counter.epoch)+
            n("stack",sp)+n("caller",caller)+n("installer_argument",argument)+
            n("activity",s.values[11])+n("screen_flags",s.values[12])+
            n("current",s.values[0])+n("pending",s.values[1])+n("previous",s.values[2])+
            n("input",s.values[3])+n("secondary",s.values[4])+n("draw",s.values[5])+n("scene",s.values[6])+
            n("holder",s.values[7])+n("draw_alias",s.values[8])+n("device",s.values[9])+n("root_child",s.values[10])+
            n("root_flags",s.root.flags)+n("root_next",s.root.next)+n("root_draw",s.root.draw)+
            n("child_id",s.child.id)+n("child_flags",s.child.flags)+n("child_draw",s.child.draw)+n("child_next",s.child.next)+
            ",\"child_read\":"+(s.childRead?"true":"false")+",\"outer_read\":"+(s.outerRead?"true":"false")+
            n("outer",s.outer)+n("native",s.native)+",\"native_read\":"+(s.nativeRead?"true":"false")+
            ",\"cached_native_words\":"+words(s.nativeWords)+
            ",\"writes\":false");
        if(index==5){if(!ownerFrames.leave(sp))traceGap("frame return mismatch");else ++returns;}
    }catch(...){++exceptions;traceGap("observer exception");}
}
MENU_BRIDGE(entryHook,0,0,original,observe)
MENU_BRIDGE(prepaintHook,1,4,original,observe)
MENU_BRIDGE(postdrawHook,2,8,original,observe)
MENU_BRIDGE(cleanupHook,3,12,original,observe)
MENU_BRIDGE(installHook,4,16,original,observe)
MENU_BRIDGE(returnHook,5,20,original,observe)
}
uint64_t epoch() noexcept {return enabled?counter.epoch:0;}
uint64_t frame() noexcept {const auto* f=ownerFrames.top();return enabled&&f?f->id:0;}
void install() noexcept {
    LastError preserve;
    if(!Remaster::feature(Remaster::Feature::transitions,L"QFR_UI_TRANSITIONS"))return;
    try {
        for(const auto& site:sites) {
            unsigned char actual[16]{};
            if(!read(site.va,actual,site.size)||memcmp(actual,site.bytes,site.size)) {
                ++gaps;Qfr::event("native_transition_gate",",\"enabled\":false,\"reason\":\"phase bytes mismatch\"");return;
            }
        }
        void* hooks[]={reinterpret_cast<void*>(&entryHook),reinterpret_cast<void*>(&prepaintHook),reinterpret_cast<void*>(&postdrawHook),
            reinterpret_cast<void*>(&cleanupHook),reinterpret_cast<void*>(&installHook),reinterpret_cast<void*>(&returnHook)};
        for(size_t i=0;i<std::size(sites);++i) {
            original[i]=reinterpret_cast<void*>(sites[i].va);const auto before=original[i];
            Compat::hookFunction(original[i],hooks[i],sites[i].name);
            if(original[i]==before){++gaps;Qfr::event("native_transition_gate",",\"enabled\":false,\"reason\":\"hook installation failed\"");return;}
        }
        enabled=true;Remaster::ready(Remaster::transitionReady);
        Qfr::event("native_transition_mode",",\"schema\":\"quantum-transition-observation-1\",\"capture_only\":true,\"hooks\":6,\"writes\":false,\"sampling\":\"every reached phase boundary; state epochs qualify existing metadata sample keys\",\"limitations\":[\"prepaint join can be reached without input dispatch\",\"cached target descriptor is not Lock or storage-generation proof\",\"no visual retirement or extra graphics calls\",\"clear/write receipts need producer and returned Lock joins\",\"early initialization unobserved\"]");
    }catch(...){++exceptions;++gaps;Qfr::event("native_transition_gate",",\"enabled\":false,\"reason\":\"installation exception\"");}
}
void finish() noexcept {
    if(!enabled)return;LastError preserve;
    try{Qfr::event("native_transition_end",n("snapshots",snapshots)+n("entries",entries)+n("returns",returns)+
        n("prepaints",prepaints)+n("outside_frame_installers",outsideInstallers)+n("gaps",gaps)+
        n("observer_exceptions",exceptions)+n("exit_thread_frame_depth",ownerFrames.depth())+n("owner_thread",owner));}catch(...){}
}
}
