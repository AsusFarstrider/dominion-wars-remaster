#include <Remaster/Runtime.h>
#include "CanvasRuntime.h"
#ifndef MENU_CANVAS_TEST
#include <Research/OpeningFlow/OpeningFlow.h>
#include <Research/HudCaptain/HudCaptain.h>
#include <Research/HudAssembly/HudAssembly.h>
#endif
#include "Canvas565.h"
#include "quantum_font_research.h"
#include "PredicateBridge.h"
#include "PublicationState.h"
#include "ScopeUnlock.h"
#include "InputPolicy.h"
#include "MenuFamilies.h"
#ifdef MENU_CANVAS_TEST
#include "RuntimeTestMemory.h"
#endif
#include "MainBackdrop.h"
#include "CanvasHookSites.h"
#include "ProducerSites.h"
#include <Research/MenuProbe/Bridge.h>
#include <Research/MenuProbe/Core.h>
#include <Research/FlightRecorder/Recorder.h>
#include <Common/Hook.h>
#include <Windows.h>
#include <ddraw.h>
#include <array>
#include <atomic>
#include <memory>
#include <string>

namespace MenuCanvasRuntime {
namespace {
struct LastError {DWORD value=GetLastError();~LastError(){SetLastError(value);}};
bool enabled=false;
bool mainEnabled=false;
std::atomic<DWORD> ownerThread{0};
std::atomic<bool> foreignThreadSeen{false},scaledWriteSeen{false};
uint64_t nextPass=0,commits=0,rejections=0,shadowDraws=0;
uint64_t controllerExceptions=0;
void* inputOriginals[6]{};
bool readable(uint32_t address,size_t bytes) {
#ifdef MENU_CANVAS_TEST
    if(MenuCanvasTest::managed(address,bytes))return MenuCanvasTest::readable(address,bytes);
#endif
    if(address<0x10000 || !bytes || uint64_t(address)+bytes>0x100000000ull)return false;
    uintptr_t cursor=address;const uint64_t end=uint64_t(address)+bytes;
    while(cursor<end) {
        MEMORY_BASIC_INFORMATION m{};
        if(!VirtualQuery(reinterpret_cast<void*>(cursor),&m,sizeof(m)) || m.State!=MEM_COMMIT ||
            (m.Protect&(PAGE_GUARD|PAGE_NOACCESS)))return false;
        const auto p=m.Protect&0xFF;
        if(p!=PAGE_READONLY && p!=PAGE_READWRITE && p!=PAGE_WRITECOPY && p!=PAGE_EXECUTE_READ &&
            p!=PAGE_EXECUTE_READWRITE && p!=PAGE_EXECUTE_WRITECOPY)return false;
        const auto next=reinterpret_cast<uintptr_t>(m.BaseAddress)+m.RegionSize;
        if(next<=cursor)return false;cursor=next;
    }
    return true;
}
bool readBytes(uint32_t address,void* output,size_t bytes) {
#ifdef MENU_CANVAS_TEST
    if(MenuCanvasTest::managed(address,bytes))return MenuCanvasTest::read(address,output,bytes);
#endif
    if(!readable(address,bytes))return false;
    std::memcpy(output,reinterpret_cast<const void*>(address),bytes);return true;
}
template<class T> bool read(uint32_t address,T& value) {return readBytes(address,&value,sizeof(value));}
template<class T> void write(uint32_t address,const T& value) {
#ifdef MENU_CANVAS_TEST
    if(MenuCanvasTest::managed(address,sizeof(value))){MenuCanvasTest::write(address,&value,sizeof(value));return;}
#endif
    std::memcpy(reinterpret_cast<void*>(address),&value,sizeof(value));
}
const MenuProbe::Reader reader=readBytes;
const auto& buttons=MenuCanvas::selection.buttons; // inherited regression fixture
struct Target {uint32_t slot=0,outer=0,native=0,surface=0;uint16_t w=0,h=0,outerW=0,outerH=0;};
bool target(Target& t) {
    uint32_t holder=0;uint16_t flags=0;QImageResearch d{};
    if(!read(0xA39B64,holder)||!read(holder,t.outer)||!read(t.outer,flags)||!(flags&0x8000)||
        !read(t.outer+0x20,t.native)||!read(t.native,d)||!(d.flags&0x200)||!d.surface||
        d.origin_x||d.origin_y||d.pixel_bits!=16||!MenuCanvas::validSize(d.width,d.height)||
        !read(t.outer+0x10,t.outerW)||!read(t.outer+0x12,t.outerH))return false;
    t.slot=holder;t.surface=d.surface;t.w=d.width;t.h=d.height;return true;
}
bool same(const Target& a,const Target& b) {
    return a.slot==b.slot&&a.outer==b.outer&&a.native==b.native&&a.surface==b.surface&&a.w==b.w&&a.h==b.h&&a.outerW==b.outerW&&a.outerH==b.outerH;
}
bool mainBackgroundCoverage(const Target& t) {
    std::array<int16_t,4> clip{};
    return t.w>=800&&t.h>=600&&read(t.outer+0x46,clip)&&clip[0]<=0&&clip[1]<=0&&
        clip[2]>=0&&clip[3]>=0&&int32_t(clip[0])+clip[2]>=800&&int32_t(clip[1])+clip[3]>=600;
}
bool menuInputFilters(const MenuCanvas::Family& f,uint32_t before,uint32_t after) {
    if(after)return false;
    if(!before)return true;
    // A mission return retains the original default input filter. For these
    // exact menu IDs (>0xA0), 47DDC0 only takes its cursor/default fall-through;
    // it is not an unknown UI owner. Preserve its original invocation and data.
    uint32_t commandMode=1;
    if(before!=0x47DDC0||!read(0x718690,commandMode)||commandMode||f.id<=0xA0)return false;
    for(const auto& b:f.buttons)if(b.id<=0xA0)return false;
    return true;
}
const MenuCanvas::Family* screen(bool rendering=false,uint32_t provedPending=0) {
    uint32_t state=0,pending=0,input=0,secondary=0,draw=0,scene=1,pre=1,post=1;uint16_t mode=0;
    MenuProbe::Node root{},global{};
    if(!read(0x4D0A24,state)||!read(0x4D0A28,pending))return nullptr;
    const auto* f=MenuCanvas::forState(state,mainEnabled);
    if(!f||!MenuCanvas::pendingAllowed(*f,pending,rendering,provedPending))return nullptr;
    const bool valid=
        read(0x713834,input)&&input==0x42A210&&read(0x713838,secondary)&&secondary==0x42A230&&
        read(0x71383C,draw)&&draw==0x42A290&&read(0x713840,scene)&&scene==0&&
        read(0x5F7ED4,pre)&&read(0x5F7ED0,post)&&menuInputFilters(*f,pre,post)&&
        read(0x713DE8,mode)&&mode!=15&&MenuProbe::node(reader,f->root,root)&&
        root.id==f->id&&root.flags==f->flags&&root.x==0&&root.y==0&&root.w==800&&root.h==600&&
        root.parent==0x5F7ED8&&root.next==0&&root.child==f->firstButton+5*0x90&&root.draw==f->draw&&root.input==0&&
        MenuProbe::node(reader,0x5F7ED8,global)&&global.x==0&&global.y==0&&global.flags==4&&
        global.draw==0&&global.parent==0&&global.next==0&&global.child==f->root;
    if(valid&&f==&MenuCanvas::mainMenu) {
        uint32_t resource=0,previous=1;
        if(!read(f->root+0x50,resource)||resource!=0x201||!read(f->root+0x6F,previous)||previous)return nullptr;
    }
    return valid?f:nullptr;
}
int button(const MenuProbe::Node& n,const MenuCanvas::Family* required=nullptr) {
    const auto* f=MenuCanvas::forNode(n.address,mainEnabled);
    if(!f||(required&&required!=f))return -1;
    for(size_t i=0;i<f->buttons.size();++i) {
        const auto& b=f->buttons[i];
        uint16_t mask=0;
        if(n.address==f->firstButton+i*0x90&&n.id==b.id&&n.x==b.x&&n.y==b.y&&n.w==b.w&&n.h==b.h&&
            n.parent==f->root&&n.child==0&&n.next==(i?f->firstButton+(i-1)*0x90:0)&&n.input==f->input&&n.draw==f->buttonDraw&&
            read(n.address+0x46,mask)&&mask==0x4001&&
            (n.flags&~f->dynamicFlags)==1) {
            if(f==&MenuCanvas::mainMenu) {
                uint16_t image=0;uint32_t bank=0,previous=0,counter=0;
                const uint32_t expectedBank=9u|(25u<<8)|(uint32_t(b.hoverRestart)<<16);
                if(!read(n.address+0x10,image)||image!=b.baseImage||!read(n.address+0x50,bank)||bank!=expectedBank||
                    !read(n.address+0x6F,previous)||previous!=(i<5?f->firstButton+uint32_t(i+1)*0x90:0)||
                    !read(n.address+0x54,counter)||counter>=25)return -1;
            }
            return static_cast<int>(i);
        }
    }
    return -1;
}
bool family(bool rendering=false,const MenuCanvas::Family* required=nullptr,uint32_t provedPending=0) {
    const auto* f=screen(rendering,provedPending);
    if(!f||(required&&required!=f))return false;
    for(size_t i=0;i<f->buttons.size();++i){MenuProbe::Node n{};if(!MenuProbe::node(reader,f->firstButton+uint32_t(i)*0x90,n)||button(n,f)!=int(i))return false;}
    return true;
}
struct Pass {
    uint64_t id=0,epoch=0;uint32_t thread=0,stack=0;Target target{};
    bool good=true,background=false,nativeClear=false,published=false;uint32_t visited=0,draws=0;
    const MenuCanvas::Family* family=nullptr;
    uint32_t currentNode=0,departurePending=0,departureNode=0;
    IDirectDrawSurface4* com=nullptr;
    std::shared_ptr<MainBackdrop::Layer> backdrop;std::vector<uint16_t> pixels;QImageResearch image{};
    ~Pass(){if(com)com->lpVtbl->Release(com);}
};
thread_local std::unique_ptr<Pass> active;
std::unique_ptr<MenuCanvas::Plan> plan;
Target lastTarget{};MenuCanvas::PublicationState publication;
const MenuCanvas::Family* lastFamily=nullptr;
struct InputFrame {uint32_t stack;uint64_t generation;};
thread_local std::array<InputFrame,32> inputFrames{};
thread_local size_t inputDepth=0,inputOverflow=0;
void event(const char* kind,const std::string& fields={}) {
    Qfr::event(kind,",\"pass\":"+std::to_string(active?active->id:0)+fields);
}
void reject(const char* reason) {
    publication.invalidate();
    if(active&&active->good){if(active->backdrop)active->backdrop->mainOwns=false;active->good=false;++rejections;event("native_canvas_reject",",\"reason\":"+Qfr::quote(reason));}
}
void start(uint32_t stack,const MenuCanvas::Family* f=&MenuCanvas::selection) {
    // Retain the outer frame on overlap: no inner pass may publish beneath it.
    if(active){reject("overlapping_selection_pass");return;}
    publication.invalidate();
    if(!family(false,f))return;
    Target t;if(!target(t))return;
    if(f==&MenuCanvas::mainMenu&&(t.outerW!=t.w||t.outerH!=t.h||!mainBackgroundCoverage(t)))return;
    auto p=std::make_unique<Pass>();p->id=++nextPass;p->epoch=publication.epoch();p->thread=GetCurrentThreadId();p->stack=stack;p->target=t;p->family=f;p->currentNode=f->root;
    if(f==&MenuCanvas::mainMenu){p->backdrop=MainBackdrop::borrow({t.outer,t.surface,t.w,t.h},p->id);if(p->backdrop)p->backdrop->mainOwns=true;}
    p->pixels.assign(800*600+64,0xA55A);std::fill(p->pixels.begin()+32,p->pixels.end()-32,uint16_t{0});
    p->image.flags=0;p->image.unknown_02=255;p->image.and_mask=255;p->image.pixel_bits=16;
    p->image.width=800;p->image.height=600;p->image.pitch=1600;
    p->image.pixels=static_cast<uint32_t>(reinterpret_cast<uintptr_t>(p->pixels.data()+32));
    p->com=reinterpret_cast<IDirectDrawSurface4*>(t.surface);p->com->lpVtbl->AddRef(p->com);active=std::move(p);
    event("native_canvas_begin",",\"family\":"+Qfr::quote(f->name)+",\"stack\":"+std::to_string(stack)+",\"surface\":"+std::to_string(t.surface));
}
bool renderFamily() {
    if(!active||!family(true,active->family,active->departurePending))return false;
    uint32_t pending=0;if(!read(0x4D0A28,pending))return false;
    if(pending==active->family->state)return true;
    // The original completion handler must have finished restoring sibling
    // disabled state and clearing its activation bit, not just been entered.
    if(active->family!=&MenuCanvas::mainMenu||!active->departureNode)return false;
    for(size_t i=0;i<6;++i) {
        uint32_t flags=0;const auto p=MenuCanvas::mainMenu.firstButton+uint32_t(i)*0x90;
        if(!read(p+4,flags)||(flags&0x10000)||(p==active->departureNode&&(flags&0x100000)))return false;
    }
    return true;
}
void shadow(const uint32_t* args,void* trampoline) {
    if(!active||!active->good)return;
    QImageResearch source{};Target now{};
    if(!renderFamily()||!target(now)||!same(now,active->target)||args[2]!=now.outer||!read(args[1],source)) {
        reject("unsupported_source_destination_or_owner");return;
    }
    if(active->backdrop) {
        const auto result=MainBackdrop::mirror(*active->backdrop,args,trampoline,active->currentNode);
        if(result==MainBackdrop::Mirror::failed){reject("native_background_producer_rejected");return;}
        if(result==MainBackdrop::Mirror::clear)active->nativeClear=true;
        else {++shadowDraws;++active->draws;if(result==MainBackdrop::Mirror::background)active->background=true;}
        return;
    }
    // The original main-menu painter clears the physical target before its
    // opaque logical background. Keep that physical operation unchanged; never
    // mirror its native-sized solid into the smaller logical allocation.
    if(active->family==&MenuCanvas::mainMenu&&source.flags==0&&source.unknown_02==255&&source.pixel_bits==8&&
        source.and_mask==0&&source.xor_value==0&&!source.origin_x&&!source.origin_y&&
        source.width==now.w&&source.height==now.h&&!args[3]&&!args[4]&&
        now.outerW>800&&args[0]==0x4A1774&&active->currentNode==MenuCanvas::mainMenu.root&&
        !active->background&&!active->draws&&!active->nativeClear) {
        active->nativeClear=true;
        event("native_canvas_original_clear",",\"family\":\"main_menu\",\"mirrored\":false");return;
    }
    if(!(source.flags==1||source.flags==3)||source.pixel_bits!=8||
        source.and_mask!=255||source.xor_value!=0||source.surface||!source.width||!source.height||
        uint64_t(source.width)*source.height>800*600||!readable(source.pixels,size_t(source.width)*source.height)) {
        reject("unsupported_source_destination_or_owner");return;
    }
    if(active->family==&MenuCanvas::mainMenu){uint32_t palette=0;
        if(source.unknown_02!=255||!read(0x4EC390,palette)||!readable(palette,512)){reject("main_palette_or_raw_tag_unsupported");return;}}
    const int64_t x=int32_t(args[3])+int64_t(source.origin_x),y=int32_t(args[4])+int64_t(source.origin_y);
    if(x < -32768 || y < -32768 || x+source.width>32767 || y+source.height>32767) {
        reject("legacy_coordinate_overflow");return;
    }
    // Never change the real holder, surface alias, source pixels or palette.
    // Preserve the engine's shared scratch and source pitch around this shadow
    // call. The bridge subsequently performs the original physical draw once.
    constexpr std::array<uint32_t,11> scratch={0xA39B50,0xA39B54,0xA39B58,0xA39B5C,0xA39B60,
        0xA39B68,0xA39B6C,0xA39B70,0xA39B74,0xA39B7C,0xA39D80};
    std::array<uint32_t,11> saved{};for(size_t i=0;i<scratch.size();++i)read(scratch[i],saved[i]);
    uint32_t previousClip=0;read(0xA39B78,previousClip);
    std::array<int16_t,4> local{0,0,800,600},originalClip{};
    const bool outerClip=active->family==&MenuCanvas::mainMenu&&!previousClip;
    if(previousClip||outerClip) {
        if(!read(previousClip?previousClip:now.outer+0x46,originalClip)||originalClip[2]<0||originalClip[3]<0) {reject("unsupported_original_clip");return;}
        const auto intersection=dw::ui::intersect({0,0,800,600},{originalClip[0],originalClip[1],
            int32_t(originalClip[0])+originalClip[2],int32_t(originalClip[1])+originalClip[3]});
        if(intersection)local={static_cast<int16_t>(intersection->left),static_cast<int16_t>(intersection->top),
            static_cast<int16_t>(intersection->right-intersection->left),static_cast<int16_t>(intersection->bottom-intersection->top)};
        else local={0,0,0,0};
    }
    const auto localAddress=static_cast<uint32_t>(reinterpret_cast<uintptr_t>(local.data()));
    write(0xA39B78,localAddress);
    using Blit=uint32_t(__cdecl*)(QImageResearch*,QImageResearch*,int,int);
    reinterpret_cast<Blit>(trampoline)(reinterpret_cast<QImageResearch*>(args[1]),&active->image,int32_t(args[3]),int32_t(args[4]));
    write(args[1]+0x16,source.pitch);write(0xA39B78,previousClip);
    for(size_t i=0;i<scratch.size();++i)write(scratch[i],saved[i]);
    ++shadowDraws;++active->draws;
    if(source.flags==1&&source.width==800&&source.height==600&&!source.origin_x&&!source.origin_y&&
        !args[3]&&!args[4]&&!previousClip&&
        (active->family!=&MenuCanvas::mainMenu||(active->currentNode==active->family->root&&args[0]==0x46AE63&&mainBackgroundCoverage(now))))active->background=true;
}
void complete(uint32_t stack) {
    if(!active||active->thread!=GetCurrentThreadId()||active->stack!=stack)return;
    Target now{};
    if(active->epoch!=publication.epoch())reject("candidate_predates_invalidation");
    if(!renderFamily()||!target(now)||!same(now,active->target))reject("completion_owner_changed");
    if(active->family==&MenuCanvas::mainMenu&&active->target.w>800&&!active->nativeClear)reject("missing_original_native_clear");
    if(!active->background||active->visited!=63||!active->draws)reject("incomplete_selection_producers");
    for(size_t i=0;i<32;++i)if(active->pixels[i]!=0xA55A||active->pixels[active->pixels.size()-1-i]!=0xA55A)reject("logical_buffer_canary");
    if(active->good) {
        DDSURFACEDESC2 d{};d.dwSize=sizeof(d);
        const auto locked=active->com->lpVtbl->Lock(active->com,nullptr,&d,DDLOCK_WAIT,nullptr);
        if(FAILED(locked)){event("native_canvas_lock",",\"hr\":"+std::to_string(locked));reject("physical_lock_failed");}
        else {
            // Construct this guard immediately: subsequent logging and plan
            // allocations can throw. Interface lifetime outlasts the guard.
            MenuCanvas::ScopeUnlock unlock([surface=active->com]{return surface->lpVtbl->Unlock(surface,nullptr);});
            event("native_canvas_lock",",\"hr\":"+std::to_string(locked));
            const bool format=(d.ddpfPixelFormat.dwFlags&DDPF_RGB)&&d.ddpfPixelFormat.dwRGBBitCount==16&&
                d.ddpfPixelFormat.dwRBitMask==0xF800&&d.ddpfPixelFormat.dwGBitMask==0x07E0&&d.ddpfPixelFormat.dwBBitMask==0x001F;
            bool copied=false;
            if(format&&d.dwWidth==now.w&&d.dwHeight==now.h&&d.lPitch>0&&d.lpSurface) {
                if(!plan||!same(lastTarget,now)) {
                    auto next=MenuCanvas::Plan::fit(800,600,d.dwWidth,d.dwHeight);
                    if(next){plan=std::make_unique<MenuCanvas::Plan>(std::move(*next));lastTarget=now;}
                }
                if(foreignThreadSeen.exchange(false))reject("foreign_UI_thread_before_write");
                if(active->epoch!=publication.epoch())reject("candidate_invalidated_before_write");
                if(plan&&active->good){publication.beforeScaledWrite();scaledWriteSeen=true;
                    copied=active->backdrop?MainBackdrop::compose(*active->backdrop,*plan,
                        {d.lpSurface,size_t(d.lPitch)*d.dwHeight,size_t(d.lPitch),d.dwWidth,d.dwHeight}):plan->compose({active->pixels.data()+32,800*600*2,1600,800,600},
                        {d.lpSurface,size_t(d.lPitch)*d.dwHeight,size_t(d.lPitch),d.dwWidth,d.dwHeight});}
            }
            const auto unlocked=unlock.close();
            if(MainBackdrop::drainCoverageGap())coverageLost("foreign_coverage_gap_before_publication");
            if(foreignThreadSeen.exchange(false))reject("foreign_UI_thread_before_publication");
            if(active->epoch!=publication.epoch())reject("candidate_invalidated_before_publication");
            if(!copied)reject("unsupported_physical_target");
            else if(FAILED(unlocked))reject("physical_unlock_failed_after_write");
            else if(!active->good)publication.invalidate();
            else if(!publication.committed(active->id,active->epoch,family(false,active->family)))reject("stale_candidate_commit_refused");
            else {++commits;active->published=true;lastFamily=active->family;
                event("native_canvas_commit",",\"background_scope\":"+std::to_string(active->backdrop?active->backdrop->scope:0)+",\"family\":"+Qfr::quote(active->family->name)+",\"interactive\":"+(publication.scaledPredicatesAllowed()?"true":"false")+
                    ",\"width\":"+std::to_string(d.dwWidth)+",\"height\":"+std::to_string(d.dwHeight)+",\"pitch\":"+std::to_string(d.lPitch)+",\"draws\":"+std::to_string(active->draws));}
        }
    }
    event("native_canvas_complete",",\"committed\":"+std::string(active->published?"true":"false"));active.reset();
}
bool captureOwned(const MenuCanvas::Family* f) {
    uint32_t captured=0;if(!read(0x5F7F6C,captured))return false;
    if(!captured)return true;
    MenuProbe::Node n{};return MenuProbe::node(reader,captured,n)&&button(n,f)>=0;
}
void __stdcall inputObserve(unsigned index,const DWORD* frame) noexcept {
    if(!enabled)return;LastError preserve;
#ifndef MENU_CANVAS_TEST
    OpeningFlow::inputScope(index);
    HudCaptain::inputScope(index);
    HudAssembly::inputScope(index);
#endif
    const auto owner=ownerThread.load();if(!owner)return;if(owner!=GetCurrentThreadId()){foreignThreadSeen=true;return;}
    try {
        if(MainBackdrop::drainCoverageGap())coverageLost("foreign_coverage_gap");
        MenuCanvas::drainForeignInput(publication,foreignThreadSeen);
        const uint32_t sp=frame[3]+4;
        if(index==5) {
            resourceBoundary("main_group_cleanup");return;
        }else if(index==1) {
            if(active)reject("input_reentry_during_active_draw");
            MenuCanvas::inputFrameEntered(publication,inputDepth);
            if(inputDepth==inputFrames.size()){++inputOverflow;publication.invalidate();event("native_canvas_input_gap",",\"reason\":\"input nesting limit\"");return;}
            inputFrames[inputDepth++]={sp,publication.generation()};
        }else if(index==2) {
            if(inputOverflow){--inputOverflow;return;}
            if(!inputDepth||inputFrames[inputDepth-1].stack!=sp+4){publication.invalidate();inputDepth=0;event("native_canvas_input_gap",",\"reason\":\"input frame stack mismatch\"");return;}
            uint32_t capture=0,consumed=0;read(0x5F7F6C,capture);read(0x5F7F94,consumed);
            std::string flags="[";
            uint32_t state=0;read(0x4D0A24,state);const auto* context=MenuCanvas::forState(state,mainEnabled);
            if(context)for(size_t i=0;i<context->buttons.size();++i){uint32_t f=0;read(context->firstButton+4+uint32_t(i)*0x90,f);if(i)flags+=",";flags+=std::to_string(f);}flags+="]";
            event("native_canvas_input_end",",\"depth\":"+std::to_string(inputDepth)+",\"capture\":"+std::to_string(capture)+",\"consumed\":"+std::to_string(consumed)+",\"flags\":"+flags);
            --inputDepth;
        }else if(index==3||index==4) {
            uint32_t args[3]{};if(!read(sp,args))return;
            event("native_canvas_callback",",\"node\":"+std::to_string(args[1])+",\"event_code\":"+std::to_string(args[2]));
#ifndef MENU_CANVAS_TEST
            if(index==4&&OpeningFlow::mainEvent(args[0],args[1],args[2])){reject("modal_completion_handed_to_outer_canvas");return;}
#endif
            if(index==4&&active&&active->family==&MenuCanvas::mainMenu&&args[2]==9999) {
                MenuProbe::Node n{};bool disabled=true;uint32_t counter=0;
                uint32_t expected=MenuCanvas::mainDeparture(args[1],false);
#ifndef MENU_CANVAS_TEST
                if(OpeningFlow::submenuMode())expected=MenuCanvas::mainDeparture(args[1],true);
#endif
                for(size_t i=0;i<6;++i){uint32_t flags=0;const auto p=MenuCanvas::mainMenu.firstButton+uint32_t(i)*0x90;
                    if(!read(p+4,flags)||(p!=args[1]&&!(flags&0x10000)))disabled=false;}
                if(!expected||args[0]!=0x46B4B1||!active->good||active->epoch!=publication.epoch()||
                    active->currentNode!=args[1]||active->departurePending||inputDepth||!disabled||
                    !family(false,&MenuCanvas::mainMenu)||!MenuProbe::node(reader,args[1],n)||
                    button(n,&MenuCanvas::mainMenu)<0||!(n.flags&0x100000)||(n.flags&0x10000)||
                    !read(n.address+0x54,counter)||counter!=8) {
                    reject("unverified_main_menu_completion");return;
                }
                active->departurePending=expected;active->departureNode=args[1];
                event("native_canvas_main_completion",",\"node\":"+std::to_string(args[1])+",\"expected_pending\":"+std::to_string(expected));
            }
        }
    }catch(...){++controllerExceptions;publication.invalidate();}
}
uint32_t __stdcall predicate(const DWORD* frame) noexcept {
    const auto original=static_cast<uint32_t>(reinterpret_cast<uintptr_t>(inputOriginals[0]));
#ifndef MENU_CANVAS_TEST
    uint32_t openingFrame[9]{};std::memcpy(openingFrame,frame,sizeof(openingFrame));
    const auto opening=OpeningFlow::predicate(openingFrame,original);if(opening!=original)return opening;
    const auto captain=HudCaptain::predicate(openingFrame,original);if(captain!=original)return captain;
#endif
    if(!enabled)return original;LastError preserve;
    const auto owner=ownerThread.load();if(!owner)return original;if(owner!=GetCurrentThreadId()) {
        foreignThreadSeen=true;
        const bool known=MenuCanvas::forNode(frame[1],mainEnabled)!=nullptr;
        return known&&scaledWriteSeen.load()?0x41DC62:original;
    }
    try {
        if(MainBackdrop::drainCoverageGap())coverageLost("foreign_coverage_gap");
        MenuCanvas::drainForeignInput(publication,foreignThreadSeen);
        if(active)reject("predicate_reentry_during_active_draw");
        const uint32_t nodeAddress=frame[1],sp=frame[3]+4;
        // Known static family only; all unrelated predicates replay untouched.
        const auto* nodeFamily=MenuCanvas::forNode(nodeAddress,mainEnabled);
        if(!nodeFamily)return original;
        if(publication.originalPredicatesAllowed())return original;
        MenuProbe::Node n{};Target now{};int32_t x=0,y=0,parentX=0,parentY=0;
        const bool admitted=publication.scaledPredicatesAllowed()&&GetCurrentThreadId()==ownerThread.load()&&inputDepth==1&&!inputOverflow&&
            inputFrames[0].generation==publication.generation()&&publication.generation()&&plan&&lastFamily==nodeFamily&&
            family(false,nodeFamily)&&captureOwned(nodeFamily)&&
            target(now)&&same(now,lastTarget)&&MenuProbe::node(reader,nodeAddress,n)&&button(n,nodeFamily)>=0&&
            read(sp+0x14,x)&&read(sp+0x10,y)&&read(sp+0x34,parentX)&&read(sp+0x38,parentY)&&
            parentX==0&&parentY==0;
        bool hit=false;
        if(admitted){const auto q=plan->geometry().inverse_pixel({x,y});hit=q&&dw::ui::contains({n.x,n.y,int64_t(n.x)+n.w,int64_t(n.y)+n.h},*q);}
        uint32_t captured=0,consumed=0;read(0x5F7F6C,captured);read(0x5F7F94,consumed);
        event("native_canvas_predicate",",\"family\":"+Qfr::quote(nodeFamily->name)+",\"generation\":"+std::to_string(publication.generation())+",\"node\":"+std::to_string(nodeAddress)+
            ",\"x\":"+std::to_string(x)+",\"y\":"+std::to_string(y)+",\"admitted\":"+(admitted?"true":"false")+
            ",\"hit\":"+(hit?"true":"false")+",\"capture_before\":"+std::to_string(captured)+",\"consumed_before\":"+std::to_string(consumed));
        // Both destinations retain original blocking, hover and release paths.
        const auto route=MenuCanvas::routeInput(publication,admitted,hit);
        return route==MenuCanvas::InputRoute::Original?original:route==MenuCanvas::InputRoute::Hit?0x41DADC:0x41DC62;
    }catch(...){++controllerExceptions;publication.invalidate();return 0x41DC62;}
}
CANVAS_PREDICATE_BRIDGE(predicateHook,predicate)
MENU_BRIDGE(inputEntryHook,1,4,inputOriginals,inputObserve)
MENU_BRIDGE(inputReturnHook,2,8,inputOriginals,inputObserve)
MENU_BRIDGE(callbackHook,3,12,inputOriginals,inputObserve)
MENU_BRIDGE(mainCallbackHook,4,16,inputOriginals,inputObserve)
MENU_BRIDGE(mainCleanupHook,5,20,inputOriginals,inputObserve)
}
void enable() noexcept {
    LastError preserve;wchar_t value[2]{};
    enabled=Remaster::feature(Remaster::Feature::canvas,L"QFR_UI_CANVAS");
    if(!enabled)return;enabled=false;
    value[0]=0;mainEnabled=Remaster::feature(Remaster::Feature::mainCanvas,L"QFR_UI_MAIN_CANVAS");
    try {
        const size_t count=mainEnabled?6u:4u;
        for(size_t i=0;i<count;++i){const auto& site=canvasSites[i];if(!readable(site.va,site.size)||std::memcmp(reinterpret_cast<void*>(site.va),site.bytes,site.size)) {
            Qfr::event("native_canvas_gate",",\"enabled\":false,\"reason\":\"input bytes mismatch\"");return;
        }}
        if(mainEnabled)for(const auto& site:producerSites)if(!readable(site.va,5)||std::memcmp(reinterpret_cast<void*>(site.va),site.bytes,5)) {
            Qfr::event("native_canvas_gate",",\"enabled\":false,\"reason\":\"main producer bytes mismatch\"");return;
        }
        void* hooks[]={reinterpret_cast<void*>(&predicateHook),reinterpret_cast<void*>(&inputEntryHook),reinterpret_cast<void*>(&inputReturnHook),reinterpret_cast<void*>(&callbackHook),reinterpret_cast<void*>(&mainCallbackHook),reinterpret_cast<void*>(&mainCleanupHook)};
        for(size_t i=0;i<count;++i){inputOriginals[i]=reinterpret_cast<void*>(canvasSites[i].va);const auto before=inputOriginals[i];
            Compat::hookFunction(inputOriginals[i],hooks[i],canvasSites[i].name);if(inputOriginals[i]==before)return;}
        enabled=true;Remaster::ready(Remaster::canvasReady);Qfr::event("native_canvas_mode",",\"schema\":\"quantum-ui-canvas-control-2\",\"logical_width\":800,\"logical_height\":600,\"main_menu_enabled\":"+std::string(mainEnabled?"true":"false")+",\"input\":\"explicit original family rectangles; global coordinates remain physical\",\"shadow_original_draws\":true");
    }catch(...){enabled=false;}
}
void observe(unsigned index,const uint32_t* r,const uint32_t* s,uint32_t sp,void* originalBlit) noexcept {
    if(!enabled)return;LastError preserve;
    auto owner=ownerThread.load();
    if(!owner) {
        if(index!=1||!MenuCanvas::forRoot(r[1],mainEnabled))return;
        ownerThread.compare_exchange_strong(owner,GetCurrentThreadId());owner=ownerThread.load();
    }
    if(owner!=GetCurrentThreadId()){foreignThreadSeen=true;return;}
    try {
        if(MainBackdrop::drainCoverageGap())coverageLost("foreign_coverage_gap");
        if(foreignThreadSeen.exchange(false))reject("foreign_UI_thread_observed");
        if(index==1) {
            const auto* rootFamily=MenuCanvas::forRoot(r[1],mainEnabled);
            if(rootFamily)start(sp,rootFamily);
            else if(active) {MenuProbe::Node node{};const auto i=MenuProbe::node(reader,r[1],node)?button(node,active->family):-1;
                if(i<0)reject("foreign_or_changed_draw_node");else if(active->visited&(1u<<i))reject("duplicate_selection_node");else {active->visited|=1u<<i;active->currentNode=r[1];}}
        }else if(index==2){if(active&&active->currentNode==r[1])active->currentNode=0;}
        else if(index==3)complete(sp);
        else if(index==10)shadow(s,originalBlit);
        else if(active&&(index==8||index==9||index==11))reject("foreign_surface_or_gpu_producer");
    }catch(...){++controllerExceptions;publication.invalidate();if(active){active->good=false;if(active->backdrop){active->backdrop->good=false;active->backdrop->mainOwns=false;}}
        if(index==3&&active&&active->stack==sp)active.reset();}
}
void coverageLost(const char* reason) noexcept {
    if(!enabled)return;LastError preserve;
    const auto owner=ownerThread.load();if(!owner)return;if(owner!=GetCurrentThreadId()){MainBackdrop::coverageGapPending=true;foreignThreadSeen=true;return;}
#ifndef MENU_CANVAS_TEST
    OpeningFlow::resourceBoundary(reason);
#endif
    MainBackdrop::boundary();publication.invalidate();if(active){active->good=false;if(active->backdrop){active->backdrop->good=false;active->backdrop->mainOwns=false;}}
    try{event("native_canvas_coverage_lost",",\"reason\":"+Qfr::quote(reason));}catch(...){++controllerExceptions;}
}
void resourceBoundary(const char* reason) noexcept {
#ifndef MENU_CANVAS_TEST
    OpeningFlow::resourceBoundary(reason);
#endif
    if(!enabled)return;LastError preserve;
    const auto owner=ownerThread.load();if(!owner)return;
    if(owner!=GetCurrentThreadId()){foreignThreadSeen=true;return;}
    publication.invalidate();if(active)active->good=false;
    try{event("native_canvas_resource_boundary",",\"reason\":"+Qfr::quote(reason));}catch(...){++controllerExceptions;}
}
void finish() noexcept {
    if(!enabled)return;LastError preserve;
    try{Qfr::event("native_canvas_end",",\"passes\":"+std::to_string(nextPass)+",\"commits\":"+std::to_string(commits)+",\"rejections\":"+std::to_string(rejections)+",\"shadow_draws\":"+std::to_string(shadowDraws)+",\"controller_exceptions\":"+std::to_string(controllerExceptions)+",\"last_generation\":"+std::to_string(publication.generation())+",\"input_depth\":"+std::to_string(inputDepth)+",\"active_draw\":"+(active?"true":"false")+",\"may_contain_scaled\":"+(publication.mayContainScaledPixels()?"true":"false")+",\"coherent\":"+(publication.scaledPredicatesAllowed()?"true":"false"));}catch(...){}
}
}
