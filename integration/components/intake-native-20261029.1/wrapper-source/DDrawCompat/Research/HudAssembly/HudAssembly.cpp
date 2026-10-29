#include <Remaster/Runtime.h>
#include "HudAssembly.h"
#include "Layout.h"
#include <Research/MenuProbe/Core.h>
#include <Research/FlightRecorder/Recorder.h>
#include <Windows.h>
#include <array>
#include <atomic>
#include <cstring>
#include <map>
#include <memory>
#include <set>
#include <string>
#ifdef HUD_ASSEMBLY_TEST
#include "RuntimeTestMemory.h"
#else
#include <Common/Hook.h>
#include <Research/MenuProbe/Bridge.h>
#include <Research/MenuCanvas/PredicateBridge.h>
#include "Sites.h"
#include <intrin.h>
#endif

namespace HudAssembly {
namespace {
struct LastError{DWORD saved=GetLastError();~LastError(){SetLastError(saved);}};
bool requested=false,enabled=false,ready=false;DWORD ownerThread=0;
std::atomic<bool> foreignBoundary{false};
uint64_t epoch=1,generation=0,gpu=0,cpu=0,errors=0,groups=0,radarPasses=0,previewPasses=0,queued=0,released=0;
uint64_t stripClears=0,radarClears=0,previewClears=0,geometryCommits=0;
unsigned inputDepth=0,radarDepth=0,admissionDepth=0;
bool bytes(uint32_t address,void* out,size_t count){
#ifdef HUD_ASSEMBLY_TEST
    if(MenuCanvasTest::managed(address,count))return MenuCanvasTest::read(address,out,count);
#endif
    if(address<0x10000||!count||count>1024*1024||uint64_t(address)+count>0x100000000ull)return false;
    SIZE_T got=0;return ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(address),out,count,&got)&&got==count;
}
template<class T>bool read(uint32_t a,T& out){return bytes(a,&out,sizeof(out));}
template<class T>bool write(uint32_t a,const T& v){
#ifdef HUD_ASSEMBLY_TEST
    if(MenuCanvasTest::managed(a,sizeof(v)))return MenuCanvasTest::write(a,&v,sizeof(v));
#endif
    SIZE_T got=0;return WriteProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(a),&v,sizeof(v),&got)&&got==sizeof(v);
}
uint32_t value(uint32_t a){uint32_t v=0;read(a,v);return v;}
uint32_t ptr(const void* p){return uint32_t(reinterpret_cast<uintptr_t>(p));}
const MenuProbe::Reader reader=bytes;
std::string n(const char* key,int64_t v){return ",\""+std::string(key)+"\":"+std::to_string(v);}
void trace(const char* event,const std::string& fields={}){Qfr::event(event,n("hud_epoch",epoch)+fields);}
std::set<std::string> reported;
void error(const char* why)noexcept{++errors;try{if(reported.insert(why).second)trace("native_hud_assembly_error",",\"reason\":"+Qfr::quote(why));}catch(...){};}
struct Target{uint32_t outer=0,native=0,surface=0;unsigned w=0,h=0;};
bool target(Target& t){QImageResearch im{};t.outer=value(value(0xA39B64));uint16_t flags=0;
    return read(t.outer,flags)&&(flags&0x8000)&&read(t.outer+0x20,t.native)&&read(t.native,im)&&(im.flags&0x200)&&!im.origin_x&&!im.origin_y&&im.pixel_bits==16&&im.surface&&im.width>=800&&im.height>=600&&
        (t.surface=im.surface)==value(0xA387B4)&&(t.w=im.width)&&(t.h=im.height);}
bool same(const Target&a,const Target&b){return a.outer==b.outer&&a.native==b.native&&a.surface==b.surface&&a.w==b.w&&a.h==b.h;}
bool thread(){return requested&&ownerThread==GetCurrentThreadId();}
bool renderOwner(){const auto s=value(0x4D0A24),d=value(0x71383C);return value(0x713840)==0x4022A0&&
    ((s==3&&d==0x42A7B0)||(s==2&&d==0x42AB00&&value(0x4E3134)==3));}
struct Root{uint32_t address,id,draw,input;};
constexpr Root roots[]={
    {0x4E5E90,0x29,0x47DFE0,0x47DEC0},{0x4EA588,2,0x497890,0},
    {0x4E5F20,0x45,0x47B8E0,0},{0x4E5FB0,0x46,0,0},{0x4E6040,0x44,0x47C6B0,0},
    {0x4E60D0,0x2D,0x47BD10,0},{0x4E6160,0x43,0x47CFB0,0},{0x4DF678,0x65,0,0}};
int rootIndex(uint32_t a){for(unsigned i=0;i<std::size(roots);++i)if(roots[i].address==a)return int(i);return -1;}
int family(uint32_t address){const auto a=MenuProbe::ancestry(reader,address);if(!a.gap.empty())return -1;
    for(const auto& node:a.nodes){const auto i=rootIndex(node.address);if(i>=0&&node.id==roots[i].id&&node.draw==roots[i].draw&&node.input==roots[i].input&&node.parent==0x5F7ED8)return i;}return -1;}
using Viewport=std::array<uint32_t,11>;
struct Snapshot{Layout layout;Target target;std::array<MenuProbe::Node,8> nodes{};std::array<Viewport,4> originals{};float originalP00=0,physicalP00=0;uint64_t epoch=0,id=0;};
Snapshot snapshot;
struct Publication{bool valid=false;uint64_t epoch=0,id=0;};std::array<Publication,8> published{};
struct Frame{bool active=false,good=true;int root=-1;uint32_t stack=0;uint64_t epoch=0,draws=0,gpuPackets=0;};Frame frame;
struct Scope{uint32_t node;int root;};std::vector<Scope> scopes;
struct PointLease{uint32_t stack,node;int root;int32_t x,y;Point point;};std::vector<PointLease> points;
struct Owned{uint32_t context;std::vector<unsigned char> data;uint64_t epoch;};std::map<uint32_t,Owned> retained;
size_t retainedBytes=0;std::map<int,unsigned> inputSamples;
void withdraw(){for(auto& p:published)p.valid=false;}
void drain(){if(foreignBoundary.exchange(false)){++epoch;withdraw();frame.good=false;ready=false;}}
void fail(const char* why){enabled=false;frame.good=false;withdraw();error(why);}
bool live(){if(!thread())return false;drain();Target now{};return enabled&&ready&&snapshot.epoch==epoch&&renderOwner()&&target(now)&&same(now,snapshot.target);}
bool drawing(){return live()&&frame.active&&frame.good&&frame.epoch==epoch&&!scopes.empty()&&scopes.back().root==frame.root;}
void mark(){if(frame.active)++frame.draws;}
bool inputAllowed(int root){if(!live()||inputDepth!=1||frame.active||root<0||!published[size_t(root)].valid||published[size_t(root)].epoch!=epoch||(value(0x7180B6)&255))return false;
    std::set<uint32_t> seen;auto address=value(0x5F7ED8+0x73);
    while(address){MenuProbe::Node node{};if(seen.size()>=64||!seen.insert(address).second||!MenuProbe::node(reader,address,node)||node.parent!=0x5F7ED8)return false;
        if(!(node.flags&8)&&(node.flags&0x80))return false;address=node.next;}return true;}
Viewport mapped(Viewport v,Rect r){v[1]=uint32_t(r.left);v[2]=uint32_t(r.top);v[3]=uint32_t(r.right-r.left);v[4]=uint32_t(r.bottom-r.top);return v;}
void restoreGeometry(){
    if(!snapshot.id)return;
    if(value(0x5FF694)==snapshot.layout.height-uint32_t(snapshot.layout.scene.bottom)&&value(0x5FF698)==snapshot.layout.scene.bottom){
        const auto h=snapshot.layout.height-154;write(0x5FF694,uint32_t{154});write(0x5FF698,h);write(0x5FF6A8,h);write(0x5FF6AC,h/2);write(0x4DC53E,uint16_t(h));
        for(unsigned i=0;i<4;++i)write(0x4F01A8+i*44,snapshot.originals[i]);
    }
    float current=0;if(read(0x4F0308,current)&&current==snapshot.physicalP00)write(0x4F0308,snapshot.originalP00);
    ready=false;withdraw();
}
bool transformVertices(std::vector<unsigned char>& out,uint32_t format,const void* source,uint32_t count){
    const auto textures=(format>>8)&15;
    if((format&0xE)!=4||(format&~0xFC4u)||textures>8||!count||count>16384)return false;
    const size_t stride=16+((format&0x40)?4:0)+((format&0x80)?4:0)+textures*8;
    if(stride*count>1024*1024)return false;out.resize(stride*count);if(!bytes(ptr(source),out.data(),out.size()))return false;
    const auto& tr=snapshot.layout.transform;
    for(unsigned i=0;i<count;++i){float xy[2]{};memcpy(xy,out.data()+i*stride,8);
        if(!std::isfinite(xy[0])||!std::isfinite(xy[1])||xy[0]<-256||xy[0]>1056||xy[1]<int(snapshot.target.h)-600||xy[1]>int(snapshot.target.h)+256)return false;
        xy[0]=float(tr.vertex(xy[0],true));xy[1]=float(tr.vertex(xy[1],false));memcpy(out.data()+i*stride,xy,8);
    }return true;
}
uint32_t pointBefore(const uint32_t* r,uint32_t original){
    if(!thread())return original;LastError preserve;const auto node=r[1];const auto root=family(node);
    if(root<0)return original;
    if(!inputAllowed(root))return 0x41DC62;
    const auto sp=r[3]+4;int32_t x=0,y=0,px=0,py=0;
    if(!read(sp+0x14,x)||!read(sp+0x10,y)||!read(sp+0x34,px)||!read(sp+0x38,py)){fail("input_point_stack_unreadable");return 0x41DC62;}
    const auto p=snapshot.layout.transform.inverse(x+px,y+py);if(!p)return 0x41DC62;
    if(points.size()>=64||std::any_of(points.begin(),points.end(),[&](const auto& q){return q.stack==sp;})){fail("input_point_reentry");return 0x41DC62;}
    points.push_back({sp,node,root,x,y,*p});
    if(!write(sp+0x14,int32_t(p->x-px))||!write(sp+0x10,int32_t(p->y-py))){write(sp+0x14,x);write(sp+0x10,y);points.pop_back();fail("input_point_write_failed");return 0x41DC62;}
    if(inputSamples[root]++<8)trace("native_hud_assembly_input",n("root",roots[root].address)+n("node",node)+n("physical_x",x+px)+n("physical_y",y+py)+n("source_x",p->x)+n("source_y",p->y)+n("shape_flags",value(node+4)&0x400002)+n("buttons",value(0x5F7E00)));
    return original;
}
void pointAfter(uint32_t sp){if(!thread())return;for(auto i=points.begin();i!=points.end();++i)if(i->stack==sp){write(sp+0x14,i->x);write(sp+0x10,i->y);points.erase(i);return;}}
std::optional<Point> radarPoint(){if(!inputAllowed(7))return {};int16_t xy[2]{};if(!read(0x5F7E5C,xy))return {};return snapshot.layout.transform.inverse(xy[0],xy[1]);}
}

void prepareFrame() noexcept {
    if(!thread())return;LastError preserve;drain();
    try{
        if(!renderOwner()){restoreGeometry();return;}
        if(!enabled)return;
        Target t{};uint16_t p=0,q=0,inset=0;
        if(!target(t)||t.w!=value(0x5FF670)||t.h!=value(0x5FF674)||!(value(0x713898)&255)||!read(0x4DF684,p)||!read(0x4DF686,q)||!read(0x4E5FBE,inset))return;
        const auto layout=Layout::fit(t.w,t.h,p,q,inset);if(!layout){fail("layout_dimensions_not_admitted");return;}
        Snapshot next=snapshot;next.layout=*layout;next.target=t;
        for(unsigned i=0;i<std::size(roots);++i){auto& node=next.nodes[i];
            if(!MenuProbe::node(reader,roots[i].address,node)||node.id!=roots[i].id||node.draw!=roots[i].draw||node.input!=roots[i].input||node.parent!=0x5F7ED8||node.w>800||node.h>600){fail("assembly_root_identity_changed");return;}}
        MenuProbe::Node world{};if(!MenuProbe::node(reader,0x4DC530,world)||world.id!=0x63||world.parent!=0x5F7ED8||world.x||world.y){fail("world_input_identity_changed");return;}
        const bool first=!snapshot.id||!same(t,snapshot.target)||snapshot.layout.panelWidth!=layout->panelWidth||snapshot.layout.panelHeight!=layout->panelHeight||snapshot.layout.previewInset!=layout->previewInset;
        if(first){
            for(unsigned i=0;i<4;++i)if(!read(0x4F01A8+i*44,next.originals[i])||next.originals[i][0]!=44){fail("original_viewport_records_unreadable");return;}
            if(!read(0x4F0308,next.originalP00)||!std::isfinite(next.originalP00)||next.originalP00<=0){fail("original_projection_unreadable");return;}
            if(snapshot.id&&next.originalP00==snapshot.physicalP00)next.originalP00=snapshot.originalP00;
            next.originals[0]=mapped(next.originals[0],{0,0,t.w,t.h-154});
            next.originals[1]=mapped(next.originals[1],{int32_t(value(0x5FF6C8)),int32_t(value(0x5FF6CC)),int32_t(value(0x5FF6D0)),int32_t(value(0x5FF6D4))});
            const int px=int(value(0x5FF6E0)),py=int(value(0x5FF6E4));
            next.originals[2]=mapped(next.originals[2],{px,py,px+int(value(0x5FF6D8)),py+int(value(0x5FF6DC))});
        }
        next.physicalP00=float(next.originalP00*layout->projectionFactor());
        const auto sceneH=uint32_t(layout->scene.bottom),reserve=t.h-sceneH;
        if(!write(0x5FF694,reserve)||!write(0x5FF698,sceneH)||!write(0x5FF6A4,t.w)||!write(0x5FF6A8,sceneH)||!write(0x5FF6B0,t.w/2)||!write(0x5FF6AC,sceneH/2)||
            !write(0x4DC53C,uint16_t(t.w))||!write(0x4DC53E,uint16_t(sceneH))||!write(0x4F0308,next.physicalP00)||
            !write(0x4F01A8,mapped(next.originals[0],layout->scene))||!write(0x4F01D4,mapped(next.originals[1],layout->radar))||
            !write(0x4F0200,mapped(next.originals[2],layout->preview))||!write(0x4F022C,mapped(next.originals[3],{0,0,t.w,t.h}))){fail("physical_geometry_commit_failed");return;}
        const bool changed=first||!ready||snapshot.epoch!=epoch;
        next.epoch=epoch;next.id=changed?++generation:snapshot.id;snapshot=next;ready=true;
        if(changed){withdraw();++geometryCommits;uint32_t bits=0;memcpy(&bits,&next.physicalP00,4);trace("native_hud_assembly_layout",n("generation",next.id)+n("surface",t.surface)+n("width",t.w)+n("height",t.h)+n("scale_n",layout->transform.n)+n("scale_d",layout->transform.d)+n("left",layout->transform.dx)+n("scene_height",sceneH)+n("radar_left",layout->radar.left)+n("radar_top",layout->radar.top)+n("radar_right",layout->radar.right)+n("preview_left",layout->preview.left)+n("preview_top",layout->preview.top)+n("preview_right",layout->preview.right)+n("preview_bottom",layout->preview.bottom)+n("projection_p00_bits",bits));}
    }catch(...){fail("prepare_exception");}
}
void observe(unsigned index,const uint32_t* r,const uint32_t*,uint32_t sp) noexcept {
    if(!thread())return;LastError preserve;drain();
    try{
        if(index==1){const auto root=rootIndex(r[1]);
            if(root>=0&&live()){
                if(frame.active){fail("subtree_reentry");return;}
                published[size_t(root)].valid=false;frame={true,true,root,sp,epoch,0,0};scopes.clear();
            }
            if(frame.active)scopes.push_back({r[1],family(r[1])});
        }else if(index==2&&frame.active){if(scopes.empty()||scopes.back().node!=r[1])fail("draw_scope_mismatch");else scopes.pop_back();}
        else if(index==3&&frame.active&&sp==frame.stack){const auto f=frame;frame={};scopes.clear();
            if(f.good&&f.epoch==epoch&&live()){
                published[size_t(f.root)]={true,epoch,snapshot.id};++groups;
                trace("native_hud_assembly_commit",n("generation",snapshot.id)+n("root",roots[f.root].address)+n("draws",f.draws)+n("gpu_packets",f.gpuPackets)+n("state",value(0x4D0A24)));
            }else{published[size_t(f.root)].valid=false;fail("subtree_publication_failed");}
        }else if(drawing()&&(index==9||index==14))fail("unsupported_owned_copy_or_fill");
    }catch(...){fail("observer_exception");}
}
void inputScope(unsigned index) noexcept {
    if(!thread())return;drain();
    if(index==1){if(inputDepth||frame.active){withdraw();frame.good=false;}++inputDepth;}
    else if(index==2){if(inputDepth)--inputDepth;else error("input_scope_unbalanced");if(!inputDepth&&!points.empty()){for(const auto&p:points){write(p.stack+0x14,p.x);write(p.stack+0x10,p.y);}points.clear();fail("input_point_lease_unclosed");}}
}
Draw::Draw(uintptr_t caller,uint32_t format,const void* source,uint32_t count) noexcept {
    if(!thread()||!live()||admissionDepth||retained.count(ptr(source))||(caller>=0x4A61B0&&caller<0x4A6240)||(!drawing()&&!radarDepth))return;LastError preserve;
    try{if(!transformVertices(vertices,format,source,count)){fail("unsupported_owned_gpu_vertices");return;}adapted=true;++gpu;mark();if(frame.active)++frame.gpuPackets;
        if(gpu<=16)trace("native_hud_assembly_gpu",n("root",frame.active?roots[frame.root].address:0x4DF678)+n("format",format)+n("vertices",count)+n("caller",caller));
    }catch(...){fail("gpu_adapter_exception");}
}
void Draw::result(int64_t r) noexcept {if(adapted&&r<0)fail("gpu_draw_failed");}
uint32_t software(Blit original,QImageResearch* source,QImageResearch* dest,int x,int y){
    if(!drawing())return original(source,dest,x,y);LastError preserve;
    const auto unchanged=[&]{SetLastError(preserve.saved);const auto result=original(source,dest,x,y);preserve.saved=GetLastError();return result;};
    QImageResearch src{};auto address=ptr(source);if(!read(address,src)){fail("software_source_unreadable");return unchanged();}
    if(src.flags&0x8000){address=value(address+0x20);if(!read(address,src)){fail("software_inner_unreadable");return unchanged();}}
    if((src.flags!=1&&src.flags!=3)||src.pixel_bits!=8||src.surface||!src.width||!src.height||src.width>1024||src.height>600||(ptr(dest)!=snapshot.target.outer&&ptr(dest)!=snapshot.target.native)){fail("unsupported_software_source_or_target");return unchanged();}
    const int sx=x+src.origin_x,sy=y+src.origin_y;const auto& tr=snapshot.layout.transform;
    const auto box=tr.rect({sx,sy,sx+src.width,sy+src.height});const int w=int(box.right-box.left),h=int(box.bottom-box.top);
    if(w<=0||h<=0||w>8192||h>4096){fail("software_scaled_extent_invalid");return unchanged();}
    std::vector<uint8_t> old(size_t(src.width)*src.height),pixels(size_t(w)*size_t(h));
    if(!bytes(src.pixels,old.data(),old.size())){fail("software_pixels_unreadable");return unchanged();}
    for(int yy=0;yy<h;++yy)for(int xx=0;xx<w;++xx){const int a=std::clamp(tr.cell(int(box.left)+xx,true)-sx,0,int(src.width)-1),b=std::clamp(tr.cell(int(box.top)+yy,false)-sy,0,int(src.height)-1);pixels[size_t(yy)*size_t(w)+xx]=old[size_t(b)*src.width+a];}
    const auto clipAddress=value(0xA39B78);std::array<int16_t,4> clip{};if(!read(clipAddress?clipAddress:snapshot.target.outer+0x46,clip)){fail("software_clip_unreadable");return unchanged();}
    const auto physical=dw::ui::intersect(tr.rect({clip[0],clip[1],int(clip[0])+clip[2],int(clip[1])+clip[3]}),{0,0,snapshot.target.w,snapshot.target.h});
    std::array<int16_t,4> replacement{};if(physical)replacement={int16_t(physical->left),int16_t(physical->top),int16_t(physical->right-physical->left),int16_t(physical->bottom-physical->top)};
    QImageResearch scaled=src;scaled.origin_x=scaled.origin_y=0;scaled.width=uint16_t(w);scaled.height=uint16_t(h);scaled.pitch=int16_t(w);scaled.pixels=ptr(pixels.data());
    struct Restore{uint32_t prior;~Restore(){write(0xA39B78,prior);}}restore{clipAddress};
    if(!write(0xA39B78,ptr(replacement.data()))){fail("software_clip_write_failed");return unchanged();}
    mark();++cpu;if(cpu<=16)trace("native_hud_assembly_software",n("root",roots[frame.root].address)+n("source_width",src.width)+n("source_height",src.height)+n("width",w)+n("height",h));
    SetLastError(preserve.saved);const auto result=original(&scaled,dest,int(box.left),int(box.top));preserve.saved=GetLastError();write(address+0x16,int16_t(src.width));return result;
}
Clear::Clear(uintptr_t caller,uintptr_t viewport,uint32_t count,const D3DRECT* source,uint32_t flags,uint32_t,float z,uint32_t stencil) noexcept {
    if(!live()||(caller!=0x4023F2&&caller!=0x459107&&caller!=0x4027C8))return;LastError preserve;
    try{D3DRECT r{};if(viewport!=value(0xA38C88)||count!=1||!flags||(flags&~3u)||z!=1.0f||stencil||!bytes(ptr(source),&r,sizeof(r))){fail("clear_arguments_not_admitted");return;}
        Rect expected{},destination{};const auto w=snapshot.target.w,h=snapshot.target.h;
        if(caller==0x4023F2){const auto left=value(0x5FF6D0);expected={left,snapshot.layout.scene.bottom,w-left,h};destination=snapshot.layout.reserve;++stripClears;}
        else if(caller==0x459107){expected={int32_t(value(0x5FF6C8)),int32_t(value(0x5FF6CC)),int32_t(value(0x5FF6D0)),int32_t(value(0x5FF6D4))};destination=snapshot.layout.radar;++radarClears;}
        else {const auto x=int32_t(value(0x5FF6E0)),y=int32_t(value(0x5FF6E4));expected={x,y,x+value(0x5FF6D8),y+value(0x5FF6DC)};destination=snapshot.layout.preview;++previewClears;}
        if(r.x1!=expected.left||r.y1!=expected.top||r.x2!=expected.right||r.y2!=expected.bottom){fail("clear_source_rectangle_changed");return;}
        rectangle={LONG(destination.left),LONG(destination.top),LONG(destination.right),LONG(destination.bottom)};adapted=true;
        if(stripClears+radarClears+previewClears<=24)trace("native_hud_assembly_clear",n("caller",caller)+n("left",rectangle.x1)+n("top",rectangle.y1)+n("right",rectangle.x2)+n("bottom",rectangle.y2));
    }catch(...){fail("clear_adapter_exception");}
}
void Clear::result(int64_t r) noexcept {if(adapted&&r<0)fail("clear_failed");}
void boundary(const char* reason) noexcept {if(!requested)return;if(ownerThread!=GetCurrentThreadId()){foreignBoundary=true;return;}++epoch;ready=false;withdraw();frame.good=false;
    try{trace("native_hud_assembly_boundary",",\"reason\":"+Qfr::quote(reason));}catch(...){};}
void finish() noexcept {if(!requested)return;try{trace("native_hud_assembly_end",n("layouts",geometryCommits)+n("groups",groups)+n("gpu_draws",gpu)+n("software_draws",cpu)+n("radar_passes",radarPasses)+n("preview_passes",previewPasses)+n("strip_clears",stripClears)+n("radar_clears",radarClears)+n("preview_clears",previewClears)+n("queued",queued)+n("released",released)+n("retained_bytes",retainedBytes)+n("errors",errors));}catch(...){};}
}

namespace HudAssembly {
namespace {
void* originals[14]{};
using Plain=uint32_t(__cdecl*)(uint32_t,uint32_t,uint32_t,const void*,uint32_t,uint32_t);
using Indexed=uint32_t(__cdecl*)(uint32_t,uint32_t,uint32_t,const void*,uint32_t,const void*,uint32_t,uint32_t);
using Begin=uint32_t(__cdecl*)(uint32_t);
using Radar=uint32_t(__cdecl*)(void*,void*);
using Scene=uint32_t(__cdecl*)(void*,void*,void*);
bool reserveQueued(uint32_t context,std::vector<unsigned char>& vertices,uint32_t& address){
    address=ptr(vertices.data());if(!value(context+0xAC))return false;
    const auto count=vertices.size();retained.emplace(address,Owned{context,std::move(vertices),epoch});retainedBytes+=count;return true;
}
void keepQueued(uint32_t context,uint32_t before,uint32_t address,uint32_t result)noexcept{
    if(!result)return;
    const auto command=value(context+0x10)+before*192;
    if(value(context+0x14)!=before+1||value(command+8)!=address){fail("queued_vertex_ownership_unconfirmed");
        // Keep the copy even when a postcondition fails; it may already be referenced.
    }
    ++queued;
    try{if(queued<=8)trace("native_hud_assembly_queue",n("context",context)+n("command",command)+n("vertices",address)+n("bytes",retained.at(address).data.size()));}catch(...){}
}
uint32_t __cdecl plain(uint32_t context,uint32_t type,uint32_t format,const void* vertices,uint32_t count,uint32_t flags){
    auto original=reinterpret_cast<Plain>(originals[3]);
    if(!live()||admissionDepth||(!radarDepth&&!drawing()))return original(context,type,format,vertices,count,flags);
    LastError preserve;std::vector<unsigned char> owned;
    uint32_t address=0;bool deferred=false;
    try{if(retainedBytes>64*1024*1024||!transformVertices(owned,format,vertices,count)){fail("primitive_admission_not_supported");return original(context,type,format,vertices,count,flags);}deferred=reserveQueued(context,owned,address);}
    catch(...){fail("primitive_copy_allocation_failed");return original(context,type,format,vertices,count,flags);}
    const auto before=value(context+0x14);++admissionDepth;mark();++gpu;
    const auto result=original(context,type,format,reinterpret_cast<const void*>(address),count,flags);preserve.saved=GetLastError();--admissionDepth;
    if(deferred)keepQueued(context,before,address,result);if(!result)fail("owned_primitive_submission_failed");return result;
}
uint32_t __cdecl indexed(uint32_t context,uint32_t type,uint32_t format,const void* vertices,uint32_t count,const void* indices,uint32_t indexCount,uint32_t flags){
    auto original=reinterpret_cast<Indexed>(originals[4]);
    if(!live()||admissionDepth||(!radarDepth&&!drawing()))return original(context,type,format,vertices,count,indices,indexCount,flags);
    LastError preserve;std::vector<unsigned char> owned;
    uint32_t address=0;bool deferred=false;
    try{if(retainedBytes>64*1024*1024||!transformVertices(owned,format,vertices,count)){fail("indexed_admission_not_supported");return original(context,type,format,vertices,count,indices,indexCount,flags);}deferred=reserveQueued(context,owned,address);}
    catch(...){fail("indexed_copy_allocation_failed");return original(context,type,format,vertices,count,indices,indexCount,flags);}
    const auto before=value(context+0x14);++admissionDepth;mark();++gpu;
    const auto result=original(context,type,format,reinterpret_cast<const void*>(address),count,indices,indexCount,flags);preserve.saved=GetLastError();--admissionDepth;
    if(deferred)keepQueued(context,before,address,result);if(!result)fail("owned_indexed_submission_failed");return result;
}
uint32_t __cdecl begin(uint32_t context){
    auto original=reinterpret_cast<Begin>(originals[5]);const auto result=original(context);const auto last=GetLastError();
    if(thread()&&!value(context+0x14)){
        for(auto i=retained.begin();i!=retained.end();){if(i->second.context==context){retainedBytes-=i->second.data.size();i=retained.erase(i);++released;}else ++i;}
    }
    SetLastError(last);return result;
}
uint32_t __cdecl radar(void* device,void* viewport){
    auto original=reinterpret_cast<Radar>(originals[2]);if(!live())return original(device,viewport);
    ++radarDepth;const auto result=original(device,viewport);const auto last=GetLastError();--radarDepth;++radarPasses;SetLastError(last);return result;
}
uint32_t preview(Scene original,uintptr_t caller,void* device,void* viewport,void* scratch){
    if(!live()||caller!=0x4027F2||ptr(device)!=value(0xA38D74)||ptr(viewport)!=value(0xA38C88))return original(device,viewport,scratch);
    LastError preserve;auto* d=reinterpret_cast<IDirect3DDevice3*>(device);D3DMATRIX saved{};
    if(FAILED(d->lpVtbl->GetTransform(d,D3DTRANSFORMSTATE_PROJECTION,&saved))){fail("preview_projection_query_failed");return original(device,viewport,scratch);}
    auto isolated=saved;const float p00=snapshot.originalP00;memcpy(&isolated,&p00,4);
    if(FAILED(d->lpVtbl->SetTransform(d,D3DTRANSFORMSTATE_PROJECTION,&isolated))){fail("preview_projection_bind_failed");return original(device,viewport,scratch);}
    SetLastError(preserve.saved);const auto result=original(device,viewport,scratch);preserve.saved=GetLastError();
    if(FAILED(d->lpVtbl->SetTransform(d,D3DTRANSFORMSTATE_PROJECTION,&saved)))fail("preview_projection_restore_failed");
    ++previewPasses;if(previewPasses<=8){uint32_t bits=0;memcpy(&bits,&p00,4);trace("native_hud_assembly_preview",n("generation",snapshot.id)+n("projection_p00_bits",bits)+n("caller",caller));}
    return result;
}
#ifndef HUD_ASSEMBLY_TEST
uint32_t __cdecl scene(void* device,void* viewport,void* scratch){return preview(reinterpret_cast<Scene>(originals[6]),reinterpret_cast<uintptr_t>(_ReturnAddress()),device,viewport,scratch);}
uint32_t __stdcall hitBefore(const DWORD* r)noexcept{
    const auto original=ptr(originals[0]);try{uint32_t saved[9]{};memcpy(saved,r,sizeof(saved));return pointBefore(saved,original);}catch(...){fail("point_lease_exception");return 0x41DC62;}}
void __stdcall restore(unsigned,const DWORD* r)noexcept{try{pointAfter(r[3]+4);}catch(...){fail("point_restore_exception");}}
template<unsigned Index>uint32_t __stdcall coordinate(const DWORD* saved)noexcept{
    const auto original=ptr(originals[Index]);
    try{const auto point=radarPoint();if(!point)return original;auto* r=const_cast<DWORD*>(saved);
        if constexpr(Index==7)r[6]=(r[6]&0xFFFF0000)|uint16_t(point->x);
        if constexpr(Index==8)r[6]=(r[6]&0xFFFF0000)|uint16_t(point->y);
        if constexpr(Index==9)r[5]=uint32_t(int32_t(point->x));
        if constexpr(Index==10)r[6]=uint32_t(int32_t(point->y));
        if constexpr(Index==11)r[7]=(r[7]&0xFFFF0000)|uint16_t(point->x);
        if constexpr(Index==12)r[7]=(r[7]&0xFFFF0000)|uint16_t(point->y);
        if constexpr(Index==13){r[7]=uint32_t(int32_t(point->x));r[5]=uint32_t(int32_t(point->y));}
        return sites[Index].va+sites[Index].size;
    }catch(...){fail("radar_private_point_exception");return original;}}
CANVAS_PREDICATE_BRIDGE(hitHook,hitBefore)
MENU_BRIDGE(restoreHook,0,4,originals,restore)
#define COORDINATE_HOOK(symbol,index) uint32_t __stdcall symbol##Decide(const DWORD* r)noexcept{return coordinate<index>(r);} CANVAS_PREDICATE_BRIDGE(symbol,symbol##Decide)
COORDINATE_HOOK(cxX,7)
COORDINATE_HOOK(cxY,8)
COORDINATE_HOOK(edxX,9)
COORDINATE_HOOK(ecxY,10)
COORDINATE_HOOK(axX,11)
COORDINATE_HOOK(axY,12)
COORDINATE_HOOK(pairXY,13)
#undef COORDINATE_HOOK
#endif
}
void install() noexcept {
    LastError preserve;if(!Remaster::feature(Remaster::Feature::hud,L"QFR_HUD_ASSEMBLY"))return;
    requested=true;ownerThread=GetCurrentThreadId();
    try{
#ifndef HUD_ASSEMBLY_TEST
        for(const auto& s:sites){std::array<uint8_t,16> actual{};if(!bytes(s.va,actual.data(),s.size)||memcmp(actual.data(),s.bytes,s.size)){error("assembly_hook_bytes_mismatch");return;}}
        void* hooks[]={reinterpret_cast<void*>(&hitHook),reinterpret_cast<void*>(&restoreHook),reinterpret_cast<void*>(&radar),reinterpret_cast<void*>(&plain),reinterpret_cast<void*>(&indexed),reinterpret_cast<void*>(&begin),reinterpret_cast<void*>(&scene),
            reinterpret_cast<void*>(&cxX),reinterpret_cast<void*>(&cxY),reinterpret_cast<void*>(&edxX),reinterpret_cast<void*>(&ecxY),reinterpret_cast<void*>(&axX),reinterpret_cast<void*>(&axY),reinterpret_cast<void*>(&pairXY)};
        for(unsigned i=0;i<std::size(sites);++i){originals[i]=reinterpret_cast<void*>(sites[i].va);const auto before=originals[i];Compat::hookFunction(originals[i],hooks[i],sites[i].name);if(before==originals[i]){error("assembly_hook_install_failed");return;}}
#endif
        enabled=true;Remaster::ready(Remaster::hudReady);trace("native_hud_assembly_mode",",\"schema\":\"quantum-shared-hud-1\",\"roots\":8,\"world_mouse_physical\":true,\"temporary_captain_anchor\":false");
    }catch(...){enabled=false;error("assembly_install_exception");}
}
}
