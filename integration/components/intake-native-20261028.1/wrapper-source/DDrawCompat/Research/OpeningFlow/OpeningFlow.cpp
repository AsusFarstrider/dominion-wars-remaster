#include <Remaster/Runtime.h>
#include "OpeningFlow.h"
#ifndef OPENING_FLOW_TEST
#include <Research/HudCaptain/HudCaptain.h>
#include <Research/HudAssembly/HudAssembly.h>
#endif
#include "OpeningCore.h"
#include "HdHistory.h"
#include "CardImage.h"
#include <mutex>
#include <Research/MenuProbe/Core.h>
#include <Research/MenuProbe/Bridge.h>
#include <Research/MenuCanvas/ScopeUnlock.h>
#include <Research/MenuCanvas/quantum_font_research.h>
#include <Research/FlightRecorder/Recorder.h>
#include <Common/Hook.h>
#include <Windows.h>
#include <ddraw.h>
#include <bcrypt.h>
#include <intrin.h>
#include <fstream>
#include <filesystem>
#include <iterator>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <stdexcept>
#ifdef OPENING_FLOW_TEST
#include "RuntimeTestMemory.h"
#endif

#include <Research/MenuCanvas/MainBackdrop.h>
namespace OpeningFlow {
namespace {
bool enabled=false;DWORD ownerThread=0;uint64_t epoch=1,serial=0,frames=0,movieCopies=0,errors=0;
bool submenus=false,sliderReady=false;
struct LastError {DWORD value=GetLastError();~LastError(){SetLastError(value);}};
bool readBytes(uint32_t address,void* out,size_t bytes) {
#ifdef OPENING_FLOW_TEST
    if(MenuCanvasTest::managed(address,bytes))return MenuCanvasTest::read(address,out,bytes);
#endif
    if(address<0x10000||!bytes||bytes>4096||uint64_t(address)+bytes>0x100000000ull)return false;
    SIZE_T got=0;return ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(address),out,bytes,&got)&&got==bytes;
}
template<class T>bool read(uint32_t a,T& v){return readBytes(a,&v,sizeof(v));}
bool writeOrigin(uint32_t a,int16_t x,int16_t y) {
    const int16_t xy[]={x,y};
#ifdef OPENING_FLOW_TEST
    if(MenuCanvasTest::managed(a,sizeof(xy)))return MenuCanvasTest::write(a,xy,sizeof(xy));
#endif
    SIZE_T wrote=0;return WriteProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(a),xy,sizeof(xy),&wrote)&&wrote==sizeof(xy);
}
const MenuProbe::Reader reader=readBytes;
std::string n(const char* key,int64_t value){return ",\""+std::string(key)+"\":"+std::to_string(value);}
uint32_t value(uint32_t a){uint32_t v=0;read(a,v);return v;}
void trace(const char* kind,const std::string& fields={}) {Qfr::event(kind,n("opening_epoch",epoch)+n("state",value(0x4D0A24))+n("pending",value(0x4D0A28))+fields);}
std::set<std::string> reported;
void error(const char* why) noexcept {
    ++errors;
    try{const auto key=std::to_string(value(0x4D0A24))+"|"+why;
        if(reported.insert(key).second)trace("native_opening_error",",\"reason\":"+Qfr::quote(why));}catch(...){}
}
bool sameThread(){return enabled&&ownerThread==GetCurrentThreadId();}
struct Spec {uint32_t state,root,id,draw,input;bool movie;
    uint32_t flags=0;uint16_t w=800,h=600;int16_t x=0,y=0;bool outer=false,modal=false;uint32_t underlying=0x4E47C0;};
constexpr Spec specs[]={
    {4,0x4E3138,0x3E9,0x46AAE0,0x46AAC0,true},
    {5,0x4E31C8,0x3E8,0x46AC60,0x46AC40,false},
    {6,0x4E3258,0x3EA,0x46AC60,0x46ADC0,false},
    {8,0x4E4FC0,0x444,0x472360,0x472380,true},
    {9,0x4E4FC0,0x444,0x472360,0x472380,true},
    {11,0x4E3378,0x406,0x46F030,0x46CC40,false},
    {12,0x4C7AB0,0x3EB,0x415B10,0x415EC0,false},
    {15,0x4E57B0,0x4C1,0x474A50,0x474980,false,0,800,600,0,0,true},
    {20,0x4E05E8,0x2007,0x461BF0,0,false,0x84,800,600,0,0,true},
    {21,0x4E0828,0x2011,0x462BD0,0,false,0x84,0,0,0,0,true},
    {22,0x4E1908,0x2026,0x464BB0,0,false,4,0,0,0,0,true},
    {27,0x4E9B78,0,0x488560,0,false,4,0,0,0,0,true},
    {27,0x4DB6F8,0x458,0x4342D0,0,false,0x80,400,450,200,75,true,true,0x4E9B78},
    {7,0x4E47C0,0x405,0x46ADD0,0,false,0x84,800,600,0,0,true},
    {7,0x4DB6F8,0x458,0x4342D0,0,false,0x80,400,450,200,75,true,true},
    {7,0x4DF100,0x47D,0x4342D0,0,false,0x80,600,450,100,75,true,true},
    {1,0x4DA738,0x493,0x434700,0,false,0x80,400,110,200,245,true}};
const Spec* specFor(uint32_t state){
    // Progress painting can occur during several constructor states. Its exact
    // active attached owner, rather than current=pending1, owns this adapter.
    if(submenus&&value(0x5F7ED8+0x73)==0x4DA738&&value(0x7180B5)%256==1)return &specs[std::size(specs)-1];
    for(const auto& s:specs)if(s.state==state&&s.root!=0x4DA738&&(!s.outer||(submenus&&value(0x5F7ED8+0x73)==s.root))&&
    (!s.modal||value(0x5F7ED8+0x73)==s.root)&&(s.root!=0x4DB6F8||sliderReady))return &s;return nullptr;}
bool rootIdentity(const Spec& s,const MenuProbe::Node& root) {
    return root.id==s.id&&(root.flags&~0x1F0000u)==s.flags&&root.w==s.w&&root.h==s.h&&
        root.draw==s.draw&&root.input==s.input&&root.parent==0x5F7ED8;
}
bool directLoadingContext() {
    const auto& s=specs[std::size(specs)-1];MenuProbe::Node root{};
    return value(0x7180B5)%256==1&&MenuProbe::node(reader,s.root,root)&&root.id==s.id&&
        (root.flags&~0x1F0000u)==s.flags&&root.w==s.w&&root.h==s.h&&root.draw==s.draw&&root.input==s.input&&root.x==s.x&&root.y==s.y;
}
bool backdropSibling(uint32_t address,uint32_t expected) {
    if(expected==0x4E9B78) {MenuProbe::Node n{};return address==expected&&MenuProbe::node(reader,address,n)&&n.id==0&&
        (n.flags&~0x1F0000u)==4&&!n.x&&!n.y&&!n.w&&!n.h&&n.draw==0x488560&&!n.input&&!n.next&&n.parent==0x5F7ED8;}
    MenuProbe::Node main{};return address==0x4E47C0&&MenuProbe::node(reader,address,main)&&main.id==0x405&&
        (main.flags&~0x1F0000u)==0x84&&!main.x&&!main.y&&main.w==800&&main.h==600&&
        main.draw==0x46ADD0&&!main.input&&!main.next&&main.parent==0x5F7ED8;
}
const Spec* context(bool stable=true) {
    const auto* s=specFor(value(0x4D0A24));MenuProbe::Node root{},global{};
    const bool loading=s&&s->state==1;
    if(!s||(!loading&&stable&&value(0x4D0A28)!=s->state)||(loading?value(0x7180B5)%256!=1:
       (value(0x713834)!=0x42A210||value(0x713838)!=0x42A230||value(0x71383C)!=0x42A290||value(0x713840)))||!MenuProbe::node(reader,s->root,root)||
       !rootIdentity(*s,root)||root.x!=s->x||root.y!=s->y||(s->modal?!backdropSibling(root.next,s->underlying):root.next!=0)||!MenuProbe::node(reader,0x5F7ED8,global)||global.x||global.y||
       global.flags!=4||global.child!=s->root||global.parent||global.next||global.draw)return nullptr;
    return s;
}
void normalizeSubmenu() {
    const auto* c=specFor(value(0x4D0A24));MenuProbe::Node root{},global{};
    if(!c||!c->outer||!MenuProbe::node(reader,c->root,root)||!rootIdentity(*c,root)||
       !MenuProbe::node(reader,0x5F7ED8,global)||global.child!=c->root||global.flags!=4||
       global.x||global.y||global.parent||global.next||global.draw||(c->modal?!backdropSibling(root.next,c->underlying):root.next!=0))return;
    if(root.x==c->x&&root.y==c->y)return;
    ++epoch;
    if(!writeOrigin(c->root+8,c->x,c->y))error("submenu_origin_write_failed");
    else trace("native_opening_modal_origin",n("root",c->root)+n("old_x",root.x)+n("old_y",root.y)+n("x",c->x)+n("y",c->y));
}
struct Target {uint32_t surface=0;unsigned w=0,h=0;};
bool target(Target& t) {
    uint32_t slot=0,outer=0;QImageResearch image{};uint16_t flags=0;
    if(!read(0xA39B64,slot)||!read(slot,outer)||!read(outer,flags)||!(flags&0x8000)||
       !read(outer+0x20,slot)||!read(slot,image)||!(image.flags&0x200)||image.origin_x||image.origin_y||
       image.pixel_bits!=16||!image.surface||!MenuCanvas::validSize(image.width,image.height)||
       image.width<800||image.height<600||value(0xA387B4)!=image.surface)return false;
    t={image.surface,image.width,image.height};return true;
}
bool format(const DDSURFACEDESC2& d,const Target& t) {
    return d.dwSize==sizeof(d)&&d.dwWidth==t.w&&d.dwHeight==t.h&&d.lPitch>=int(t.w*2)&&d.lpSurface&&
        (d.ddpfPixelFormat.dwFlags&DDPF_RGB)&&d.ddpfPixelFormat.dwRGBBitCount==16&&
        d.ddpfPixelFormat.dwRBitMask==0xF800&&d.ddpfPixelFormat.dwGBitMask==0x7E0&&d.ddpfPixelFormat.dwBBitMask==0x1F&&
        uint64_t(d.lPitch)*d.dwHeight<=256*1024*1024;
}
MenuCanvas::WriteView view(const DDSURFACEDESC2& d){return {d.lpSurface,size_t(d.lPitch)*d.dwHeight,size_t(d.lPitch),d.dwWidth,d.dwHeight};}
std::unique_ptr<MenuCanvas::Plan> plan;unsigned planW=0,planH=0;
bool planFor(const Target& t) {
    if(plan&&planW==t.w&&planH==t.h)return true;
    auto p=MenuCanvas::Plan::fit(800,600,t.w,t.h);if(!p)return false;
    plan=std::make_unique<MenuCanvas::Plan>(std::move(*p));planW=t.w;planH=t.h;return true;
}
struct Publication {uint64_t generation=0,epoch=0;const Spec* spec=nullptr;uint32_t surface=0;bool interactive=false;uint32_t hdHandle=0;uint64_t hdLifetime=0;uint32_t cardResource=0,cardW=0,cardH=0;};
Publication uiPublication,moviePublication;
std::set<uint32_t> uiMembers;
thread_local bool completing=false,copyingVideo=false;
struct Busy {bool& flag;explicit Busy(bool& f):flag(f){flag=true;}~Busy(){flag=false;}};
void invalidateInput(const char* reason) noexcept {
    ++epoch;uiPublication.interactive=false;moviePublication.interactive=false;
    try{trace("native_opening_input_invalidated",",\"reason\":"+Qfr::quote(reason));}catch(...){}
}
struct Surface {uint64_t clear=0;bool scaled=false;};std::map<uintptr_t,Surface> surfaces;
struct Pass {const Spec* spec=nullptr;uint32_t stack=0;uint64_t epoch=0;bool opaque=false,gpu=false;Target target;bool good=true;
    uint32_t node=0,expectedModal=0,completionNode=0;bool modalPainted=false,directLoading=false;std::shared_ptr<MainBackdrop::Layer> backdrop;};
thread_local Pass pass;thread_local unsigned inputDepth=0;
std::unique_ptr<Canvas> uiCanvas;
void* privateOriginals[5]{};
void loadingObserve(unsigned,const DWORD*);
std::map<uint32_t,dw::ui::Point> privatePoints;
int64_t floorDiv(int64_t a,int64_t b){const auto q=a/b;return q-(a%b<0);}
dw::ui::Point extendedPoint(int32_t x,int32_t y) {
    const auto& g=plan->geometry();if(const auto p=g.inverse_pixel({x,y}))return *p;
    const auto scale=g.uniform_scale(),ox=g.offset_x(),oy=g.offset_y();
    // Outside-content drag remains outside the authored canvas. Original
    // handlers retain endpoint clamping and release behavior.
    return {floorDiv(int64_t(x)*ox.denominator-ox.numerator,2*scale.numerator),
        floorDiv(int64_t(y)*oy.denominator-oy.numerator,2*scale.numerator)};
}
bool privatePoint(uint32_t node,uint32_t owner,int32_t x,int32_t y,dw::ui::Point& out) {
    const auto chain=MenuProbe::ancestry(reader,node);
    if(!chain.gap.empty()||std::none_of(chain.nodes.begin(),chain.nodes.end(),[owner](const auto& n){return n.address==owner;}))return false;
    const auto* c=context();Target t{};
    const bool admitted=c&&c->root==owner&&uiMembers.count(node)&&uiPublication.spec==c&&uiPublication.interactive&&
        uiPublication.epoch==epoch&&inputDepth==1&&!pass.spec&&!completing&&!copyingVideo&&plan&&
        target(t)&&t.surface==uiPublication.surface&&t.w==planW&&t.h==planH;
    if(admitted){out=extendedPoint(x,y);privatePoints[node]=out;return true;}
    if(uiPublication.generation&&uiPublication.spec&&uiPublication.spec->root==owner) {
        // The last committed point keeps an existing drag stationary if a
        // publication becomes uncertain. No native-pixel value reaches it.
        const auto p=privatePoints.find(node);out=p==privatePoints.end()?dw::ui::Point{-32768,-32768}:p->second;
        invalidateInput("custom_input_without_coherent_publication");error("custom_input_quarantined");return true;
    }
    return false;
}
void __stdcall privateObserve(unsigned index,const DWORD* saved) noexcept {
    if(!sameThread()||!submenus)return;LastError preserve;
    try {
        if(index>=3){loadingObserve(index,saved);return;}
        auto* f=const_cast<DWORD*>(saved);const auto node=index==2?f[1]:f[0];
        if(index==2) {uint32_t caller=0;if(!read(f[3]+4+0x24,caller)||caller!=0x420083)return;}
        int32_t x=index==2?int32_t(f[6]):int32_t(f[4]);
        const int32_t y=index==0?int32_t(f[2]):index==1?int32_t(f[4]):int32_t(f[7]);
        if(index==1){int16_t globalX=0;if(!read(0x5F7E5C,globalX))return;x=globalX;}
        dw::ui::Point p{};if(!privatePoint(node,index==0?0x4DB6F8:0x4DF100,x,y,p))return;
        if(index==0){f[4]=DWORD(p.x);f[2]=DWORD(p.y);}
        else if(index==1)f[4]=DWORD(p.y);
        else {f[6]=DWORD(p.x);f[7]=DWORD(p.y);}
        trace("native_opening_custom_input",n("kind",index)+n("node",node)+n("physical_x",x)+n("physical_y",y)+n("logical_x",p.x)+n("logical_y",p.y));
    }catch(...){invalidateInput("custom_input_exception");error("custom_input_exception");}
}
MENU_BRIDGE(sliderHook,0,0,privateOriginals,privateObserve)
MENU_BRIDGE(listHook,1,4,privateOriginals,privateObserve)
MENU_BRIDGE(rowHook,2,8,privateOriginals,privateObserve)
MENU_BRIDGE(loadingEntryHook,3,12,privateOriginals,privateObserve)
MENU_BRIDGE(loadingPresentHook,4,16,privateOriginals,privateObserve)
bool installPrivateHooks() {
    struct Site {uint32_t address;unsigned size;uint8_t bytes[12];};
    const Site sites[]={{0x420FC9,5,{0x8B,0x4F,0x77,0x8B,0xC1}},
        {0x420032,6,{0x8B,0x77,0x77,0x8B,0x46,0x42}},
        {0x41FB09,8,{0x89,0x44,0x24,0x14,0x89,0x4C,0x24,0x18}},
        {0x434560,7,{0x8B,0x44,0x24,0x04,0x83,0xEC,0x08}},
        {0x4A79B0,9,{0x8A,0x41,0x04,0x81,0xEC,0x00,0x01,0x00,0x00}}};
    for(const auto& s:sites){uint8_t bytes[12]{};if(!readBytes(s.address,bytes,s.size)||memcmp(bytes,s.bytes,s.size))return false;}
    void* hooks[]={reinterpret_cast<void*>(&sliderHook),reinterpret_cast<void*>(&listHook),reinterpret_cast<void*>(&rowHook),reinterpret_cast<void*>(&loadingEntryHook),reinterpret_cast<void*>(&loadingPresentHook)};
    const char* names[]={"opening_preferences_private_xy","opening_load_private_y","opening_load_row_private_xy","opening_loading_begin","opening_loading_present"};
    for(unsigned i=0;i<5;++i){privateOriginals[i]=reinterpret_cast<void*>(sites[i].address);const auto before=privateOriginals[i];
        Compat::hookFunction(privateOriginals[i],hooks[i],names[i]);if(privateOriginals[i]==before)return false;}
    return true;
}
struct Card {
    unsigned state,resource;const char* name;const wchar_t* file;const char* digest;
    std::shared_ptr<CardImage> image;bool pixelReceipt=false;
};
bool cardsEnabled=false;
Card cards[]={
    {5,0x101,"publisher",L"publisher.dwcard","4b89bd1608a09db212401246b91584fb398275599e91b1c7c575f2e9d31055c5",{},false},
    {6,0x103,"title",L"title.dwcard","3818d7e2a72b634585edfb4134fe64869fce6d7d26757f8e255e24ce79b816bf",{},false}};
Card* cardFor(const Spec& spec) {
    if(!cardsEnabled)return nullptr;
    // Painter timers/clicks can request the next owner before the final paint.
    const auto pending=value(0x4D0A28);
    if(pending!=spec.state&&pending!=spec.state+1)return nullptr;
    for(auto& card:cards)if(card.state==spec.state&&card.image&&value(spec.root+0x50)==card.resource&&
        !value(spec.root+0x73))return &card;
    return nullptr;
}
void loadCards();
void cardPixels(Card& card,const DDSURFACEDESC2& d) {
    if(card.pixelReceipt||!card.image->identity(d.dwWidth,d.dwHeight))return;
    const auto bytes=card.image->bytes();std::vector<unsigned char> packed(bytes);
    for(unsigned y=0;y<d.dwHeight;++y)memcpy(packed.data()+size_t(y)*d.dwWidth*2,
        static_cast<unsigned char*>(d.lpSurface)+size_t(y)*d.lPitch,size_t(d.dwWidth)*2);
    unsigned char a[32]{},b[32]{};
    const bool hashed=BCryptHash(BCRYPT_SHA256_ALG_HANDLE,nullptr,0,static_cast<PUCHAR>(const_cast<void*>(card.image->data())),ULONG(bytes),a,32)>=0&&
        BCryptHash(BCRYPT_SHA256_ALG_HANDLE,nullptr,0,packed.data(),ULONG(bytes),b,32)>=0;
    const bool equal=hashed&&!memcmp(a,b,32);std::string sa,sb;const char* hex="0123456789abcdef";
    for(unsigned i=0;i<32;++i){sa+=hex[a[i]>>4];sa+=hex[a[i]&15];sb+=hex[b[i]>>4];sb+=hex[b[i]&15];}
    trace("native_card_pixels",n("resource",card.resource)+n("bytes",bytes)+",\"source_sha256\":"+Qfr::quote(sa)+
        ",\"target_sha256\":"+Qfr::quote(sb)+",\"equal\":"+(equal?"true":"false"));
    card.pixelReceipt=true;if(!equal)throw std::runtime_error("card identity mismatch");
}

bool supportedTree(uint32_t node,unsigned depth,std::set<uint32_t>& visited) {
    if(depth>24)return false;
    while(node) {
        if(visited.size()>=512||!visited.insert(node).second)return false;
        MenuProbe::Node n{};if(!MenuProbe::node(reader,node,n))return false;
        if(!(n.flags&8)&&(n.flags&(2|0x400000)))return false;
        if(n.child&&!supportedTree(n.child,depth+1,visited))return false;
        node=n.next;
    }
    return true;
}
void complete(uint32_t stack) {
    if(!pass.spec||stack!=pass.stack)return;
    const Pass p=pass;pass={};Target now{};
    const bool mainMonitor=p.spec->root==0x4E47C0;
    if(mainMonitor&&!p.expectedModal)return;
    if(mainMonitor&&p.backdrop&&p.backdrop->composed)return; // Passing Main shadow controller owns ordinary frames.
    const auto ownerValid=[&] {
        const auto* c=context(false);
        if(p.directLoading)return directLoadingContext();
        if(!mainMonitor)return c==p.spec;
        if(p.modalPainted||value(0x4D0A24)!=7||value(0x4D0A28)!=7||!backdropSibling(0x4E47C0,0x4E47C0))return false;
        for(unsigned i=0;i<6;++i){const auto address=0x4E4850+i*0x90,flags=value(address+4);
            if((flags&0x10000)||(address==p.completionNode&&(flags&0x100000)))return false;}
        if(c==p.spec)return true;
        const auto* modal=specFor(7);MenuProbe::Node root{},global{};
        if(!modal||!modal->modal||modal->root!=p.expectedModal||modal->underlying!=0x4E47C0||
            !MenuProbe::node(reader,modal->root,root)||!rootIdentity(*modal,root)||root.next!=0x4E47C0||
            !MenuProbe::node(reader,0x5F7ED8,global)||global.child!=modal->root||global.flags!=4||global.x||global.y||
            global.parent||global.next||global.draw||value(0x713834)!=0x42A210||value(0x713838)!=0x42A230||value(0x71383C)!=0x42A290||value(0x713840))return false;
        // The constructor can have just attached a native-centered modal that
        // this traversal has not painted. Fit only the completed Main pixels;
        // the next original traversal normalizes and paints the new modal.
        return (root.x==modal->x&&root.y==modal->y)||
            (root.x==int(p.target.w-modal->w)/2&&root.y==int(p.target.h-modal->h)/2);
    };
    if(p.epoch!=epoch||!ownerValid()||!target(now)||now.surface!=p.target.surface||now.w!=p.target.w||now.h!=p.target.h) {
        error("screen_changed_before_snapshot");return;
    }
    if(!p.good||!p.opaque){error(p.gpu?"screen_missing_opaque_background_gpu_seen":"screen_missing_opaque_background");return;}
    std::set<uint32_t> nodes;
    if(p.directLoading)nodes.insert(p.spec->root);
    else if(!supportedTree(p.spec->root,0,nodes)){error("screen_contains_unsupported_shape_or_tree");return;}
    auto* card=cardFor(*p.spec);
    Busy busy(completing);
    auto* surface=reinterpret_cast<IDirectDrawSurface4*>(now.surface);surface->lpVtbl->AddRef(surface);
    struct Release {IDirectDrawSurface4* p;~Release(){p->lpVtbl->Release(p);}} release{surface};
    DDSURFACEDESC2 d{};d.dwSize=sizeof(d);const auto hr=surface->lpVtbl->Lock(surface,nullptr,&d,DDLOCK_WAIT,nullptr);
    if(FAILED(hr)){error("snapshot_lock_failed");return;}
    MenuCanvas::ScopeUnlock unlock([surface]{return surface->lpVtbl->Unlock(surface,nullptr);});
    bool copied=false;
    if(format(d,now)&&p.epoch==epoch&&ownerValid()&&(card||planFor(now))) {
        if(p.backdrop&&p.backdrop->good) {
            surfaces[now.surface].scaled=true;uiMembers=std::move(nodes);uiPublication={++serial,epoch,p.spec,now.surface,false};
            copied=MainBackdrop::compose(*p.backdrop,*plan,view(d));
        }else if(p.backdrop&&p.backdrop->nativeWritten) {
            error("native_background_failed_after_write_no_snapshot_fallback");
        }else if(card) {
            surfaces[now.surface].scaled=true;uiMembers=std::move(nodes);uiPublication={++serial,epoch,p.spec,now.surface,false};
            uiPublication.cardResource=card->resource;uiPublication.cardW=now.w;uiPublication.cardH=now.h;
            copied=card->image->compose(view(d));if(copied)cardPixels(*card,d);
        }else {
        if(p.backdrop)trace("native_main_background_legacy_fallback",n("scope",p.backdrop->scope)+",\"fresh_original_repaint\":true");
        if(!uiCanvas)uiCanvas=std::make_unique<Canvas>();
        if(uiCanvas->snapshot({d.lpSurface,size_t(d.lPitch)*d.dwHeight,size_t(d.lPitch),d.dwWidth,d.dwHeight})) {
            // A failed Unlock cannot prove that the attempted fitted write
            // disappeared. Revoke unscaled provenance before the first write.
            surfaces[now.surface].scaled=true;uiMembers=std::move(nodes);uiPublication={++serial,epoch,p.spec,now.surface,false};
            copied=uiCanvas->fit(*plan,view(d));
        }
    }
    }
    const auto result=unlock.close();
    if(MainBackdrop::drainCoverageGap())resourceBoundary("foreign_coverage_gap_before_publication");
    if(!copied||FAILED(result)||p.epoch!=epoch){error("snapshot_copy_unlock_or_epoch_failed");return;}
    surfaces[now.surface].scaled=true;
    uiPublication={++serial,epoch,p.spec,now.surface,!mainMonitor&&!p.directLoading&&context()==p.spec};++frames;
    if(card) {
        uiPublication.cardResource=card->resource;uiPublication.cardW=now.w;uiPublication.cardH=now.h;
        const auto fit=dw::ui::Canvas::fit({0,0,card->image->width(),card->image->height()},{0,0,now.w,now.h});
        const auto bounds=fit->content_bounds();
        trace("native_card_commit",n("generation",serial)+n("root",p.spec->root)+n("resource",card->resource)+n("surface",now.surface)+
            n("source_width",card->image->width())+n("source_height",card->image->height())+n("target_width",now.w)+n("target_height",now.h)+n("target_pitch",d.lPitch)+
            n("left",bounds.left)+n("top",bounds.top)+n("right",bounds.right)+n("bottom",bounds.bottom)+
            ",\"identity\":"+(card->image->identity(now.w,now.h)?"true":"false")+",\"interactive\":"+(uiPublication.interactive?"true":"false"));
    }
    trace("native_opening_screen_commit",n("background_scope",p.backdrop?p.backdrop->scope:0)+n("generation",serial)+n("root",p.spec->root)+n("surface",now.surface)+
        n("width",now.w)+n("height",now.h)+",\"interactive\":"+(uiPublication.interactive?"true":"false")+
        ",\"gpu_producer_observed\":"+(p.gpu?"true":"false"));
}
struct Loading {bool active=false;uint32_t stack=0,clip=0;uint64_t epoch=0;};Loading loading;
bool clearLoading(const Target& t) {
    auto* surface=reinterpret_cast<IDirectDrawSurface4*>(t.surface);
    surface->lpVtbl->AddRef(surface);
    struct Release{IDirectDrawSurface4* p;~Release(){p->lpVtbl->Release(p);}} release{surface};
    DDBLTFX fx{};fx.dwSize=sizeof(fx);fx.dwFillColor=0;
    const auto hr=surface->lpVtbl->Blt(surface,nullptr,nullptr,nullptr,DDBLT_COLORFILL|DDBLT_WAIT,&fx);
    if(FAILED(hr)){error("loading_background_clear_failed");return false;}
    surfaces[t.surface]={++serial,false};return true;
}
void loadingObserve(unsigned index,const DWORD* f) {
    if(index==3) {
        if(loading.active){loading={};pass={};++epoch;error("loading_previous_update_incomplete");}
        if(pass.spec||completing||copyingVideo){error("loading_update_reentry");return;}
        const auto* c=&specs[std::size(specs)-1];MenuProbe::Node root{};Target t{};
        const bool widget=MenuProbe::node(reader,c->root,root)&&root.id==c->id&&(root.flags&~0x1F0000u)==c->flags&&
            root.w==c->w&&root.h==c->h&&root.draw==c->draw&&root.input==c->input&&value(0x7180B5)%256==1;
        const bool validTarget=target(t);
        trace("native_opening_loading_attempt",n("root",c->root)+n("parent",root.parent)+n("next",root.next)+n("head",value(0x5F7ED8+0x73))+
            ",\"static_widget_valid\":"+(widget?"true":"false")+",\"target_valid\":"+(validTarget?"true":"false"));
        if(!widget||!validTarget){error("loading_direct_owner_not_admitted");return;}
        if((root.x!=c->x||root.y!=c->y)&&!writeOrigin(c->root+8,c->x,c->y)){error("loading_origin_write_failed");return;}
        ++epoch;uiPublication.interactive=false;moviePublication.interactive=false;
        if(!clearLoading(t))return;
        loading={true,f[3]+4,value(0xA39B78),epoch};
        pass={c,loading.stack,epoch,true,false,t};pass.directLoading=true;
        trace("native_opening_loading_begin",n("root",c->root)+n("surface",t.surface));
    }else if(index==4&&loading.active) {
        uint32_t caller=0;if(!read(f[3]+4,caller)||caller!=0x4346F8)return;
        const auto prior=loading;loading={};
        if(f[6]!=0xA38770||prior.epoch!=epoch||value(0xA39B78)!=prior.clip) {
            pass={};error("loading_present_owner_or_clip_changed");return;
        }
        const auto beforeFrames=frames;complete(prior.stack);
        trace("native_opening_loading_present",n("progress_bits",value(0x71404C))+n("surface",uiPublication.surface)+
            ",\"fitted\":"+(frames>beforeFrames?"true":"false"));
    }
}
struct Lease {bool active=false,written=false;uintptr_t surface=0,descriptor=0;uint64_t epoch=0,token=0;DDSURFACEDESC2 d{};};
thread_local Lease lease;
bool hdRequested=false,safetyInstalled=false;std::recursive_mutex movieMutex;
struct HdProfile {unsigned resource,frames,rate,divisor,x,y;};
constexpr HdProfile hdProfiles[]={{0x5C5,225,15,1,80,60},{0x5C6,601,2997,100,0,0},{0x5C7,281,30,1,80,60}};
struct Movie {const HdProfile* profile=nullptr;unsigned hdReceipts=0;bool hdBound=false;std::unique_ptr<HdHistory> hd;uint64_t lifetime=0,seedClear=0;std::unique_ptr<Canvas> canvas;bool history=false;MovieGeometry geometry;};
std::map<uint32_t,std::shared_ptr<Movie>> movies;uint64_t lifetimes=0,locks=0;
using Open=void*(__stdcall*)(const void*,uint32_t);using Close=void(__stdcall*)(void*);
Open originalOpen=nullptr;Close originalClose=nullptr;
using Goto=void(__stdcall*)(void*,unsigned,unsigned);Goto originalGoto=nullptr;
bool header(uint32_t p,MovieGeometry& g) {
    return read(p,g.width)&&read(p+4,g.height)&&read(p+0x20,g.handleFlags)&&
        read(p+0x1E4,g.internalWidth)&&read(p+0x1E8,g.internalHeight);
}
const HdProfile* hdProfile(uint32_t p,const MovieGeometry& g) {
    if(g.width!=2560||g.height!=1440||g.internalWidth!=g.width||g.internalHeight!=g.height||
        (g.handleFlags&(0x70000000|0x80000)))return nullptr;
    for(const auto& profile:hdProfiles)if(value(p+8)==profile.frames&&value(p+0x14)==profile.rate&&value(p+0x18)==profile.divisor)return &profile;
    return nullptr;
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
        const bool hd=(hdRequested||Remaster::enabled())&&(!known||large(g));
        if(hd&&(!known||!sameThread()||!safetyInstalled||!hdRequested||!hdProfile(pointer,g)||(Remaster::enabled()&&!Remaster::functionalReady())))reject=true;
        else if(sameThread()) {
            auto m=std::make_shared<Movie>();m->lifetime=++lifetimes;m->geometry=g;
            if(hd){m->profile=hdProfile(pointer,g);m->hd=std::make_unique<HdHistory>(g.width,g.height);}
            movies[pointer]=std::move(m);
            trace(hd?"native_hd_open":"native_opening_video_open",n("handle",pointer)+n("lifetime",lifetimes)+
                n("open_flags",flags)+n("source_width",g.width)+n("source_height",g.height));
        }
    }catch(...){reject=hdRequested;if(sameThread())error("video_open_observation_failed");}
    if(reject){
        {std::lock_guard<std::recursive_mutex> lock(movieMutex);movies.erase(pointer);}
        originalClose(handle);handle=nullptr;
        if(sameThread())error("HD_open_refused_closed_once");
        else Qfr::event("native_hd_open_refused_thread",",\"closed_once\":true");
    }
    SetLastError(after);return handle;
}
void __stdcall binkClose(void* handle) {
    const DWORD before=GetLastError();
    std::lock_guard<std::recursive_mutex> lock(movieMutex);
    try {
        const auto pointer=static_cast<uint32_t>(reinterpret_cast<uintptr_t>(handle));const auto i=movies.find(pointer);
        const auto life=i==movies.end()?0:i->second->lifetime;
        const bool hd=i!=movies.end()&&bool(i->second->hd);
        movies.erase(pointer);if(sameThread())moviePublication={};
        if(sameThread())trace(hd?"native_hd_close":"native_opening_video_close",n("handle",pointer)+n("lifetime",life));
    }catch(...){if(sameThread())error("video_close_observation_failed");}
    SetLastError(before);originalClose(handle);
}
void __stdcall binkGoto(void* handle,unsigned frame,unsigned flags) {
    const DWORD before=GetLastError();
    std::lock_guard<std::recursive_mutex> lock(movieMutex);
    try {
        auto i=movies.find(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(handle)));
        if(i!=movies.end()&&i->second->hd){i->second->hd->reset();
            if(sameThread()){moviePublication={};trace("native_hd_seek",n("frame",frame)+n("flags",flags)+n("lifetime",i->second->lifetime));}}
    }catch(...){if(sameThread())error("HD_seek_observation_failed");}
    SetLastError(before);originalGoto(handle,frame,flags);
}
bool movieOwner(const Spec* s,uint32_t handle,uint32_t& slot,uint32_t& media) {
    if(!s||!s->movie||context()!=s||value(0x5DE6A8)%256!=1||value(0x5DE69C)!=10)return false;
    slot=value(s->state==4?0x4E47B8:0x4E4FBC);if(slot>=4)return false;
    unsigned count=0;
    for(unsigned i=0;i<4;++i)if(value(0x74AA08+i*0x24)==2&&value(0x74AA0C+i*0x24))++count;
    media=value(0x74A9F0+slot*0x24);
    return count==1&&value(0x74AA08+slot*0x24)==2&&value(0x74AA0C+slot*0x24)==handle&&!pass.spec;
}
bool dllIdentity() {
    wchar_t file[32768]{};const auto module=GetModuleHandleW(L"binkw32.dll");
    if(!module||!GetModuleFileNameW(module,file,32768))return false;
    std::ifstream stream(std::filesystem::path(file),std::ios::binary);std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(stream)),{});
    unsigned char digest[32]{};
    const unsigned char expected[]={0x65,0x35,0x8b,0x3c,0xde,0x72,0x81,0xdf,0x10,0xcb,0x68,0xac,0x8e,0x1c,0x9f,0x87,0x05,0x89,0x3a,0x47,0xdf,0x33,0xf8,0x7b,0xb8,0x82,0x07,0x3f,0xd0,0x95,0x5c,0x90};
    return !bytes.empty()&&bytes.size()<16*1024*1024&&BCryptHash(BCRYPT_SHA256_ALG_HANDLE,nullptr,0,bytes.data(),ULONG(bytes.size()),digest,32)>=0&&!memcmp(digest,expected,32);
}
bool fileIdentity(const std::filesystem::path& path,const char* expected) {
    std::ifstream file(path,std::ios::binary);std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(file)),{});
    unsigned char digest[32]{};if(bytes.empty()||bytes.size()>128*1024*1024||
        BCryptHash(BCRYPT_SHA256_ALG_HANDLE,nullptr,0,bytes.data(),ULONG(bytes.size()),digest,32)<0)return false;
    const char* hex="0123456789abcdef";std::string actual;for(auto b:digest){actual+=hex[b>>4];actual+=hex[b&15];}
    return actual==expected;
}

void loadCards() {
    if(!Remaster::feature(Remaster::Feature::cards,L"QFR_OPENING_CARDS"))return;
    wchar_t exe[32768]{};if(!GetModuleFileNameW(nullptr,exe,32768))throw std::runtime_error("card module path");
    const auto base=std::filesystem::path(exe).parent_path()/L"rs/Remaster/OpeningCards";
    for(auto& card:cards) {
        if(!Remaster::cardAllowed(card.resource))continue;
        const auto path=base/card.file;
        if(!fileIdentity(path,card.digest)){error("card_asset_missing_or_identity_mismatch_original_retained");continue;}
        std::ifstream stream(path,std::ios::binary);std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(stream)),{});
        card.image=CardImage::parse(bytes);if(!card.image){error("card_asset_invalid_original_retained");continue;}
        cardsEnabled=true;trace("native_card_asset",n("resource",card.resource)+n("source_width",card.image->width())+n("source_height",card.image->height())+
            ",\"name\":"+Qfr::quote(card.name)+",\"asset_sha256\":"+Qfr::quote(card.digest));
    }
}

}
bool installSafety() noexcept {
    LastError preserve;
    if(!Remaster::feature(Remaster::Feature::hd,L"QFR_HD_MOVIE")&&!Remaster::enabled()) {
        // An experimental HD installation must not become unsafe if launched
        // without its research runner/opt-ins. Refuse DLL load in that case.
        wchar_t path[32768]{};if(!GetModuleFileNameW(nullptr,path,32768))return false;
        for(const auto name:{L"Paramount.bik",L"GizmoLogo.bik",L"SSLogo1.bik"}) {
            std::ifstream file(std::filesystem::path(path).parent_path()/L"rs/Video"/name,std::ios::binary);
            uint32_t fields[7]{};file.read(reinterpret_cast<char*>(fields),sizeof(fields));
            if(file.gcount()==sizeof(fields)&&(fields[5]>800||fields[6]>600))return false;
        }
        return true;
    }
    hdRequested=Remaster::feature(Remaster::Feature::hd,L"QFR_HD_MOVIE");
    try {
        wchar_t path[32768]{};if(!GetModuleFileNameW(nullptr,path,32768)||!dllIdentity())return false;
        if(!fileIdentity(path,"de3e3ede7a9b4ce65a891201547f36413f3a880e4df3f28fb8a81e51f687abc6")||
           (!Remaster::enabled()&&(!fileIdentity(std::filesystem::path(path).parent_path()/L"rs/Video/Paramount.bik",
               "5a3506e3f48190073591c1349f4e35dbfe8dde9d2ded19364fa865817deecff3")||
           !fileIdentity(std::filesystem::path(path).parent_path()/L"rs/Video/GizmoLogo.bik",
               "c65bf6cb39b6838ab76f059152790bd0bf437d513c93b4c7b2bdd81d700dc851"))))return false;
        if(!Remaster::enabled()&&!fileIdentity(std::filesystem::path(path).parent_path()/L"rs/Video/SSLogo1.bik",
                "e444e02185d1fd7d7c7be5edf3bd2842cd43cfe9a89e715a2542ce59c0f7249e")&&
           !fileIdentity(std::filesystem::path(path).parent_path()/L"rs/Video/SSLogo1.bik",
                "910eb6d55755c72c67a557138634fc344fff25560cba7e267899ed9a9904bc50"))return false;
        originalClose=reinterpret_cast<Close>(Compat::hookIatFunction(GetModuleHandleW(nullptr),"_BinkClose@4",reinterpret_cast<void*>(&binkClose)));
        originalGoto=reinterpret_cast<Goto>(Compat::hookIatFunction(GetModuleHandleW(nullptr),"_BinkGoto@12",reinterpret_cast<void*>(&binkGoto)));
        originalOpen=reinterpret_cast<Open>(Compat::hookIatFunction(GetModuleHandleW(nullptr),"_BinkOpen@8",reinterpret_cast<void*>(&binkOpen)));
        safetyInstalled=originalOpen&&originalClose&&originalGoto;
        return safetyInstalled;
    }catch(...){return false;}
}
void install(bool copyReady) noexcept {
    LastError preserve;
    if(!Remaster::feature(Remaster::Feature::opening,L"QFR_OPENING_FLOW"))return;
    try {
        if(!copyReady){error("video_copy_hook_missing");return;}
        if(!dllIdentity()){error("shipped_bink_identity_mismatch");return;}
        if(!safetyInstalled) {
        originalOpen=reinterpret_cast<Open>(Compat::hookIatFunction(GetModuleHandleW(nullptr),"_BinkOpen@8",reinterpret_cast<void*>(&binkOpen)));
        originalClose=reinterpret_cast<Close>(Compat::hookIatFunction(GetModuleHandleW(nullptr),"_BinkClose@4",reinterpret_cast<void*>(&binkClose)));
        }
        if(!originalOpen||!originalClose||(hdRequested&&!safetyInstalled)){error("video_lifetime_import_hook_failed");return;}
        ownerThread=GetCurrentThreadId();enabled=true;loadCards();
        Remaster::MainButtons::install();
        try{MainBackdrop::install();}catch(...){MainBackdrop::enabled=Remaster::MainButtons::available;MainBackdrop::thread=GetCurrentThreadId();MainBackdrop::artwork.reset();error("main_background_asset_unavailable_original_retained");}
        
        if(Remaster::feature(Remaster::Feature::submenus,L"QFR_OPENING_SUBMENUS")) {
            sliderReady=installPrivateHooks();submenus=sliderReady;
            if(!submenus)error("submenu_private_hook_install_failed");
            else trace("native_opening_submenu_mode",",\"schema\":\"quantum-opening-submenus-1\",\"modal_composite\":true,\"private_input\":true");
        }
        if(!Remaster::enabled()||submenus)Remaster::ready(Remaster::openingReady);
        trace("native_opening_mode",",\"schema\":\"quantum-opening-flow-1\",\"logical_width\":800,\"logical_height\":600,\"menu_snapshot\":true,\"movie_history\":true,\"gameplay_hud_changed\":false");
    }catch(...){error("opening_install_exception");}
}
void observe(unsigned index,const uint32_t* r,const uint32_t* s,uint32_t sp,void* originalBlit) noexcept {
    if(!sameThread())return;LastError preserve;
    try {
        if(MainBackdrop::drainCoverageGap()){resourceBoundary("foreign_coverage_gap");return;}
        if(index==0&&s[1]==0x5F7ED8&&submenus) {
            normalizeSubmenu();const auto* c=context(false);Target t{};
            if(c&&c->root==0x4E47C0&&value(0x4D0A28)!=7)c=nullptr;
            if(c&&c->outer) {
                uiPublication.interactive=false;
                if(pass.spec){error("submenu_draw_reentry");pass={};return;}
                if(target(t)) {
                    // Entry is before 421E80 saves EBP/ESI/EDI. Its 421F60
                    // return hook is before those three pops, at entryESP-12.
                    pass={c,sp-12,epoch,false,false,t};
                    if(c->root==0x4E47C0||(c->modal&&c->underlying==0x4E47C0))
                        pass.backdrop=MainBackdrop::begin({value(value(0xA39B64)),t.surface,t.w,t.h},sp);
                    trace("native_opening_screen_begin",n("root",c->root)+n("surface",t.surface)+",\"outer_composite\":true");
                    // Full-screen CPU destinations begin on a defined black
                    // canvas. Modal Main supplies its own original full repaint.
                    if(!c->modal&&c->root!=0x4E47C0) {
                        using Fill=void(__cdecl*)(uint32_t,uint32_t);
                        reinterpret_cast<Fill>(0x4A1710)(value(value(0xA39B64)),0);
                        if(pass.spec==c&&pass.epoch==epoch&&pass.good)pass.opaque=true;
                    }
                }
            }
        }else if(index==1) {
            if(pass.spec){pass.node=r[1];if(r[1]==0x4DB6F8||r[1]==0x4DF100)pass.modalPainted=true;}
            const auto* c=context(false);
            if(c&&!c->outer&&!c->movie&&r[1]==c->root) {
                uiPublication.interactive=false;Target t;
                if(pass.spec){error("opening_draw_reentry");pass={};return;}
                if(target(t)) {
                    pass={c,sp,epoch,false,false,t};trace("native_opening_screen_begin",n("root",c->root)+n("surface",t.surface));
                    // Reuse Resolution's established Brief/Fleet entry repaint.
                    // Their original backgrounds retain transparent pixels;
                    // a fresh defined canvas prevents fitted-frame feedback.
                    if(c->state==11||c->state==12) {
                        const auto outer=value(value(0xA39B64));
                        using Fill=void(__cdecl*)(uint32_t,uint32_t);
                        reinterpret_cast<Fill>(0x4A1710)(outer,0);
                        if(pass.spec==c&&pass.epoch==epoch&&pass.good)pass.opaque=true;
                    }
                }
            }
        }else if(index==2){if(pass.node==r[1])pass.node=0;}
        else if(index==3)complete(sp);
        else if(pass.spec&&index==11){pass.gpu=true;if(pass.backdrop)pass.backdrop->fail("hardware_foreground_not_supported");}
        else if(pass.spec&&index==10) {
            if(pass.backdrop&&!pass.backdrop->mainOwns)MainBackdrop::mirror(*pass.backdrop,s,originalBlit,pass.node);
            QImageResearch source{};uint32_t holder=0,outer=0;std::array<int16_t,4> clip{};
            if(!read(s[1],source)||!read(0xA39B64,holder)||!read(holder,outer)||s[2]!=outer)return;
            const auto cp=value(0xA39B78);const auto clipAddress=cp?cp:outer+0x46;
            if(!read(clipAddress,clip))return;
            if(source.flags==1&&source.width==800&&source.height==600&&!source.origin_x&&!source.origin_y&&!s[3]&&!s[4]&&
               clip[0]<=0&&clip[1]<=0&&int(clip[0])+clip[2]>=800&&int(clip[1])+clip[3]>=600)pass.opaque=true;
        }
    }catch(...){MainBackdrop::boundary();pass={};uiPublication.interactive=false;error("opening_draw_exception");}
}
bool submenuMode() noexcept {return sameThread()&&submenus;}
bool mainEvent(uint32_t caller,uint32_t node,uint32_t event) noexcept {
    if(!submenuMode()||!pass.spec||pass.spec->root!=0x4E47C0||caller!=0x46B4B1||event!=9999)return false;
    LastError preserve;
    try {
        const auto modal=node==0x4E48E0?0x4DF100u:node==0x4E4A00?0x4DB6F8u:0;
        if(!modal||pass.node!=node||pass.epoch!=epoch||inputDepth||context()!=pass.spec||pass.expectedModal)return false;
        MenuProbe::Node selected{};if(!MenuProbe::node(reader,node,selected)||selected.parent!=0x4E47C0||selected.draw!=0x46B440||
            selected.input!=0x46AFA0||!(selected.flags&0x100000)||(selected.flags&0x10000)||value(node+0x54)!=8)return false;
        for(unsigned i=0;i<6;++i)if(0x4E4850+i*0x90!=node&&!(value(0x4E4850+i*0x90+4)&0x10000))return false;
        pass.expectedModal=modal;pass.completionNode=node;
        trace("native_opening_modal_completion",n("node",node)+n("expected_modal",modal));return true;
    }catch(...){error("modal_completion_observation_exception");return false;}
}
void inputScope(unsigned index) noexcept {
    if(!sameThread())return;
    if(MainBackdrop::drainCoverageGap())resourceBoundary("foreign_coverage_gap_before_input");
    if(index==1) {
        if(inputDepth||pass.spec||completing||copyingVideo)invalidateInput("nested_or_draw_reentrant_input");
        ++inputDepth;
    }else if(index==2){if(inputDepth)--inputDepth;else {invalidateInput("unbalanced_input");error("opening_input_unbalanced");}}
}
uint32_t predicate(const uint32_t* frame,uint32_t original) noexcept {
    if(!sameThread())return original;LastError preserve;
    std::lock_guard<std::recursive_mutex> lock(movieMutex);
    try {
        const auto* s=specFor(value(0x4D0A24));if(!s)return original;
        if(s->root==0x4E47C0)return original; // Passing Main controller owns all pure-Main input.
        const auto& publication=s->movie?moviePublication:uiPublication;
        const uint32_t address=frame[1];auto chain=MenuProbe::ancestry(reader,address);
        const auto root=std::find_if(chain.nodes.begin(),chain.nodes.end(),[s](const auto& n){return n.address==(s->modal?0x5F7ED8:s->root);});
        const bool known=s->movie?address==s->root:uiMembers.count(address)!=0;
        if(!chain.gap.empty()||root==chain.nodes.end()) {
            if(publication.generation&&publication.spec==s&&known) {
                if(publication.interactive||completing||pass.spec)invalidateInput("known_member_ancestry_lost");
                return 0x41DC62;
            }
            return original;
        }
        if(!publication.generation||publication.spec!=s) {
            if(publication.spec&&publication.spec->root==0x4E47C0&&s->modal&&s->underlying==0x4E47C0)return 0x41DC62;
            return original;
        }
        bool hd=false;
        if(hdRequested&&s==&specs[0]&&address==s->root) {
            const auto slot=value(0x4E47B8);const auto i=slot<4?movies.find(value(0x74AA0C+slot*0x24)):movies.end();
            hd=i!=movies.end()&&i->second->hd&&!i->second->hd->quarantined&&i->second->hd->initialized;
        }
        const bool nativeCard=address==s->root&&publication.cardResource&&cardFor(*s)&&
            publication.cardResource==value(s->root+0x50);
        bool admitted=known&&publication.interactive&&publication.epoch==epoch&&publication.spec==s&&context()==s&&inputDepth==1&&!pass.spec&&!completing&&!copyingVideo&&(!publication.cardResource||nativeCard);
        Target t{};admitted=admitted&&target(t)&&t.surface==publication.surface&&(hd?(t.w==2560&&t.h==1440):nativeCard?(t.w==publication.cardW&&t.h==publication.cardH):(plan&&planW==t.w&&planH==t.h));
        const auto sp=frame[3]+4;int32_t x=0,y=0,px=0,py=0;const auto& node=chain.nodes.front();
        int64_t accumulatedX=0,accumulatedY=0;
        for(size_t i=1;i<chain.nodes.size();++i){accumulatedX+=chain.nodes[i].x;accumulatedY+=chain.nodes[i].y;}
        admitted=admitted&&!(node.flags&(2|0x400000))&&read(sp+0x14,x)&&read(sp+0x10,y)&&read(sp+0x34,px)&&read(sp+0x38,py)&&px==accumulatedX&&py==accumulatedY;
        if(!admitted&&(publication.interactive||pass.spec||completing||copyingVideo))invalidateInput("predicate_admission_uncertain");
        bool hit=false;
        if(admitted&&(hd||nativeCard))hit=dw::ui::contains({0,0,t.w,t.h},{int64_t(x)+px,int64_t(y)+py});
        else if(admitted) {const auto q=plan->geometry().inverse_pixel({int64_t(x)+px,int64_t(y)+py});
            hit=q&&dw::ui::contains({accumulatedX+node.x,accumulatedY+node.y,accumulatedX+node.x+node.w,accumulatedY+node.y+node.h},*q);}
        trace("native_opening_predicate",n("generation",publication.generation)+n("node",address)+n("parent_x",px)+n("parent_y",py)+n("x",x)+n("y",y)+
            ",\"admitted\":"+(admitted?"true":"false")+",\"hit\":"+(hit?"true":"false"));
        return hit?0x41DADC:0x41DC62;
    }catch(...){invalidateInput("predicate_exception");error("opening_predicate_exception");return 0x41DC62;}
}
void apiBegin(const char* name,uintptr_t object,uintptr_t caller,const uintptr_t* args,size_t count) noexcept {
    if(!sameThread())return;
    if(!strcmp(name,"IDirectDrawSurface4Vtbl::Lock")) {
        lease={};
        if(caller==0x40F2C4&&count>=4&&!args[0]&&args[2]==1&&!args[3])
            lease={false,false,object,args[1],epoch,++locks,{}};
    }
}
void apiResult(const char* name,uintptr_t object,uintptr_t caller,const uintptr_t* args,size_t count,int64_t hr) noexcept {
    if(!sameThread())return;LastError preserve;
    std::lock_guard<std::recursive_mutex> lock(movieMutex);
    try {
        if(hr<0&&pass.spec&&(strstr(name,"::Lock")||strstr(name,"::Unlock")||strstr(name,"::Blt")||strstr(name,"::DrawPrimitive")))pass.good=false;
        if(!strcmp(name,"IDirectDrawSurface4Vtbl::Lock")&&caller==0x40F2C4&&lease.surface==object&&lease.epoch==epoch) {
            Target t{};lease.active=hr>=0&&target(t)&&t.surface==object&&read(static_cast<uint32_t>(lease.descriptor),lease.d)&&format(lease.d,t);
            trace("native_opening_video_lock",n("token",lease.token)+n("surface",object)+n("hr",hr)+",\"valid\":"+(lease.active?"true":"false"));
        }else if(!strcmp(name,"IDirectDrawSurface4Vtbl::Unlock")&&lease.surface==object) {
            if(lease.written) {
                Target t{};const auto* c=context();
                moviePublication.interactive=hr>=0&&caller==0x4A7B4E&&count>=1&&!args[0]&&lease.active&&lease.epoch==epoch&&c&&c==moviePublication.spec&&target(t)&&t.surface==object;
                if(moviePublication.hdHandle) {
                    const auto i=movies.find(moviePublication.hdHandle);
                    moviePublication.interactive=moviePublication.interactive&&i!=movies.end()&&i->second->lifetime==moviePublication.hdLifetime&&
                        i->second->hd&&i->second->hd->initialized&&!i->second->hd->quarantined&&
                        t.w==i->second->hd->width&&t.h==i->second->hd->height;
                    trace("native_hd_unlock",n("lifetime",moviePublication.hdLifetime)+n("token",lease.token)+n("surface",object)+n("hr",hr)+
                        ",\"published\":"+(moviePublication.interactive?"true":"false"));
                }
                trace("native_opening_video_unlock",n("token",lease.token)+n("surface",object)+n("hr",hr)+",\"published\":"+(moviePublication.interactive?"true":"false"));
                if(!moviePublication.interactive)error("video_unlock_or_owner_failed");
            }
            lease={};
        }else if(!strcmp(name,"IDirectDrawSurface4Vtbl::Blt")&&caller==0x4A7A9C&&hr>=0&&count>=5&&!args[1]&&!args[2]&&args[3]==0x01000400) {
            DDBLTFX fx{};RECT rect{};Target t{};
            if(target(t)&&t.surface==object&&read(static_cast<uint32_t>(args[4]),fx)&&fx.dwSize==sizeof(fx)&&fx.dwFillColor==0&&
               (!args[0]||(read(static_cast<uint32_t>(args[0]),rect)&&rect.left==0&&rect.top==0&&rect.right==int(t.w)&&rect.bottom==int(t.h)))) {
                surfaces[object]={++serial,false};moviePublication={};uiPublication={};
                trace("native_opening_native_clear",n("surface",object)+n("clear_generation",serial));
            }
        }
    }catch(...){lease={};moviePublication.interactive=false;error("opening_api_observation_exception");}
}
namespace {
int copyHd(BinkCopy original,uintptr_t caller,void* handle,void* destination,int pitch,unsigned height,unsigned x,unsigned y,unsigned flags) noexcept {
    const DWORD before=GetLastError();DWORD after=before;bool called=false;int result=-1;
    std::shared_ptr<Movie> owned;
    if(!sameThread()) {
        Qfr::event("native_hd_refused_thread",",\"native_writes\":false,\"codec_called\":false");
        SetLastError(before);return -1;
    }
    try {
        std::lock_guard<std::recursive_mutex> lock(movieMutex);
        const auto pointer=static_cast<uint32_t>(reinterpret_cast<uintptr_t>(handle));auto i=movies.find(pointer);
        moviePublication.interactive=false;
        if(!safetyInstalled||copyingVideo||i==movies.end()||!i->second->hd) {
            error("HD_copy_refused_no_safe_owner");SetLastError(before);return -1;
        }
        owned=i->second;auto& history=*owned->hd;MovieGeometry g{};
        if(history.quarantined||!header(pointer,g)||hdProfile(pointer,g)!=owned->profile||!owned->profile||height!=g.height||flags!=10||
           caller!=0x40F2ED||x!=owned->profile->x||y!=owned->profile->y||!history.intact()) {
            history.quarantined=true;error("HD_copy_quarantined_source_or_call");SetLastError(before);return -1;
        }
        const auto* c=context();uint32_t slot=0,media=0;Target t{};
        const auto savedEpoch=epoch,token=lease.token;
        const bool owner=movieOwner(c,pointer,slot,media)&&c==&specs[0]&&media==value(0x749020+4*owned->profile->resource);
        const bool output=owner&&lease.active&&lease.epoch==epoch&&target(t)&&lease.surface==t.surface&&
            lease.d.lpSurface==destination&&lease.d.lPitch==pitch&&format(lease.d,t)&&
            t.w==history.width&&t.h==history.height;
        // A transient lost output cannot redirect decoding into native memory.
        // The known source buffer remains valid independently of that lease.
        if(owner)owned->hdBound=true;
        if(!owned->hdBound){history.quarantined=true;error("HD_copy_quarantined_resource_identity");SetLastError(before);return -1;}
        const auto copyFlags=history.copyFlags();const auto sourceRevision=history.revision;Busy busy(copyingVideo);
        SetLastError(before);called=true;result=original(handle,history.data(),int(history.pitch),height,0,0,copyFlags);after=GetLastError();++movieCopies;
        const auto live=movies.find(pointer);
        const bool source=history.revision==sourceRevision&&history.converted(result)&&live!=movies.end()&&live->second==owned;
        Target current{};const bool publish=source&&output&&epoch==savedEpoch&&lease.active&&lease.token==token&&
            movieOwner(c,pointer,slot,media)&&target(current)&&current.surface==t.surface&&current.w==t.w&&current.h==t.h;
        bool written=false;
        if(publish) {
            // Exact1:1 rows: source history never traverses800x600 or global menu fit.
            written=history.identity(view(lease.d));
            if(written){surfaces[t.surface].scaled=true;moviePublication={++serial,epoch,c,t.surface,false,pointer,owned->lifetime};lease.written=true;
                const auto frame=value(pointer+12);
                if(Qfr::enabled()&&(!owned->hdReceipts||frame==owned->profile->frames/2||(frame>=owned->profile->frames-5&&owned->hdReceipts<3))) {
                    ++owned->hdReceipts;
                    const size_t bytes=size_t(history.pitch)*history.height;
                    std::vector<unsigned char> packed(bytes);
                    for(unsigned row=0;row<history.height;++row)std::memcpy(packed.data()+size_t(row)*history.pitch,
                        static_cast<unsigned char*>(lease.d.lpSurface)+size_t(row)*lease.d.lPitch,history.pitch);
                    unsigned char sourceDigest[32]{},targetDigest[32]{};
                    const bool hashed=BCryptHash(BCRYPT_SHA256_ALG_HANDLE,nullptr,0,static_cast<PUCHAR>(history.data()),ULONG(bytes),sourceDigest,32)>=0&&
                        BCryptHash(BCRYPT_SHA256_ALG_HANDLE,nullptr,0,packed.data(),ULONG(bytes),targetDigest,32)>=0;
                    const bool equal=hashed&&!memcmp(sourceDigest,targetDigest,32);
                    const char* hex="0123456789abcdef";std::string a,b;
                    for(unsigned index=0;index<32;++index){a+=hex[sourceDigest[index]>>4];a+=hex[sourceDigest[index]&15];b+=hex[targetDigest[index]>>4];b+=hex[targetDigest[index]&15];}
                    trace("native_hd_pixels",n("frame",frame)+n("lifetime",owned->lifetime)+n("token",token)+n("bytes",bytes)+
                        ",\"source_sha256\":"+Qfr::quote(a)+",\"target_sha256\":"+Qfr::quote(b)+",\"equal\":"+(equal?"true":"false"));
                    if(!equal){history.quarantined=true;lease.written=false;moviePublication.interactive=false;error("HD_pixel_identity_failed");}
                }
            }
        }
        if(!output)error("HD_output_withheld_invalid_native_lease");
        if(history.quarantined)error("HD_copy_result_or_guard_failed");
        trace("native_hd_copy",n("external_resource",owned->profile->resource)+n("declared_frames",owned->profile->frames)+n("rate",owned->profile->rate)+n("rate_divisor",owned->profile->divisor)+n("handle",pointer)+n("lifetime",owned->lifetime)+n("slot",slot)+n("media_slot_value",media)+n("frame",value(pointer+12))+
            n("token",token)+n("source_width",g.width)+n("source_height",g.height)+n("internal_width",g.internalWidth)+n("internal_height",g.internalHeight)+
            n("source_pitch",history.pitch)+n("source_x",0)+n("source_y",0)+n("original_x",x)+n("original_y",y)+n("copy_flags",copyFlags)+n("result",result)+
            n("target_width",t.w)+n("target_height",t.h)+n("target_pitch",lease.d.lPitch)+n("surface",lease.surface)+
            ",\"history_valid\":"+(history.initialized?"true":"false")+",\"guards_intact\":"+(history.intact()?"true":"false")+
            ",\"identity_written\":"+(written?"true":"false")+",\"native_writes_withheld\":"+(!written?"true":"false"));
        SetLastError(after);return result;
    }catch(...) {
        if(owned&&owned->hd)owned->hd->quarantined=true;
        moviePublication.interactive=false;lease.written=false;error("HD_provider_exception_no_legacy_fallback");SetLastError(called?after:before);return called?result:-1;
    }
}
}

int copy(BinkCopy original,uintptr_t caller,void* handle,void* destination,int pitch,unsigned height,unsigned x,unsigned y,unsigned flags) {
    {
        // Header probing is bounded even on a foreign thread. Failure refuses
        // rather than guessing that this is safe legacy artwork.
        MovieGeometry g{};const auto pointer=static_cast<uint32_t>(reinterpret_cast<uintptr_t>(handle));
        const DWORD before=GetLastError();const bool known=header(pointer,g);SetLastError(before);
        if(!known||large(g))return copyHd(original,caller,handle,destination,pitch,height,x,y,flags);
        std::lock_guard<std::recursive_mutex> lock(movieMutex);
        const auto i=movies.find(pointer);
        if(i!=movies.end()&&i->second->hd)return copyHd(original,caller,handle,destination,pitch,height,x,y,flags);
    }
    if(!sameThread())return original(handle,destination,pitch,height,x,y,flags);
    const DWORD before=GetLastError();bool called=false;int result=0;DWORD after=before;
    try {
        const auto pointer=static_cast<uint32_t>(reinterpret_cast<uintptr_t>(handle));auto i=movies.find(pointer);const auto* c=context();
        uint32_t slot=0,media=0;MovieGeometry geometry{};Target t{};
        const bool header=read(pointer,geometry.width)&&read(pointer+4,geometry.height)&&read(pointer+0x20,geometry.handleFlags)&&
            read(pointer+0x1E4,geometry.internalWidth)&&read(pointer+0x1E8,geometry.internalHeight);
        const bool valid=caller==0x40F2ED&&i!=movies.end()&&movieOwner(c,pointer,slot,media)&&header&&geometry.supports(height,x,y,flags)&&
            value(0x5DE6A0)==x&&value(0x5DE6A4)==y&&lease.active&&lease.epoch==epoch&&target(t)&&lease.surface==t.surface&&
            lease.d.lpSurface==destination&&lease.d.lPitch==pitch&&format(lease.d,t)&&planFor(t);
        if(!valid) {
            if(i!=movies.end()&&lease.active&&lease.d.lpSurface==destination&&surfaces[lease.surface].scaled&&i->second->history&&context(false)&&context(false)->movie) {
                if(!i->second->canvas->legacy(view(lease.d)))error("video_legacy_recovery_failed");else surfaces[lease.surface].scaled=false;
            }
            error("video_copy_not_admitted");SetLastError(before);return original(handle,destination,pitch,height,x,y,flags);
        }
        const auto ownedMovie=i->second;auto& movie=*ownedMovie;auto& storage=surfaces[t.surface];
        if(!movie.history||movie.seedClear!=storage.clear) {
            if(storage.scaled){error("video_unscaled_seed_unavailable");SetLastError(before);return original(handle,destination,pitch,height,x,y,flags);}
            movie.canvas=std::make_unique<Canvas>();
            if(!movie.canvas->snapshot({destination,size_t(pitch)*lease.d.dwHeight,size_t(pitch),lease.d.dwWidth,lease.d.dwHeight}))throw std::runtime_error("movie seed");
            movie.history=true;movie.seedClear=storage.clear;movie.geometry=geometry;
        }
        const auto token=lease.token,life=movie.lifetime,savedEpoch=epoch;
        Busy busy(copyingVideo);
        SetLastError(before);called=true;result=original(handle,movie.canvas->data(),1600,height,x,y,flags);after=GetLastError();++movieCopies;
        const auto stillLive=movies.find(pointer);
        if((result!=0&&result!=1)||!movie.canvas->intact()||stillLive==movies.end()||stillLive->second!=ownedMovie||!lease.active||lease.token!=token||epoch!=savedEpoch||!movieOwner(c,pointer,slot,media)||
           !target(t)||t.surface!=lease.surface) {
            error("video_copy_fit_or_lifetime_failed");moviePublication.interactive=false;SetLastError(after);return result;
        }
        storage.scaled=true;moviePublication={++serial,epoch,c,t.surface,false};
        if(!movie.canvas->fit(*plan,view(lease.d))){error("video_fit_failed");SetLastError(after);return result;}
        lease.written=true;
        trace("native_opening_video_fit",n("generation",serial)+n("handle",pointer)+n("lifetime",life)+n("slot",slot)+n("media_slot_value",media)+n("token",token)+
            n("source_width",geometry.width)+n("source_height",geometry.height)+n("x",x)+n("y",y)+n("result",result)+
            n("width",t.w)+n("height",t.h)+",\"repeated_history\":"+(result==1?"true":"false"));
        SetLastError(after);return result;
    }catch(...){moviePublication.interactive=false;error("video_adapter_exception");SetLastError(called?after:before);return called?result:original(handle,destination,pitch,height,x,y,flags);}
}
void resourceBoundary(const char* reason) noexcept {
#ifndef OPENING_FLOW_TEST
    HudCaptain::boundary(reason);
    HudAssembly::boundary(reason);
#endif
    if(!sameThread())return;LastError preserve;
    ++epoch;MainBackdrop::boundary();pass={};lease={};loading={};uiPublication.interactive=false;moviePublication.interactive=false;
    try{trace("native_opening_resource_boundary",",\"reason\":"+Qfr::quote(reason));}catch(...){}
}
void finish() noexcept {
    if(!enabled)return;
    try{trace("native_opening_end",n("screen_commits",frames)+n("video_copies",movieCopies)+n("errors",errors)+n("input_depth",inputDepth)+n("live_movies",movies.size()));}catch(...){}
}
}
