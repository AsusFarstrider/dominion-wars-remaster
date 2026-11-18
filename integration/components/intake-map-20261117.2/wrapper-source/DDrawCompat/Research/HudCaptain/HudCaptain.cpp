#include <Remaster/Runtime.h>
#include "HudCaptain.h"
#include "Core.h"
#include <Research/MenuProbe/Core.h>
#include <Research/FlightRecorder/Recorder.h>
#include <Windows.h>
#include <array>
#include <atomic>
#include <cstring>
#include <set>
#include <map>
#include <tuple>
#include <string>
#ifdef HUD_CAPTAIN_TEST
#include "RuntimeTestMemory.h"
#endif
namespace HudCaptain {
namespace {
constexpr uint32_t rootAddress=0x4E5FB0;
bool enabled=false,requested=false,mayBeTransformed=false;DWORD thread=0;
std::atomic<bool> foreignBoundary{false};
uint64_t epoch=1,generation=0,groups=0,gpu=0,cpu=0,errors=0;
uint64_t stripAttempts=0,stripClears=0,pausedGroups=0;
struct LastError{DWORD saved=GetLastError();~LastError(){SetLastError(saved);}};
bool bytes(uint32_t a,void* out,size_t n){
#ifdef HUD_CAPTAIN_TEST
    if(MenuCanvasTest::managed(a,n))return MenuCanvasTest::read(a,out,n);
#endif
    if(a<0x10000||!n||n>1024*1024||uint64_t(a)+n>0x100000000ull)return false;
    SIZE_T got=0;return ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(a),out,n,&got)&&got==n;
}
template<class T>bool read(uint32_t a,T& out){return bytes(a,&out,sizeof(out));}
template<class T>bool write(uint32_t a,const T& v){
#ifdef HUD_CAPTAIN_TEST
    if(MenuCanvasTest::managed(a,sizeof(v)))return MenuCanvasTest::write(a,&v,sizeof(v));
#endif
    SIZE_T n=0;return WriteProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(a),&v,sizeof(v),&n)&&n==sizeof(v);
}
uint32_t value(uint32_t a){uint32_t v=0;read(a,v);return v;}
const MenuProbe::Reader reader=bytes;
std::string number(const char* k,int64_t v){return ",\""+std::string(k)+"\":"+std::to_string(v);}
void trace(const char* kind,const std::string& s={}){Qfr::event(kind,number("hud_epoch",epoch)+s);}
std::set<std::string> reported;
std::map<std::tuple<uint32_t,uint32_t,bool>,unsigned> inputSamples;
void error(const char* why) noexcept {++errors;try{if(reported.insert(why).second)trace("native_hud_captain_error",",\"reason\":"+Qfr::quote(why));}catch(...){};}
struct Target{uint32_t outer=0,native=0,surface=0;unsigned w=0,h=0;};
bool target(Target& t){QImageResearch im{};t.outer=value(value(0xA39B64));
    uint16_t flags=0;return read(t.outer,flags)&&(flags&0x8000)&&read(t.outer+0x20,t.native)&&read(t.native,im)&&
        (im.flags&0x200)&&!im.origin_x&&!im.origin_y&&im.pixel_bits==16&&im.surface&&im.width>=800&&im.height>=600&&
        (t.surface=im.surface)==value(0xA387B4)&&(t.w=im.width)&&(t.h=im.height);
}
bool same(const Target& a,const Target& b){return a.outer==b.outer&&a.native==b.native&&a.surface==b.surface&&a.w==b.w&&a.h==b.h;}
bool renderOwner(){const auto state=value(0x4D0A24),draw=value(0x71383C);
    return value(0x713840)==0x4022A0&&((state==3&&draw==0x42A7B0)||(state==2&&draw==0x42AB00&&value(0x4E3134)==3));
}
bool inputOwner(){
    if(!renderOwner()||(value(0x7180B6)&255))return false;
    std::set<uint32_t> seen;auto a=value(0x5F7ED8+0x73);
    while(a){MenuProbe::Node n{};if(seen.size()>=64||!seen.insert(a).second||!MenuProbe::node(reader,a,n)||n.parent!=0x5F7ED8)return false;
        if(!(n.flags&8)&&(n.flags&0x80))return false;a=n.next;
    }return true;
}
bool root(MenuProbe::Node& n){return requested&&thread==GetCurrentThreadId()&&renderOwner()&&
    (value(0x4E5E80)&255)&&MenuProbe::node(reader,rootAddress,n)&&n.id==0x46&&n.w==492&&n.h==51&&
    !n.draw&&!n.input&&n.parent==0x5F7ED8&&!(n.flags&8)&&(n.flags&~0x9F0000u)==4;
}
bool member(uint32_t address){const auto a=MenuProbe::ancestry(reader,address);return a.gap.empty()&&
    std::any_of(a.nodes.begin(),a.nodes.end(),[](const auto& n){return n.address==rootAddress;});}
bool tree(uint32_t a,std::set<uint32_t>& seen,unsigned depth=0){
    if(depth>24)return false;
    while(a){MenuProbe::Node n{};if(seen.size()>=256||!seen.insert(a).second||!MenuProbe::node(reader,a,n))return false;
        if(!(n.flags&8)&&!(n.flags&4)&&(n.flags&(2|0x400000)))return false;
        if(n.child&&!tree(n.child,seen,depth+1))return false;a=n.next;
    }return true;
}
struct Frame{bool active=false,good=true,adapting=false;uint32_t stack=0;uint64_t epoch=0,id=0,draws=0;Target target;Transform transform;
    unsigned hardwareEntries=0,gpuPackets=0;};
Frame frame;
bool originalRepaint=false;
unsigned inputDepth=0;
struct Publication{bool valid=false;uint64_t epoch=0,id=0;Target target;Transform transform;};Publication publication;
std::set<uint32_t> members;
struct Scope{uint32_t node;bool owned;};std::vector<Scope> scopes;
void drain(){if(foreignBoundary.exchange(false)){++epoch;frame.good=false;publication.valid=false;}}
bool drawing(){if(!requested||thread!=GetCurrentThreadId())return false;drain();return enabled&&frame.active&&frame.good&&!frame.adapting&&frame.epoch==epoch&&!scopes.empty()&&scopes.back().owned;}
void fail(const char* why){frame.good=false;publication.valid=false;error(why);}
void beforeWrite(){mayBeTransformed=true;++frame.draws;}
void begin(uint32_t sp){
    publication.valid=false;scopes.clear();MenuProbe::Node n{};Target t{};
    if(!root(n)||!target(t))return;
    if(frame.active){fail("group_reentry");return;}
    originalRepaint=false;
    if(!enabled){frame={true,true,false,sp,epoch,++generation,0,t,{}};originalRepaint=true;return;}
    uint32_t viewport[5]{};if(!read(0x4F01A8,viewport)||viewport[0]!=44||viewport[1]||viewport[2]||viewport[3]!=t.w){error("scene_viewport_not_admitted");return;}
    std::set<uint32_t> nodes;nodes.insert(rootAddress);
    if(!tree(n.child,nodes)){error("unsupported_group_tree_or_hit_shape");enabled=false;return;}
    for(unsigned i=0;i<6;++i){MenuProbe::Node slot{};if(!MenuProbe::node(reader,0x4E7BD0+i*0x90,slot)||slot.id!=0x59+i||slot.x!=int(i*77)||slot.y||slot.w!=77||slot.h!=51||
        slot.draw!=0x47B610||slot.input!=0x47B1A0||slot.parent!=rootAddress){error("slot_identity_not_admitted");return;}}
    int right=0;for(auto a:{0x4E5E90u,0x4E5F20u,0x4E6040u,0x4E60D0u,0x4E6160u}){MenuProbe::Node other{};
        if(!MenuProbe::node(reader,a,other)){error("adjacent_hud_unreadable");return;}
        if(!(other.flags&8)&&other.w&&other.h)right=std::max(right,int(other.x)+other.w);}
    const auto tr=Transform::fit(t.w,t.h,n.x,n.y,right,int(viewport[4]));
    if(!tr){error("no_free_reserved_strip_for_trial");return;}
    members=std::move(nodes);frame={true,true,false,sp,epoch,++generation,0,t,*tr};
    if(groups<4)trace("native_hud_captain_begin",number("generation",generation)+number("surface",t.surface)+number("source_x",n.x)+number("source_y",n.y)+number("destination_x",tr->dx)+number("destination_y",tr->dy)+number("scale_n",tr->n)+number("scale_d",tr->d)+number("members",members.size()));
}
}
void install() noexcept {LastError preserve;if(!Remaster::feature(Remaster::Feature::captainTrial,L"QFR_HUD_CAPTAINS"))return;
    requested=enabled=true;thread=GetCurrentThreadId();try{trace("native_hud_captain_mode",",\"schema\":\"quantum-captain-primitive-trial-1\",\"scene_changed\":false,\"temporary_free_strip_anchor\":true");}catch(...){};
}
void observe(unsigned index,const uint32_t* r,const uint32_t* args,uint32_t sp) noexcept {
    if(!requested||thread!=GetCurrentThreadId())return;LastError preserve;
    drain();
    try{
        if(index==1){if(r[1]==rootAddress)begin(sp);
            if(frame.active)scopes.push_back({r[1],members.count(r[1])&&member(r[1])});
        }else if(index==2&&frame.active){if(scopes.empty()||scopes.back().node!=r[1])fail("callback_scope_mismatch");else scopes.pop_back();}
        else if(index==3&&frame.active&&sp==frame.stack){const auto f=frame;frame={};MenuProbe::Node n{};Target t{};
            if(originalRepaint){originalRepaint=false;if(f.epoch==epoch&&root(n)&&target(t)&&same(t,f.target)){mayBeTransformed=false;publication={};}scopes.clear();return;}
            if(f.good&&f.epoch==epoch&&f.draws&&(!f.hardwareEntries||f.gpuPackets)&&root(n)&&target(t)&&same(t,f.target)&&n.x==f.transform.ox&&n.y==f.transform.oy){
                publication={true,epoch,f.id,t,f.transform};++groups;if(value(0x4D0A24)==2)++pausedGroups;
                trace("native_hud_captain_commit",number("generation",f.id)+number("draws",f.draws)+number("surface",t.surface)+number("hardware_entries",f.hardwareEntries)+number("gpu_packets",f.gpuPackets)+number("state",value(0x4D0A24))+number("draw_owner",value(0x71383C)));
            }else{publication.valid=false;enabled=false;error("group_publication_failed_fallback_next_frame");}
            scopes.clear();
        }else if(drawing()&&index==11&&args[1])++frame.hardwareEntries;
        else if(drawing()&&(index==9||index==14))fail("unsupported_copy_or_fill_producer");
    }catch(...){frame.good=false;publication.valid=false;enabled=false;error("observer_exception");}
}
void inputScope(unsigned index) noexcept {
    if(!requested||thread!=GetCurrentThreadId())return;
    drain();
    if(index==1){if(inputDepth||frame.active){++epoch;publication.valid=false;frame.good=false;}++inputDepth;}
    else if(index==2){if(inputDepth)--inputDepth;else{++epoch;publication.valid=false;error("input_scope_unbalanced");}}
}
Draw::Draw(uint32_t format,const void* source,uint32_t count) noexcept {
    if(!drawing())return;LastError preserve;
    try{
        if(format!=0x1C4||!count||count>256){fail("unsupported_gpu_vertex_format_or_count");return;}
        vertices.resize(size_t(count)*32);if(!bytes(uint32_t(reinterpret_cast<uintptr_t>(source)),vertices.data(),vertices.size())){fail("vertex_span_unreadable");return;}
        uint32_t beforeXY[2]{},afterXY[2]{};memcpy(beforeXY,vertices.data(),8);
        for(unsigned i=0;i<count;++i){float xy[2]{};memcpy(xy,vertices.data()+i*32,8);
            if(!std::isfinite(xy[0])||!std::isfinite(xy[1])||xy[0]<frame.transform.ox-64||xy[0]>frame.transform.ox+640||xy[1]<frame.transform.oy-160||xy[1]>frame.transform.oy+128){fail("vertex_outside_known_group_extent");return;}
            xy[0]=float(frame.transform.vertex(xy[0],true));xy[1]=float(frame.transform.vertex(xy[1],false));memcpy(vertices.data()+i*32,xy,8);
        }
        memcpy(afterXY,vertices.data(),8);beforeWrite();adapted=true;++gpu;++frame.gpuPackets;
        if(gpu<=16)trace("native_hud_captain_gpu",number("generation",frame.id)+number("node",scopes.back().node)+number("vertices",count)+number("format",format)+
            number("source_x_bits",beforeXY[0])+number("source_y_bits",beforeXY[1])+number("sent_x_bits",afterXY[0])+number("sent_y_bits",afterXY[1]));
    }catch(...){fail("gpu_adapter_exception");}
}
void Draw::result(int64_t result) noexcept {if(adapted&&result<0)fail("gpu_draw_failed");}
uint32_t software(Blit original,QImageResearch* source,QImageResearch* dest,int x,int y) {
    if(!drawing())return original(source,dest,x,y);LastError preserve;
    const auto unchanged=[&]{SetLastError(preserve.saved);const auto result=original(source,dest,x,y);preserve.saved=GetLastError();return result;};
    QImageResearch src{};uint32_t address=uint32_t(reinterpret_cast<uintptr_t>(source));
    if(!read(address,src)){fail("software_descriptor_unreadable");return unchanged();}
    if(src.flags&0x8000){address=value(address+0x20);if(!read(address,src)){fail("software_inner_unreadable");return unchanged();}}
    Target t{};
    if((src.flags!=1&&src.flags!=3)||src.pixel_bits!=8||src.surface||!src.width||!src.height||src.width>512||src.height>256||
        !target(t)||!same(t,frame.target)||(uint32_t(reinterpret_cast<uintptr_t>(dest))!=t.outer&&uint32_t(reinterpret_cast<uintptr_t>(dest))!=t.native)) {
        fail("unsupported_software_source_or_target");return unchanged();
    }
    const int sx=x+src.origin_x,sy=y+src.origin_y;const auto tr=frame.transform;
    const auto box=tr.rect({sx,sy,sx+src.width,sy+src.height});const auto w=box.right-box.left,h=box.bottom-box.top;
    if(w<=0||h<=0||w>4096||h>2048){fail("software_scaled_extent_invalid");return unchanged();}
    std::vector<uint8_t> old(size_t(src.width)*src.height),pixels(size_t(w)*size_t(h));
    if(!bytes(src.pixels,old.data(),old.size())){fail("software_pixels_unreadable");return unchanged();}
    for(int yy=0;yy<h;++yy)for(int xx=0;xx<w;++xx){const auto a=std::clamp(tr.cell(int(box.left)+xx,true)-sx,0,int(src.width)-1),b=std::clamp(tr.cell(int(box.top)+yy,false)-sy,0,int(src.height)-1);
        pixels[size_t(yy)*size_t(w)+xx]=old[size_t(b)*src.width+a];}
    const auto clipAddress=value(0xA39B78);std::array<int16_t,4> clip{};
    if(!read(clipAddress?clipAddress:t.outer+0x46,clip)){fail("software_clip_unreadable");return unchanged();}
    const auto physical=dw::ui::intersect(tr.rect({clip[0],clip[1],int(clip[0])+clip[2],int(clip[1])+clip[3]}),{0,0,t.w,t.h});
    std::array<int16_t,4> replacement{};if(physical)replacement={int16_t(physical->left),int16_t(physical->top),int16_t(physical->right-physical->left),int16_t(physical->bottom-physical->top)};
    QImageResearch scaled=src;scaled.origin_x=scaled.origin_y=0;scaled.width=uint16_t(w);scaled.height=uint16_t(h);scaled.pitch=int16_t(w);scaled.pixels=uint32_t(reinterpret_cast<uintptr_t>(pixels.data()));
    struct ClipRestore{uint32_t prior;~ClipRestore(){write(0xA39B78,prior);}} restore{clipAddress};
    if(!write(0xA39B78,uint32_t(reinterpret_cast<uintptr_t>(replacement.data())))){fail("software_clip_write_failed");return unchanged();}
    beforeWrite();++cpu;
    if(cpu<=16)trace("native_hud_captain_software",number("generation",frame.id)+number("node",scopes.back().node)+number("source_width",src.width)+number("source_height",src.height)+number("destination_width",w)+number("destination_height",h));
    const DWORD before=preserve.saved;SetLastError(before);const auto result=original(&scaled,dest,int(box.left),int(box.top));const DWORD after=GetLastError();
    write(address+0x16,int16_t(src.width));preserve.saved=after;return result;
}
uint32_t predicate(const uint32_t* f,uint32_t original) noexcept {
    if(!requested||thread!=GetCurrentThreadId())return original;LastError preserve;
    drain();
    try{
        if(!member(f[1])){
            if(mayBeTransformed&&members.count(f[1])&&renderOwner()){publication.valid=false;return 0x41DC62;}
            return original;
        }
        if(!mayBeTransformed)return original;
        MenuProbe::Node n{};Target t{};MenuProbe::Node rootNode{};
        bool ok=publication.valid&&publication.epoch==epoch&&!frame.active&&inputDepth==1&&root(rootNode)&&inputOwner()&&target(t)&&same(t,publication.target)&&
            rootNode.x==publication.transform.ox&&rootNode.y==publication.transform.oy;
        const auto chain=MenuProbe::ancestry(reader,f[1]);int64_t px=0,py=0;for(size_t i=1;i<chain.nodes.size();++i){px+=chain.nodes[i].x;py+=chain.nodes[i].y;}
        int32_t x=0,y=0,parentX=0,parentY=0;const auto sp=f[3]+4;
        ok=ok&&chain.gap.empty()&&MenuProbe::node(reader,f[1],n)&&!(n.flags&(2|0x400000))&&read(sp+0x14,x)&&read(sp+0x10,y)&&
            read(sp+0x34,parentX)&&read(sp+0x38,parentY)&&parentX==px&&parentY==py;
        if(!ok){publication.valid=false;return 0x41DC62;}
        const auto p=publication.transform.inverse(x+parentX,y+parentY);
        const bool hit=p&&dw::ui::contains({px+n.x,py+n.y,px+n.x+n.w,py+n.y+n.h},*p);
        const auto buttons=value(0x5F7E00);const auto key=std::make_tuple(f[1],buttons,hit);
        if(inputSamples.size()<512&&inputSamples[key]++<4)trace("native_hud_captain_input",number("generation",publication.id)+number("node",f[1])+number("physical_x",x+parentX)+number("physical_y",y+parentY)+
            number("logical_x",p?p->x:-1)+number("logical_y",p?p->y:-1)+number("buttons",buttons)+",\"hit\":"+(hit?"true":"false"));
        return hit?0x41DADC:0x41DC62;
    }catch(...){publication.valid=false;error("predicate_exception");return 0x41DC62;}
}
void boundary(const char* reason) noexcept {if(!requested)return;if(thread!=GetCurrentThreadId()){foreignBoundary=true;return;}++epoch;frame={};originalRepaint=false;scopes.clear();publication.valid=false;
    try{trace("native_hud_captain_boundary",",\"reason\":"+Qfr::quote(reason));}catch(...){};
}
StripClear::StripClear(uintptr_t caller,uintptr_t viewport,uint32_t count,const D3DRECT* original,uint32_t flags,uint32_t color,float z,uint32_t stencil) noexcept {
    if(!requested||thread!=GetCurrentThreadId()||caller!=0x4023F2)return;LastError preserve;
    try{++stripAttempts;Target t{};D3DRECT r{};uint32_t full[5]{},scene[5]{};
        const auto left=value(0x5FF6D0),reserve=value(0x5FF694);
        const bool valid=renderOwner()&&viewport==value(0xA38C88)&&count==1&&flags==1&&!color&&z==1.0f&&!stencil&&
            target(t)&&t.w==value(0x5FF670)&&t.h==value(0x5FF674)&&left<t.w/2&&reserve>0&&reserve<t.h&&
            read(0x4F022C,full)&&full[0]==44&&!full[1]&&!full[2]&&full[3]==t.w&&full[4]==t.h&&
            read(0x4F01A8,scene)&&scene[0]==44&&!scene[1]&&!scene[2]&&scene[3]==t.w&&scene[4]==t.h-reserve&&
            bytes(uint32_t(reinterpret_cast<uintptr_t>(original)),&r,sizeof(r))&&r.x1==int(left)&&r.y1==int(t.h-reserve)&&r.x2==int(t.w-left)&&r.y2==int(t.h);
        if(!valid){error("bottom_strip_clear_not_admitted");return;}
        rectangle=r;rectangle.x2=LONG(t.w);adapted=true;++stripClears;
        if(stripClears<=8)trace("native_hud_captain_strip_clear",number("viewport",viewport)+number("surface",t.surface)+number("left",r.x1)+number("top",r.y1)+number("old_right",r.x2)+number("right",rectangle.x2)+number("bottom",r.y2)+number("state",value(0x4D0A24)));
    }catch(...){error("bottom_strip_adapter_exception");}
}
void StripClear::result(int64_t result) noexcept {if(adapted&&result<0)error("bottom_strip_clear_failed");}
void finish() noexcept {if(!requested)return;try{trace("native_hud_captain_end",number("groups",groups)+number("paused_groups",pausedGroups)+number("gpu_draws",gpu)+number("software_draws",cpu)+number("strip_attempts",stripAttempts)+number("strip_clears",stripClears)+number("errors",errors));}catch(...){};}
}
