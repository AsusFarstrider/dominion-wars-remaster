// Actual OpeningFlow.cpp with synthetic engine memory and COM callbacks.
#include <Windows.h>
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <map>
#include <new>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <thread>
#include "RuntimeTestMemory.h"
#include <Research/MenuCanvas/MenuFamilies.h>
static std::atomic<bool> failAllocation{false};
void* operator new(size_t n){if(failAllocation.exchange(false))throw std::bad_alloc();if(auto p=std::malloc(n?n:1))return p;throw std::bad_alloc();}
void operator delete(void* p)noexcept{std::free(p);}
void operator delete(void* p,size_t)noexcept{std::free(p);}
namespace MenuCanvasTest {
std::map<uint32_t,std::vector<uint8_t>> memory;uint32_t failedRead=0;
auto region(uint32_t a){auto i=memory.upper_bound(a);if(i==memory.begin())return memory.end();--i;return uint64_t(a)-i->first<i->second.size()?i:memory.end();}
bool managed(uint32_t a,size_t){return region(a)!=memory.end();}
bool read(uint32_t a,void* p,size_t n){auto i=region(a);if(i==memory.end()||uint64_t(a)-i->first+n>i->second.size()||
    (failedRead>=a&&uint64_t(failedRead)<uint64_t(a)+n))return false;std::memcpy(p,i->second.data()+a-i->first,n);return true;}
bool write(uint32_t a,const void* p,size_t n){auto i=region(a);if(i==memory.end()||uint64_t(a)-i->first+n>i->second.size())return false;std::memcpy(i->second.data()+a-i->first,p,n);return true;}
void reserve(uint32_t a,size_t n){memory[a]=std::vector<uint8_t>(n);}
template<class T>void put(uint32_t a,T v){auto i=region(a);if(i==memory.end()||uint64_t(a)-i->first+sizeof(v)>i->second.size())throw std::logic_error("fixture address");std::memcpy(i->second.data()+a-i->first,&v,sizeof(v));}
void scalar(uint32_t a,uint32_t v){reserve(a,4);put(a,v);}
}
#define OPENING_FLOW_TEST
#include <Research/OpeningFlow/OpeningFlow.cpp>
namespace Qfr {std::string quote(const std::string& s){return '"'+s+'"';}void event(const char*,const std::string&)noexcept{}}
namespace Compat {FARPROC hookIatFunction(HMODULE,const char*,void*){throw std::logic_error("test must not install hooks");}
volatile bool refuseHooks=true;
void hookFunction(void*&,void*,const char*){if(refuseHooks)throw std::logic_error("test must not install hooks");}}
namespace {
using namespace OpeningFlow;
unsigned assertions=0;
void check(bool b,const char* label){++assertions;if(!b)throw std::runtime_error(label);}
uint32_t pointer(const void* p){return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(p));}
struct Surface {
    IDirectDrawSurface4 iface{};IDirectDrawSurface4Vtbl vtable{};unsigned refs=1,locks=0,unlocks=0,blts=0;
    bool inputInLock=false,inputInUnlock=false;HRESULT unlockResult=DD_OK;
    static constexpr unsigned w=1366,h=768,pitch=w*2+8;
    std::vector<uint8_t> pixels=std::vector<uint8_t>(pitch*h,0);
    Surface(){vtable.AddRef=addRef;vtable.Release=release;vtable.Lock=lock;vtable.Unlock=unlock;vtable.Blt=blt;iface.lpVtbl=&vtable;}
    static HRESULT WINAPI blt(IDirectDrawSurface4* p,RECT* r,IDirectDrawSurface4* src,RECT* sr,DWORD flags,DDBLTFX* fx){auto& s=get(p);++s.blts;
        if(r||src||sr||flags!=(DDBLT_COLORFILL|DDBLT_WAIT)||!fx||fx->dwSize!=sizeof(*fx)||fx->dwFillColor)return DDERR_INVALIDPARAMS;
        std::fill(s.pixels.begin(),s.pixels.end(),uint8_t{});return DD_OK;}
    static Surface& get(IDirectDrawSurface4* p){return *reinterpret_cast<Surface*>(p);}
    static ULONG WINAPI addRef(IDirectDrawSurface4* p){return ++get(p).refs;}
    static ULONG WINAPI release(IDirectDrawSurface4* p){return --get(p).refs;}
    static HRESULT WINAPI lock(IDirectDrawSurface4* p,RECT*,DDSURFACEDESC2* d,DWORD,HANDLE){auto& s=get(p);++s.locks;
        d->dwWidth=w;d->dwHeight=h;d->lPitch=pitch;d->lpSurface=s.pixels.data();d->ddpfPixelFormat.dwFlags=DDPF_RGB;
        d->ddpfPixelFormat.dwRGBBitCount=16;d->ddpfPixelFormat.dwRBitMask=0xF800;d->ddpfPixelFormat.dwGBitMask=0x7E0;d->ddpfPixelFormat.dwBBitMask=0x1F;
        if(s.inputInLock){inputScope(1);inputScope(2);}return DD_OK;}
    static HRESULT WINAPI unlock(IDirectDrawSurface4* p,RECT*){auto& s=get(p);++s.unlocks;if(s.inputInUnlock){inputScope(1);inputScope(2);}return s.unlockResult;}
    void original(){for(unsigned y=0;y<600;++y)for(unsigned x=0;x<800;++x){uint16_t v=0x07E0;std::memcpy(pixels.data()+y*pitch+x*2,&v,2);}}
};
struct Fixture {
    Surface surface;uint32_t frame[9]{};std::array<uint8_t,128> stack{};
    static constexpr uint32_t slot=0xF0000000,outer=0xF0001000,native=0xF0002000,child=0xF0003000,parent=0xF0004000,unrelated=0xF0005000;
    const Spec* spec=&specs[1];
    Fixture(){
        enabled=true;ownerThread=GetCurrentThreadId();epoch=1;serial=frames=movieCopies=errors=0;uiPublication={};moviePublication={};
        submenus=sliderReady=false;privatePoints.clear();loading={};
        surfaces.clear();movies.clear();uiMembers.clear();pass={};plan.reset();uiCanvas.reset();inputDepth=0;completing=copyingVideo=false;reported.clear();
        using namespace MenuCanvasTest;memory.clear();failedRead=0;
        scalar(0x4D0A24,5);scalar(0x4D0A28,5);scalar(0x713834,0x42A210);scalar(0x713838,0x42A230);scalar(0x71383C,0x42A290);scalar(0x713840,0);
        reserve(0x5F7ED8,0x90);put(0x5F7ED8+4,uint32_t{4});put(0x5F7ED8+0x73,spec->root);
        reserve(spec->root,0x90);put(spec->root,spec->id);put(spec->root+12,uint16_t{800});put(spec->root+14,uint16_t{600});
        put(spec->root+0x48,spec->input);put(spec->root+0x4C,spec->draw);put(spec->root+0x77,uint32_t{0x5F7ED8});put(spec->root+0x73,parent);
        reserve(parent,0x90);put(parent+4,uint32_t{4});put(parent+8,int16_t{40});put(parent+10,int16_t{50});put(parent+0x77,spec->root);put(parent+0x73,child);
        reserve(child,0x90);put(child+4,uint32_t{1});put(child+8,int16_t{20});put(child+10,int16_t{30});put(child+12,uint16_t{100});put(child+14,uint16_t{40});put(child+0x77,parent);
        reserve(unrelated,0x90);put(unrelated+0x77,uint32_t{0x5F7ED8});
        scalar(0xA39B64,slot);scalar(0xA387B4,pointer(&surface.iface));reserve(slot,4);put(slot,outer);reserve(outer,0x90);put(outer,uint16_t{0x8000});put(outer+0x20,native);
        reserve(native,sizeof(QImageResearch));QImageResearch image{};image.flags=0x201;image.pixel_bits=16;image.surface=pointer(&surface.iface);image.width=Surface::w;image.height=Surface::h;put(native,image);
        frame[1]=child;frame[3]=pointer(stack.data())-4;
        point(80,90);
        check(context()==spec,"fixture exact screen context");
    }
    void point(int lx,int ly){auto fit=dw::ui::Canvas::fit({0,0,800,600},{0,0,Surface::w,Surface::h});const auto rect=fit->map_rect({lx,ly,lx+1,ly+1});
        int32_t x=int32_t(rect->left)-40,y=int32_t(rect->top)-50,px=40,py=50;
        std::memcpy(stack.data()+0x14,&x,4);std::memcpy(stack.data()+0x10,&y,4);std::memcpy(stack.data()+0x34,&px,4);std::memcpy(stack.data()+0x38,&py,4);}
    void draw(){surface.original();pass={spec,100,epoch,true,false,{pointer(&surface.iface),Surface::w,Surface::h}};complete(100);}
    uint32_t route(){inputScope(1);const auto r=predicate(frame,0x12345678);inputScope(2);return r;}
    void modal(uint32_t root,uint32_t state=7){
        using namespace MenuCanvasTest;submenus=sliderReady=true;scalar(0x4D0A24,state);scalar(0x4D0A28,state);
        put(0x5F7ED8+0x73,root);spec=specFor(state);check(spec&&spec->root==root,"modal identity selected");
        reserve(root,0x90);put(root,spec->id);put(root+4,spec->flags);put(root+8,int16_t((Surface::w-spec->w)/2));put(root+10,int16_t((Surface::h-spec->h)/2));
        put(root+12,spec->w);put(root+14,spec->h);put(root+0x4C,spec->draw);put(root+0x77,uint32_t{0x5F7ED8});put(root+0x73,parent);put(root+0x6B,uint32_t{0x4E47C0});put(parent+0x77,root);
        put(root+0x6B,spec->underlying);reserve(spec->underlying,0x90);put(spec->underlying+0x77,uint32_t{0x5F7ED8});
        if(state==27){put(spec->underlying+4,uint32_t{4});put(spec->underlying+0x4C,uint32_t{0x488560});}
        else {put(0x4E47C0,uint32_t{0x405});put(0x4E47C0+4,uint32_t{0x84});put(0x4E47C0+12,uint16_t{800});put(0x4E47C0+14,uint16_t{600});put(0x4E47C0+0x4C,uint32_t{0x46ADD0});}
    }
    void composite(){uint32_t r[9]{},s[9]{};s[1]=0x5F7ED8;observe(0,r,s,100);surface.original();pass.opaque=true;}
    dw::ui::Point physical(int x,int y){auto g=dw::ui::Canvas::fit({0,0,800,600},{0,0,Surface::w,Surface::h});auto r=g->map_rect({x,y,x+1,y+1});return {r->left,r->top};}
};
}
#include "HdCases.h"
#include "CardCases.h"
int main(){try {
    {Fixture f;f.draw();check(uiPublication.interactive,"normal commit");check(f.route()==0x41DADC,"nested-parent hit");check(f.surface.refs==1&&f.surface.locks==f.surface.unlocks,"balanced COM");
        f.point(500,500);check(f.route()==0x41DC62,"ordinary rectangle miss");f.point(80,90);check(f.route()==0x41DADC,"ordinary miss does not poison");}
    {Fixture f;f.surface.unlockResult=DDERR_SURFACELOST;f.draw();check(surfaces[pointer(&f.surface.iface)].scaled,"failed Unlock keeps scaled provenance");check(uiPublication.generation&&!uiPublication.interactive,"first uncertain write has quarantined candidate");check(f.route()==0x41DC62,"failed first Unlock never native input");f.surface.unlockResult=DD_OK;f.draw();check(f.route()==0x41DADC,"fresh repair after failed Unlock");}
    {Fixture f;f.draw();int32_t bad=41;std::memcpy(f.stack.data()+0x34,&bad,4);check(f.route()==0x41DC62,"parent mismatch miss");f.point(80,90);check(f.route()==0x41DC62,"restored parent does not revive old frame");f.draw();check(f.route()==0x41DADC,"new frame repairs parent invalidation");}
    {Fixture f;f.draw();MenuCanvasTest::put(0xA387B4,uint32_t{1});check(f.route()==0x41DC62,"target mismatch miss");MenuCanvasTest::put(0xA387B4,pointer(&f.surface.iface));check(f.route()==0x41DC62,"restored target stays quarantined");f.draw();check(f.route()==0x41DADC,"new frame repairs target");}
    {Fixture f;f.draw();MenuCanvasTest::failedRead=Fixture::child;check(f.route()==0x41DC62,"known unreadable child misses");MenuCanvasTest::failedRead=0;check(f.route()==0x41DC62,"restored ancestry stays quarantined");f.draw();check(f.route()==0x41DADC,"fresh ancestry repair");f.frame[1]=Fixture::unrelated;check(f.route()==0x12345678,"unrelated native node unchanged");}
    {Fixture f;f.draw();inputScope(1);inputScope(1);inputScope(2);check(predicate(f.frame,0x12345678)==0x41DC62,"nested input invalidates outer publication");inputScope(2);f.draw();check(f.route()==0x41DADC,"fresh draw repairs nesting");}
    {Fixture f;f.surface.inputInLock=true;f.draw();check(!uiPublication.interactive,"Lock reentry cannot publish old candidate");f.surface.inputInLock=false;f.draw();check(f.route()==0x41DADC,"fresh draw repairs Lock reentry");}
    {Fixture f;f.surface.inputInUnlock=true;f.draw();check(!uiPublication.interactive,"Unlock reentry cannot publish old candidate");check(f.route()==0x41DC62,"write before reentrant Unlock remains quarantined");f.surface.inputInUnlock=false;f.draw();check(f.route()==0x41DADC,"fresh draw repairs Unlock reentry");}
    {Fixture f;f.draw();failAllocation=true;check(f.route()==0x41DC62,"predicate allocation failure misses");check(!uiPublication.interactive,"predicate exception invalidates publication");check(f.route()==0x41DC62,"predicate exception cannot revive prior generation");f.draw();check(f.route()==0x41DADC,"fresh draw repairs predicate exception");}
    {Fixture f;const auto old=errors;failAllocation=true;error("forced-reporter-allocation-failure-with-long-diagnostic");check(errors==old+1,"allocation-safe reporter retains counter");}
    {Fixture f;f.modal(0x4DB6F8);f.composite();MenuProbe::Node root{};MenuProbe::node(reader,f.spec->root,root);
        check(root.x==200&&root.y==75,"Preferences both axes normalized");check(pass.spec==f.spec,"outer composite active");
        complete(76);check(!uiPublication.generation&&f.surface.locks==0,"nested return does not fit Main plus modal early");
        check(pass.stack==88,"421E80 three saved registers place 421F60 ESP twelve bytes below entry");
        complete(88);check(uiPublication.interactive&&frames==1&&f.surface.locks==1,"one complete modal publication at actual return stack");
        check(uiMembers.count(0x4E47C0)&&uiMembers.count(Fixture::child),"Main and modal members retained in original forest");
        const auto p=f.physical(280,180);DWORD r[9]{};r[0]=Fixture::child;r[4]=uint32_t(p.x);r[2]=uint32_t(p.y);
        inputScope(1);privateObserve(0,r);check(int32_t(r[4])==280&&int32_t(r[2])==180,"slider private coordinates inverse mapped");
        r[4]=0;r[2]=uint32_t(p.y);privateObserve(0,r);check(int32_t(r[4])<0,"captured drag extends into margin instead of sticking");inputScope(2);
        const auto last=int32_t(r[4]);uiPublication.interactive=false;r[4]=9999;inputScope(1);privateObserve(0,r);inputScope(2);
        check(int32_t(r[4])==last,"uncertain slider drag retains last authored point");
        MenuCanvasTest::put(0x5F7ED8+0x73,uint32_t{0x4E47C0});check(context()&&context()->root==0x4E47C0,"modal detach restores pure Main ownership");}
    {Fixture f;f.modal(0x4DF100);f.composite();complete(88);MenuProbe::Node root{};MenuProbe::node(reader,f.spec->root,root);
        check(root.x==100&&root.y==75,"Load both axes normalized");const auto p=f.physical(310,220);DWORD r[9]{};
        r[0]=Fixture::child;r[4]=uint32_t(p.y);MenuCanvasTest::scalar(0x5F7E5C,uint32_t(p.x));
        inputScope(1);privateObserve(1,r);check(int32_t(r[4])==220,"list private Y inverse mapped");
        r[1]=Fixture::child;r[3]=pointer(f.stack.data())-4;r[6]=uint32_t(p.x);r[7]=uint32_t(p.y);uint32_t caller=0x420083;std::memcpy(f.stack.data()+0x24,&caller,4);
        privateObserve(2,r);check(int32_t(r[6])==310&&int32_t(r[7])==220,"row helper independent XY inverse mapped");
        r[6]=uint32_t(p.x);r[7]=uint32_t(p.y);caller=0x12345678;std::memcpy(f.stack.data()+0x24,&caller,4);privateObserve(2,r);
        check(r[6]==p.x&&r[7]==p.y,"unrelated row helper caller unchanged");inputScope(2);}
    {Fixture f;submenus=true;for(const auto state:{15u,20u,21u,22u,27u}){
        MenuCanvasTest::scalar(0x4D0A24,state);MenuCanvasTest::scalar(0x4D0A28,state);const Spec* s=nullptr;
        for(const auto& candidate:specs)if(candidate.state==state&&!candidate.modal){s=&candidate;break;}
        if(!s)throw std::logic_error("missing fixture owner");
        using namespace MenuCanvasTest;reserve(s->root,0x90);put(s->root,s->id);put(s->root+4,s->flags);put(s->root+12,s->w);put(s->root+14,s->h);put(s->root+0x4C,s->draw);put(s->root+0x48,s->input);put(s->root+0x77,uint32_t{0x5F7ED8});put(0x5F7ED8+0x73,s->root);
        check(context()==s,"Credits/MP exact owner and zero-size roots admitted");}}
    {Fixture f;f.modal(0x4DB6F8,27);f.composite();complete(88);check(uiPublication.interactive&&uiPublication.spec->state==27,"Preferences composite over actual mission-list owner");}
    {Fixture f;f.modal(0x4DB6F8);using namespace MenuCanvasTest;put(0x5F7ED8+0x73,uint32_t{0x4E47C0});f.composite();complete(88);
        check(frames==0&&f.surface.locks==0,"ordinary Main retains its passing shadow compositor without second fit");}
    {Fixture f;f.modal(0x4DB6F8);using namespace MenuCanvasTest;put(0x5F7ED8+0x73,uint32_t{0x4E47C0});put(0x4E47C0+0x73,uint32_t{0x4E4B20});
        const uint32_t selected=0x4E4A00;
        for(unsigned i=0;i<6;++i){const auto a=0x4E4850+i*0x90;reserve(a,0x90);put(a+4,uint32_t{a==selected?0x100001u:0x10001u});put(a+0x77,uint32_t{0x4E47C0});put(a+0x6B,i?a-0x90:0);put(a+0x4C,uint32_t{0x46B440});put(a+0x48,uint32_t{0x46AFA0});}
        put(selected+0x54,uint32_t{8});f.composite();uint32_t r[9]{},s[9]{};r[1]=selected;observe(1,r,s,76);
        check(!mainEvent(0x12345678,selected,9999),"unproven modal caller cannot authorize final Main fit");
        check(mainEvent(0x46B4B1,selected,9999),"actual selected animation completion authorizes modal handoff");
        check(value(selected+0x54)==8,"modal observation never replays handler");
        for(unsigned i=0;i<6;++i)put(0x4E4850+i*0x90+4,uint32_t{1});
        put(0x5F7ED8+0x73,uint32_t{0x4DB6F8});complete(88);
        check(frames==1&&uiPublication.spec->root==0x4E47C0,"newly inserted unpainted modal leaves a fitted departing Main");
        check(!uiPublication.interactive,"departing Main cannot publish invisible modal input");
        check(f.route()==0x41DC62,"new modal hit tests wait for actual first fitted modal paint");
        put(0x5F7ED8+0x73,uint32_t{0x4E47C0});f.frame[1]=selected;check(f.route()==0x12345678,"return to pure Main delegates to passing input controller");}
    {Fixture f;using namespace MenuCanvasTest;put(0x4D0A28,uint32_t{6});uint32_t r[9]{},s[9]{};r[1]=f.spec->root;
        observe(1,r,s,100);check(pass.spec==f.spec,"original departing owner is captured despite pending next state");
        f.surface.original();pass.opaque=true;complete(100);check(frames==1&&!uiPublication.interactive,"departing original screen stays fitted without admitting input");}
    check(MenuCanvas::mainDeparture(0x4E4970,true)==20&&MenuCanvas::mainDeparture(0x4E4A90,true)==15,"known Multiplayer/Credits completion identities");
    check(MenuCanvas::mainDeparture(0x4E4970,false)==0,"expanded Main destinations remain opt-in");
    check(MenuCanvas::pendingAllowed(MenuCanvas::mainMenu,20,true,20),"proved Multiplayer departing visual accepted");
    check(!MenuCanvas::pendingAllowed(MenuCanvas::mainMenu,20,true,15),"unproved destination remains rejected");
    check(!MenuCanvas::pendingAllowed(MenuCanvas::mainMenu,20,false,20),"departing visual never enables destination input");
    {Fixture f;using namespace MenuCanvasTest;submenus=sliderReady=true;
        scalar(0x4D0A24,1);scalar(0x4D0A28,3);scalar(0x7180B5,1);scalar(0xA39B78,0);scalar(0x71404C,0x3F000000);
        put(0x5F7ED8+0x73,uint32_t{0x4DA738});const auto* s=specFor(1);reserve(s->root,0x90);put(s->root,s->id);put(s->root+4,s->flags);put(s->root+8,int16_t{483});put(s->root+10,int16_t{329});put(s->root+12,s->w);put(s->root+14,s->h);put(s->root+0x4C,s->draw);put(s->root+0x77,uint32_t{0x5F7ED8});
        put(0x4D0A24,uint32_t{16}); // Another constructor state, same active loading owner.
        DWORD r[9]{};r[3]=pointer(f.stack.data())-4;f.surface.original();privateObserve(3,r);
        MenuProbe::Node root{};MenuProbe::node(reader,s->root,root);check(root.x==200&&root.y==245,"loading panel authored origin");
        check(loading.active&&pass.spec==s&&f.surface.blts==1,"direct updater owns fresh defined background");
        check(f.surface.pixels[0]==0,"loading does not inherit fitted background pixels");
        uint32_t caller=0x4346F8;std::memcpy(f.stack.data(),&caller,4);r[6]=0xA38770;
        privateObserve(4,r);check(frames==1&&!loading.active&&f.surface.locks==1,"loading fit happens before direct presentation");
        privateObserve(4,r);check(frames==1,"same direct present cannot fit twice");
        put(0x5F7ED8+0x73,uint32_t{0});put(s->root+0x77,uint32_t{0});
        privateObserve(3,r);check(loading.active&&pass.directLoading,"UI tree reset does not end the direct progress owner");
        privateObserve(4,r);check(frames==2&&f.surface.blts==2,"detached later loading updates each clear and fit before original present");
        check(!uiPublication.interactive,"direct loading never publishes an interactive detached widget");
        put(0x7180B5,uint32_t{0});privateObserve(3,r);check(!loading.active,"inactive loading resource remains untouched");}
    runHdCases();
    runCardCases();
    std::cout<<"PASS "<<assertions<<" actual opening-controller assertions including HD provider\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
