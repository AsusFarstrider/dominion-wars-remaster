// Runs the actual controller/route implementation with synthetic engine memory,
// blits and COM Lock/Unlock. No game code, graphics device or window is executed.
#include <Windows.h>
#pragma comment(lib,"bcrypt.lib")
#include <atomic>
#include <thread>
#include <cstdlib>
#include <cstring>
#include <map>
#include <new>
#include <stdexcept>
#include <vector>
#include <iostream>
#include "RuntimeTestMemory.h"
static std::atomic<bool> failNextAllocation{false};
void* operator new(size_t bytes){if(failNextAllocation.exchange(false))throw std::bad_alloc();if(auto p=std::malloc(bytes?bytes:1))return p;throw std::bad_alloc();}
void operator delete(void* p) noexcept {std::free(p);}
void operator delete(void* p,size_t) noexcept {std::free(p);}
namespace MenuCanvasTest {
std::map<uint32_t,std::vector<uint8_t>> regions;
uint32_t failedRead=0;
auto region(uint32_t a){auto i=regions.upper_bound(a);if(i==regions.begin())return regions.end();--i;return uint64_t(a)-i->first<i->second.size()?i:regions.end();}
bool managed(uint32_t a,size_t){return region(a)!=regions.end();}
bool readable(uint32_t a,size_t n){const auto i=region(a);return i!=regions.end()&&uint64_t(a)-i->first+n<=i->second.size()&&!(failedRead>=a&&uint64_t(failedRead)<uint64_t(a)+n);}
bool read(uint32_t a,void* p,size_t n){if(!readable(a,n))return false;const auto i=region(a);std::memcpy(p,i->second.data()+a-i->first,n);return true;}
void write(uint32_t a,const void* p,size_t n){const auto i=region(a);if(i==regions.end()||uint64_t(a)-i->first+n>i->second.size())throw std::logic_error("bad fixture write");std::memcpy(i->second.data()+a-i->first,p,n);}
void reserve(uint32_t a,size_t n){regions[a]=std::vector<uint8_t>(n);}
template<class T> void put(uint32_t a,const T& v){write(a,&v,sizeof(v));}
void scalar(uint32_t a,uint32_t v){reserve(a,4);put(a,v);}
}
#define MENU_CANVAS_TEST
#include "CanvasRuntime.cpp"
namespace Qfr {
std::string quote(const std::string& s){return '"'+s+'"';}
void event(const char*,const std::string&) noexcept {}
}
namespace Compat {
void hookFunction(void*&,void*,const char*){throw std::logic_error("console test must not install hooks");}
}
namespace {
unsigned assertions=0;
void check(bool b,const char* label){++assertions;if(!b)throw std::runtime_error(label);}
uint32_t pointer(const void* p){return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(p));}
struct Surface;
Surface* currentSurface=nullptr;
struct Surface {
    IDirectDrawSurface4 iface{};IDirectDrawSurface4Vtbl methods{};
    unsigned refs=1,locks=0,unlocks=0;
    HRESULT lockResult=DD_OK,unlockResult=DD_OK;
    bool allocationFailureAfterLock=false;
    bool foreignDuringLock=false,foreignDuringUnlock=false;
    unsigned lockReentry=0,unlockReentry=0;
    bool notifyForReentry=true;
    uint32_t reentrantRoute=0;
    static constexpr uint32_t width=1366,height=768,pitch=width*2+8;
    std::vector<uint8_t> pixels=std::vector<uint8_t>(pitch*height,0xA5);
    Surface(){methods.AddRef=addRef;methods.Release=release;methods.Lock=lock;methods.Unlock=unlock;iface.lpVtbl=&methods;currentSurface=this;}
    static ULONG WINAPI addRef(IDirectDrawSurface4*){return ++currentSurface->refs;}
    static ULONG WINAPI release(IDirectDrawSurface4*){return --currentSurface->refs;}
    static void reenter(unsigned kind,bool notify){
        if(!kind)return;
        if(notify)MenuCanvasRuntime::foreignThreadSeen=true;
        DWORD words[32]{},frame[9]{};words[5]=1063;words[4]=101;frame[1]=0x4E50E0;frame[3]=pointer(words)-4;
        if(kind==1){MenuCanvasRuntime::inputObserve(1,frame);frame[3]-=4;MenuCanvasRuntime::inputObserve(2,frame);}
        else if(kind==2)currentSurface->reentrantRoute=MenuCanvasRuntime::predicate(frame);
        else if(kind==4)MenuCanvasRuntime::resourceBoundary("simulated_same_pointer_restore");
        else MenuCanvas::drainForeignInput(MenuCanvasRuntime::publication,MenuCanvasRuntime::foreignThreadSeen);
    }
    static HRESULT WINAPI lock(IDirectDrawSurface4*,RECT*,DDSURFACEDESC2* d,DWORD,HANDLE){
        auto& s=*currentSurface;++s.locks;if(FAILED(s.lockResult))return s.lockResult;
        d->dwWidth=width;d->dwHeight=height;d->lPitch=pitch;d->lpSurface=s.pixels.data();
        d->ddpfPixelFormat.dwFlags=DDPF_RGB;d->ddpfPixelFormat.dwRGBBitCount=16;
        d->ddpfPixelFormat.dwRBitMask=0xF800;d->ddpfPixelFormat.dwGBitMask=0x07E0;d->ddpfPixelFormat.dwBBitMask=0x001F;
        if(s.allocationFailureAfterLock){s.allocationFailureAfterLock=false;failNextAllocation=true;}
        if(s.foreignDuringLock)MenuCanvasRuntime::foreignThreadSeen=true;
        reenter(s.lockReentry,s.notifyForReentry);
        return DD_OK;
    }
    static HRESULT WINAPI unlock(IDirectDrawSurface4*,RECT*){++currentSurface->unlocks;if(currentSurface->foreignDuringUnlock)MenuCanvasRuntime::foreignThreadSeen=true;reenter(currentSurface->unlockReentry,currentSurface->notifyForReentry);return currentSurface->unlockResult;}
    uint16_t pixel(unsigned x,unsigned y){uint16_t p;std::memcpy(&p,pixels.data()+y*pitch+x*2,2);return p;}
    void originalSmallDraw(){for(unsigned y=0;y<600;++y)for(unsigned x=0;x<800;++x){const uint16_t c=0x07E0;std::memcpy(pixels.data()+y*pitch+x*2,&c,2);}}
};
using namespace MenuCanvasRuntime;
struct Fixture {
    Surface surface;
    std::vector<uint8_t> sourcePixels=std::vector<uint8_t>(800*600,1);
    std::array<uint8_t,128> inputMemory{},nestedMemory{};
    uint32_t draws[9]{},arguments[9]{};
    DWORD input[9]{};
    static constexpr uint32_t slot=0xF0000000,outer=0xF0001000,native=0xF0002000,source=0xF0003000;
    static uint32_t __cdecl blit(QImageResearch*,QImageResearch* destination,int,int){
        auto* pixels=reinterpret_cast<uint16_t*>(destination->pixels);
        std::fill(pixels,pixels+800*600,uint16_t{0xF81F});return 0;
    }
    Fixture(){
        enabled=true;mainEnabled=false;lastFamily=nullptr;active.reset();plan.reset();publication={};lastTarget={};ownerThread=GetCurrentThreadId();
        foreignThreadSeen=false;scaledWriteSeen=false;inputDepth=inputOverflow=0;nextPass=commits=rejections=shadowDraws=controllerExceptions=0;
        inputOriginals[0]=reinterpret_cast<void*>(0x12345678);
        MenuCanvasTest::regions.clear();MenuCanvasTest::failedRead=0;
        using namespace MenuCanvasTest;
        for(auto p:{0x4D0A24u,0x4D0A28u})scalar(p,10);
        scalar(0x713834,0x42A210);scalar(0x713838,0x42A230);scalar(0x71383C,0x42A290);scalar(0x713840,0);
        scalar(0x713DE8,16);scalar(0x5F7ED4,0);scalar(0x5F7ED0,0);scalar(0x5F7F6C,0);scalar(0x5F7F94,0);
        reserve(0x4E5050,0x3F0);reserve(0x5F7ED8,0x90);reserve(0xA39B50,0x30);scalar(0xA39D80,0);
        put(0x5F7EDC,uint32_t{4});put(0x5F7ED8+0x73,uint32_t{0x4E5050});
        put(0x4E5050,uint32_t{0x43F});put(0x4E5054,uint32_t{4});put(0x4E5050+12,uint16_t{800});put(0x4E5050+14,uint16_t{600});
        put(0x4E5050+0x77,uint32_t{0x5F7ED8});put(0x4E5050+0x73,uint32_t{0x4E53B0});put(0x4E5050+0x4C,uint32_t{0x472630});
        for(size_t i=0;i<buttons.size();++i){
            const auto p=0x4E50E0+uint32_t(i)*0x90;const auto& b=buttons[i];
            put(p,b.id);put(p+4,uint32_t{1});put(p+8,b.x);put(p+10,b.y);put(p+12,b.w);put(p+14,b.h);
            put(p+0x77,uint32_t{0x4E5050});put(p+0x6B,i?p-0x90:0);put(p+0x48,uint32_t{0x4729F0});put(p+0x46,uint16_t{0x4001});
        }
        reserve(slot,4);put(slot,outer);reserve(outer,0x90);put(outer,uint16_t{0x8000});put(outer+0x20,native);
        QImageResearch d{};d.flags=0x201;d.width=Surface::width;d.height=Surface::height;d.pixel_bits=16;d.surface=pointer(&surface.iface);
        reserve(native,sizeof(d));put(native,d);put(0xA39B64,slot);
        d={};d.flags=1;d.and_mask=255;d.pixel_bits=8;d.width=800;d.height=600;d.pixels=pointer(sourcePixels.data());d.pitch=800;
        reserve(source,sizeof(d));put(source,d);
        arguments[1]=source;arguments[2]=outer;draws[1]=0x4E5050;input[1]=0x4E50E0;
        const auto sp=pointer(inputMemory.data());input[3]=sp-4;
        const int32_t x=1063,y=101;std::memcpy(inputMemory.data()+0x14,&x,4);std::memcpy(inputMemory.data()+0x10,&y,4);
        check(family(),"fixture meets exact live family admission");
    }
    ~Fixture() noexcept(false){active.reset();plan.reset();check(surface.refs==1,"surface references released");enabled=false;}
    void begin(uint32_t stack=0x1000){draws[1]=0x4E5050;observe(1,draws,arguments,stack,reinterpret_cast<void*>(&blit));}
    void producers(){observe(10,draws,arguments,0,reinterpret_cast<void*>(&blit));for(uint32_t i=0;i<6;++i){draws[1]=0x4E50E0+i*0x90;observe(1,draws,arguments,0,reinterpret_cast<void*>(&blit));}}
    void end(uint32_t stack=0x1000){observe(3,draws,arguments,stack,reinterpret_cast<void*>(&blit));}
    void repair(){begin();producers();end();check(publication.scaledPredicatesAllowed(),"complete repair publishes scaled generation");}
    void inputBegin(){inputObserve(1,input);}
    void inputEnd(){std::array<DWORD,9> r{};std::copy(std::begin(input),std::end(input),r.begin());r[3]-=4;inputObserve(2,r.data());}
    uint32_t hit(){return predicate(input);}
    void expectSuspended(){inputBegin();check(hit()==0x41DC62,"actual predicate takes miss while quarantined");inputEnd();}
    void expectHit(){inputBegin();check(hit()==0x41DADC,"actual predicate takes original hit continuation after repair");inputEnd();}
};
void tests(){
    {
        Fixture f;f.repair();f.expectHit();
        f.inputBegin();const int32_t margin=100;std::memcpy(f.inputMemory.data()+0x14,&margin,4);
        check(f.hit()==0x41DC62&&publication.scaledPredicatesAllowed(),"ordinary margin is not an ownership failure");
        const int32_t inside=1063;std::memcpy(f.inputMemory.data()+0x14,&inside,4);check(f.hit()==0x41DADC,"ordinary reentry needs no redraw");f.inputEnd();
    }
    for(unsigned cause=0;cause<6;++cause){
        Fixture f;f.repair();f.inputBegin();
        if(cause==0)MenuCanvasTest::put(0x5F7ED8+0x73,uint32_t{0xF0009999});
        if(cause==1)MenuCanvasTest::put(0x5F7F6C,uint32_t{0xF0009999});
        if(cause==2){std::array<DWORD,9> nested{};std::copy(std::begin(f.input),std::end(f.input),nested.begin());nested[3]=pointer(f.nestedMemory.data())-4;inputObserve(1,nested.data());nested[3]-=4;inputObserve(2,nested.data());}
        if(cause==3){const int32_t origin=1;std::memcpy(f.inputMemory.data()+0x34,&origin,4);}
        if(cause==4)MenuCanvasTest::put(Fixture::native+10,uint16_t{1367});
        if(cause==5)MenuCanvasTest::failedRead=0x4E50E0+0x46;
        check(f.hit()==0x41DC62,"runtime admission failure suspends input");
        MenuCanvasTest::put(0x5F7ED8+0x73,uint32_t{0x4E5050});MenuCanvasTest::put(0x5F7F6C,uint32_t{0});MenuCanvasTest::failedRead=0;
        MenuCanvasTest::put(Fixture::native+10,uint16_t{1366});
        const int32_t zero=0;std::memcpy(f.inputMemory.data()+0x34,&zero,4);
        check(f.hit()==0x41DC62&&!publication.scaledPredicatesAllowed(),"restored guards before redraw cannot regain old generation");
        f.inputEnd();f.repair();f.expectHit();
    }
    {
        Fixture f;f.repair();f.inputBegin();foreignThreadSeen=true;
        check(f.hit()==0x41DC62&&!foreignThreadSeen.load(),"owner predicate drains foreign invalidation immediately");
        check(f.hit()==0x41DC62,"foreign invalidation remains latched after drain");f.inputEnd();f.repair();f.expectHit();
    }
    {
        Fixture f;f.repair();foreignThreadSeen=true;f.inputBegin();check(!foreignThreadSeen.load()&&!publication.scaledPredicatesAllowed(),"owner input entry drains foreign flag");
        f.inputEnd();f.expectSuspended();f.repair();f.expectHit();
    }
    {
        Fixture f;f.repair();f.begin();f.producers();coverageLost("missing required glyph after complete node set");f.surface.originalSmallDraw();
        f.end();check(commits==1&&f.surface.pixel(1063,101)==0xF81F,"rejected update can retain scaled pixels outside original footprint");f.expectSuspended();f.repair();f.expectHit();
    }
    {
        Fixture f;f.repair();f.begin();f.producers();const auto outerPass=active->id;
        f.begin(0x2000);check(active&&active->id==outerPass&&!active->good,"nested root retains poisoned outer owner");
        f.producers();f.end(0x2000);check(active&&commits==1,"inner epilogue cannot publish or dispose outer");
        f.end();check(!active&&commits==1,"outer completion disposes without commit");f.expectSuspended();f.repair();f.expectHit();
    }
    {
        Fixture f;f.repair();f.surface.lockResult=DDERR_SURFACELOST;f.begin();f.producers();f.end();
        check(f.surface.locks==2&&f.surface.unlocks==1,"failed Lock does not call Unlock");f.surface.lockResult=DD_OK;
        f.expectSuspended();f.repair();f.expectHit();
    }
    {
        Fixture f;f.surface.unlockResult=DDERR_SURFACELOST;f.begin();f.producers();f.end();
        check(commits==0&&publication.mayContainScaledPixels(),"first failed Unlock retains possible scaled ownership");
        f.surface.unlockResult=DD_OK;f.expectSuspended();f.repair();f.expectHit();
    }
    {
        Fixture f;f.repair();f.surface.allocationFailureAfterLock=true;f.begin();f.producers();f.end();
        check(!failNextAllocation&&f.surface.locks==2&&f.surface.unlocks==2,"actual post-Lock allocation failure triggers RAII Unlock");
        check(!active&&f.surface.refs==1,"exception unwinds matching owner after Unlock and Release");f.expectSuspended();f.repair();f.expectHit();
    }
    {
        Fixture f;f.repair();MenuCanvasTest::put(0x4E50E0+0x46,uint16_t{0});f.begin();f.end();
        MenuCanvasTest::put(0x4E50E0+0x46,uint16_t{0x4001});f.expectSuspended();f.repair();f.expectHit();
    }
    for(bool afterWrite:{false,true}){
        Fixture f;f.repair();f.surface.foreignDuringLock=!afterWrite;f.surface.foreignDuringUnlock=afterWrite;
        f.begin();f.producers();f.end();check(commits==1&&!publication.scaledPredicatesAllowed(),"pending foreign notification at write/publication boundary vetoes commit");
        f.surface.foreignDuringLock=f.surface.foreignDuringUnlock=false;f.expectSuspended();f.repair();f.expectHit();
    }
    for(bool afterWrite:{false,true})for(bool notify:{false,true})for(unsigned kind:{1u,2u}){
        Fixture f;f.repair();f.surface.notifyForReentry=notify;
        (afterWrite?f.surface.unlockReentry:f.surface.lockReentry)=kind;
        f.begin();f.producers();f.end();
        check(commits==1&&!publication.scaledPredicatesAllowed(),"owner input draining poison during Lock/Unlock cannot let old candidate repair itself");
        if(kind==2)check(f.surface.reentrantRoute==0x41DC62,"reentrant predicate retains original miss continuation");
        f.surface.lockReentry=f.surface.unlockReentry=0;f.expectSuspended();f.repair();f.expectHit();
    }
    for(bool afterWrite:{false,true}){
        Fixture f;f.repair();(afterWrite?f.surface.unlockReentry:f.surface.lockReentry)=3;
        f.begin();f.producers();f.end();check(commits==1&&!publication.scaledPredicatesAllowed(),"checked epoch vetoes candidate even if helper drained flag without marking active bad");
        f.surface.lockReentry=f.surface.unlockReentry=0;f.expectSuspended();f.repair();f.expectHit();
    }
}
}
namespace {
struct MainFixture:Fixture {
    MainFixture() {
        using namespace MenuCanvasTest;const auto& f=MenuCanvas::mainMenu;
        mainEnabled=true;put(0x4D0A24,f.state);put(0x4D0A28,f.state);
        reserve(f.root,0x3F0);put(f.root,f.id);put(f.root+4,f.flags);
        put(f.root+12,uint16_t{800});put(f.root+14,uint16_t{600});
        put(f.root+0x77,uint32_t{0x5F7ED8});put(f.root+0x73,f.firstButton+5*0x90);put(f.root+0x4C,f.draw);
        put(f.root+0x50,uint32_t{0x201});put(outer+0x10,uint16_t{Surface::width});put(outer+0x12,uint16_t{Surface::height});
        put(outer+0x46,std::array<int16_t,4>{0,0,int16_t(Surface::width),int16_t(Surface::height)});
        scalar(0x4EC390,0xF1000000);reserve(0xF1000000,512);
        QImageResearch raw{};read(source,&raw,sizeof(raw));raw.unknown_02=255;put(source,raw);
        put(0x5F7ED8+0x73,f.root);
        for(size_t i=0;i<f.buttons.size();++i) {
            const auto p=f.firstButton+uint32_t(i)*0x90;const auto& b=f.buttons[i];
            put(p,b.id);put(p+4,uint32_t{1});put(p+8,b.x);put(p+10,b.y);put(p+12,b.w);put(p+14,b.h);
            put(p+0x77,f.root);put(p+0x6B,i?p-0x90:0);put(p+0x48,f.input);put(p+0x4C,f.buttonDraw);put(p+0x46,uint16_t{0x4001});
            put(p+0x10,b.baseImage);put(p+0x50,9u|(25u<<8)|(uint32_t(b.hoverRestart)<<16));
            put(p+0x6F,i<5?p+0x90:0);
        }
        input[1]=f.firstButton;
        check(family(false,&f),"main fixture matches initialized original family");
    }
    void begin(uint32_t stack=0x1000){draws[1]=MenuCanvas::mainMenu.root;observe(1,draws,arguments,stack,reinterpret_cast<void*>(&blit));}
    void clear(uint8_t color=0) {
        using namespace MenuCanvasTest;
        QImageResearch original{};read(source,&original,sizeof(original));
        QImageResearch solid{};solid.flags=0;solid.unknown_02=255;solid.pixel_bits=8;solid.xor_value=color;
        solid.width=Surface::width;solid.height=Surface::height;solid.pixels=0xBAD; // inactive field must not be dereferenced
        const auto caller=arguments[0];arguments[0]=0x4A1774;
        put(source,solid);observe(10,draws,arguments,0,reinterpret_cast<void*>(&blit));put(source,original);arguments[0]=caller;
    }
    void activationFlags(uint32_t pending) {
        const auto actor=pending==10?0x4E4850u:0x4E4B20u;
        for(uint32_t i=0;i<6;++i){const auto p=MenuCanvas::mainMenu.firstButton+i*0x90;
            MenuCanvasTest::put(p+4,uint32_t(p==actor?0x120001:0x10001));}
        MenuCanvasTest::put(actor+0x54,uint32_t{8});
    }
    void activationComplete(uint32_t pending,uint32_t caller=0x46B4B1) {
        const auto actor=pending==10?0x4E4850u:0x4E4B20u;
        DWORD args[3]={caller,actor,9999},frame[9]{};frame[3]=pointer(args)-4;
        inputObserve(4,frame);
        uint32_t counter=0;MenuCanvasTest::read(actor+0x54,&counter,4);
        check(counter==8,"completion observer does not replay handler or reset original animation counter");
        MenuCanvasTest::put(actor+0x54,uint32_t{0}); // original handler resets it
        for(uint32_t i=0;i<6;++i){const auto p=MenuCanvas::mainMenu.firstButton+i*0x90;
            uint32_t flags=0;MenuCanvasTest::read(p+4,&flags,4);flags&=~0x10000u;if(p==actor)flags&=~0x100000u;
            MenuCanvasTest::put(p+4,flags);}
        MenuCanvasTest::put(0x4D0A28,pending);
        MenuCanvasTest::put(actor+0x54,uint32_t{8}); // returning original painter writes final local counter
    }
    void producers(bool withClear=true,uint32_t pendingDuringDraw=0) {
        if(withClear)clear();
        const auto caller=arguments[0];arguments[0]=0x46AE63;
        observe(10,draws,arguments,0,reinterpret_cast<void*>(&blit));arguments[0]=caller;
        for(uint32_t i=0;i<6;++i) {
            draws[1]=MenuCanvas::mainMenu.firstButton+i*0x90;
            observe(1,draws,arguments,0,reinterpret_cast<void*>(&blit));
            if((pendingDuringDraw==10&&i==0)||(pendingDuringDraw==18&&i==5))activationComplete(pendingDuringDraw);
            if(i==2&&pendingDuringDraw&&pendingDuringDraw!=10&&pendingDuringDraw!=18)MenuCanvasTest::put(0x4D0A28,pendingDuringDraw);
            observe(10,draws,arguments,0,reinterpret_cast<void*>(&blit));
            observe(2,draws,arguments,0,reinterpret_cast<void*>(&blit));
        }
    }
    void repair(){begin();producers();end();check(publication.scaledPredicatesAllowed(),"main complete composition publishes interactive generation");}
    void point(uint32_t node,int64_t x,int64_t y) {
        const auto physical=plan->geometry().map_rect({x,y,x+1,y+1});
        check(bool(physical),"logical test point maps inside output");
        const auto px=int32_t(physical->left),py=int32_t(physical->top);
        input[1]=node;std::memcpy(inputMemory.data()+0x14,&px,4);std::memcpy(inputMemory.data()+0x10,&py,4);
    }
    static uint32_t __cdecl paletteBlit(QImageResearch* src,QImageResearch* dst,int x,int y) {
        QImageResearch decoded{};
        if(!MenuCanvasTest::read(pointer(src),&decoded,sizeof(decoded)))throw std::logic_error("palette mock source unreadable");
        uint32_t palette=0;uint16_t color=0;MenuCanvasTest::read(0x4EC390,&palette,4);MenuCanvasTest::read(palette+2,&color,2);
        auto* pixels=reinterpret_cast<uint16_t*>(dst->pixels);
        for(int row=0;row<decoded.height;++row)for(int col=0;col<decoded.width;++col)
            if(x+col>=0&&x+col<800&&y+row>=0&&y+row<600)pixels[(y+row)*800+x+col]=color;
        return 0;
    }
    void styledPrimitive(uint16_t width,uint16_t height,int x,int y,uint16_t color,bool opaque=false) {
        QImageResearch d{};MenuCanvasTest::read(source,&d,sizeof(d));d.width=width;d.height=height;d.flags=opaque?1:3;
        MenuCanvasTest::put(source,d);MenuCanvasTest::put(0xF1000002,color);
        arguments[0]=opaque?0x46AE63:0x4A2B1B;arguments[3]=uint32_t(x);arguments[4]=uint32_t(y);
        observe(10,draws,arguments,0,reinterpret_cast<void*>(&paletteBlit));
    }
};
void mainTests() {
    {
        MainFixture f;f.begin();f.clear();
        check(active&&active->nativeClear&&active->draws==0&&shadowDraws==0,"native-sized solid is observed without shadow blit or inactive pointer access");
        f.producers(false);f.end();
        check(commits==1&&lastFamily==&MenuCanvas::mainMenu&&publication.scaledPredicatesAllowed(),"main family owns completed composition");
        f.inputBegin();
        for(size_t i=0;i<MenuCanvas::mainMenu.buttons.size();++i) {
            const auto& b=MenuCanvas::mainMenu.buttons[i];
            f.point(MenuCanvas::mainMenu.firstButton+uint32_t(i)*0x90,b.x+20,b.y+20);
            check(f.hit()==0x41DADC,"each original main rectangle maps to existing hit continuation");
        }
        f.point(0x4E4850,400,130);check(f.hit()==0x41DADC,"overlap retains first original rectangle");
        f.point(0x4E48E0,400,130);check(f.hit()==0x41DADC,"overlap retains second original rectangle; original traversal decides priority");
        const int32_t margin=1;std::memcpy(f.inputMemory.data()+0x14,&margin,4);
        check(f.hit()==0x41DC62&&publication.scaledPredicatesAllowed(),"main margin misses without poisoning valid context");f.inputEnd();
    }
    for(uint32_t pending:{10u,18u}) {
        MainFixture f;f.activationFlags(pending);f.begin();f.producers(true,pending);f.end();
        check(commits==1&&lastFamily==&MenuCanvas::mainMenu&&publication.mayContainScaledPixels(),"draw-triggered supported transition can finish a complete visual");
        check(!publication.scaledPredicatesAllowed()&&!family(false)&&!family(true),"departing visual never renews interactive ownership or unscoped pending admission");
        f.point(0x4E4850,400,70);f.expectSuspended();
        MenuCanvasTest::put(0x4D0A28,uint32_t{7});f.expectSuspended();f.repair();f.expectHit();
    }
    for(uint32_t pending:{10u,18u}) {
        MainFixture f;MenuCanvasTest::put(0x4D0A28,pending);f.begin();
        check(!active&&commits==0,"pass cannot start with an already-pending main transition");
    }
    {
        MainFixture f;f.activationFlags(10);f.begin();f.clear();
        f.arguments[0]=0x46AE63;
        observe(10,f.draws,f.arguments,0,reinterpret_cast<void*>(&Fixture::blit));
        f.draws[1]=0x4E4850;observe(1,f.draws,f.arguments,0,reinterpret_cast<void*>(&Fixture::blit));
        f.activationComplete(10,0x12345678);f.end();
        check(commits==0&&!publication.scaledPredicatesAllowed(),"wrong draw-completion callsite cannot authorize a departing composition");
    }
    {
        Fixture f;MenuCanvasTest::put(0x4E50E0+4,uint32_t{0x10001});
        check(!family(),"main disabled/activation flag mask does not broaden selection admission");
    }
    for(unsigned cause=0;cause<5;++cause) {
        MainFixture f;f.begin();
        if(cause==0)f.producers(false);
        if(cause==1){f.clear(1);f.producers(false);}
        if(cause==2){f.clear();f.clear();f.producers(false);}
        if(cause==3)f.producers(true,9);
        if(cause==4){f.producers();MenuCanvasTest::put(0x4E4850+0x4C,uint32_t{0});}
        f.end();check(commits==0&&!publication.scaledPredicatesAllowed(),"missing/foreign clear, unsupported transition or changed callback cannot publish");
    }
    {
        MainFixture f;f.repair();f.point(0x4E4850,400,70);f.expectHit();
        MenuCanvasTest::put(0x4D0A24,uint32_t{10});MenuCanvasTest::put(0x4D0A28,uint32_t{10});
        MenuCanvasTest::put(0x5F7ED8+0x73,uint32_t{0x4E5050});f.input[1]=0x4E50E0;
        f.expectSuspended();check(!publication.scaledPredicatesAllowed(),"same physical target cannot reuse another family's committed input context");
        f.Fixture::repair();check(lastFamily==&MenuCanvas::selection,"original selection can regain ownership after main family");
        const int32_t x=1063,y=101;std::memcpy(f.inputMemory.data()+0x14,&x,4);std::memcpy(f.inputMemory.data()+0x10,&y,4);f.expectHit();
    }
    {
        MainFixture f;mainEnabled=false;f.begin();f.producers();f.end();
        check(commits==0&&f.hit()==0x12345678,"main opt-in off leaves its nodes on original predicate");
    }
    {
        MainFixture f;f.activationFlags(10);f.repair();f.point(0x4E48E0,400,140);f.inputBegin();
        check(f.hit()==0x41DADC&&publication.scaledPredicatesAllowed(),"disabled overlapping rectangle retains original blocking hit path, not a forced miss");f.inputEnd();
    }
    for(unsigned cause=0;cause<8;++cause) {
        MainFixture f;f.repair();f.begin();f.producers();
        if(cause==0)MenuCanvasTest::put(0x4E47C0+0x50,uint32_t{0x202});
        if(cause==1)MenuCanvasTest::put(0x4E4850+0x10,uint32_t{0x203});
        if(cause==2)MenuCanvasTest::put(0x4E4850+0x50,uint32_t{0});
        if(cause==3)MenuCanvasTest::put(0x4E4850+0x6F,uint32_t{0});
        if(cause==4)MenuCanvasTest::put(0x5F7ED8+0x73,uint32_t{0x4DF100});
        if(cause==5)MenuCanvasTest::put(0x5F7ED8+0x73,uint32_t{0x4DB6F8});
        if(cause==6)MenuCanvasTest::put(Fixture::outer+0x10,uint16_t{1200});
        if(cause==7){DWORD frame[9]{};inputObserve(5,frame);MenuCanvasTest::put(0x4E47C0+0x77,uint32_t{0});}
        f.end();check(commits==1&&!publication.scaledPredicatesAllowed(),"resource identity, links, modal insertion, wrapper geometry and cleanup invalidate candidate");
    }
    {
        MainFixture f;MenuCanvasTest::put(Fixture::outer+0x4A,int16_t{400});f.begin();
        check(!active&&commits==0,"reduced original wrapper clip cannot masquerade as a complete logical background");
    }
    {
        MainFixture f;MenuCanvasTest::put(0x4EC390,uint32_t{0});f.begin();f.producers();f.end();
        check(commits==0,"unreadable live palette rejects main raw shadow");
    }
    for(bool duringUnlock:{false,true}) {
        MainFixture f;f.repair();f.surface.notifyForReentry=false;
        (duringUnlock?f.surface.unlockReentry:f.surface.lockReentry)=4;
        f.begin();f.producers();f.end();
        check(commits==1&&!publication.scaledPredicatesAllowed(),"same-pointer resource lifecycle during Lock/Unlock cannot publish stale candidate");
        f.surface.lockReentry=f.surface.unlockReentry=0;f.repair();
    }
    {
        MainFixture f;f.begin();f.clear();
        f.styledPrimitive(800,600,0,0,0x07E0,true);
        f.styledPrimitive(4,9,700,580,0xF800); // transient font ramp
        for(uint32_t i=0;i<6;++i){f.draws[1]=MenuCanvas::mainMenu.firstButton+i*0x90;
            observe(1,f.draws,f.arguments,0,reinterpret_cast<void*>(&MainFixture::paletteBlit));
            const auto& b=MenuCanvas::mainMenu.buttons[i];f.styledPrimitive(180,100,b.x,b.y,0x001F);}
        f.end();check(commits==1,"main background/text/buttons compose with changing live palette");
        const auto glyph=plan->geometry().map_rect({700,580,701,581});
        const auto buttonPixel=plan->geometry().map_rect({400,70,401,71});
        const auto backgroundPixel=plan->geometry().map_rect({100,550,101,551});
        check(f.surface.pixel(unsigned(glyph->left),unsigned(glyph->top))==0xF800,"text retains palette at glyph submission after root palette restoration");
        check(f.surface.pixel(unsigned(buttonPixel->left),unsigned(buttonPixel->top))==0x001F,"child uses restored live palette");
        check(f.surface.pixel(unsigned(backgroundPixel->left),unsigned(backgroundPixel->top))==0x07E0,"opaque background retains earlier palette conversion");
    }
}
}
namespace {
void nativeMainTests() {
    MainBackdrop::enabled=true;MainBackdrop::thread=GetCurrentThreadId();
    std::vector<unsigned char> data(16+64*36*2);memcpy(data.data(),"DWCARD1\0",8);const uint32_t w=64,h=36;
    memcpy(data.data()+8,&w,4);memcpy(data.data()+12,&h,4);
    for(size_t i=16;i<data.size();i+=2){const uint16_t v=0x4444;memcpy(data.data()+i,&v,2);}
    MainBackdrop::artwork=OpeningFlow::CardImage::parse(data);
    {
        MainFixture f;f.begin();auto held=active->backdrop;check(bool(held),"actual Main controller selects shared native provider");f.clear();
        f.styledPrimitive(800,600,0,0,0x07E0,true);f.styledPrimitive(13,7,700,580,0xF800);
        for(uint32_t i=0;i<6;++i){f.draws[1]=MenuCanvas::mainMenu.firstButton+i*0x90;
            observe(1,f.draws,f.arguments,0,reinterpret_cast<void*>(&MainFixture::paletteBlit));
            const auto& b=MenuCanvas::mainMenu.buttons[i];f.styledPrimitive(180,100,b.x,b.y,0x001F);}
        f.end();check(held->composed&&publication.scaledPredicatesAllowed(),"native Main commits through original input publication");
        const auto glyph=plan->geometry().map_rect({700,580,701,581});
        check(f.surface.pixel(unsigned(glyph->left),unsigned(glyph->top))==0xF800,"native Main retains foreground font palette");
        check(f.surface.pixel(10,300)==0x4444,"native Main background fills sideband");
        const auto originalBackground=plan->geometry().map_rect({100,550,101,551});
        check(f.surface.pixel(unsigned(originalBackground->left),unsigned(originalBackground->top))==0x4444,"old opaque800background cannot cover new native artwork");
        auto next=MainBackdrop::borrow(held->target,999);check(next!=held,"completed layer cannot be borrowed into another frame");
        MainBackdrop::boundary();
    }
    {
        MainFixture f;f.begin();auto held=active->backdrop;f.clear();f.styledPrimitive(800,600,0,0,0x07E0,true);
        MenuCanvasTest::put(0x5F7ED8+0x73,uint32_t{0xDEADBEEF});
        f.arguments[0]=0x46B57E;f.arguments[3]=320;f.arguments[4]=50;
        observe(10,f.draws,f.arguments,0,reinterpret_cast<void*>(&MainFixture::paletteBlit));
        check(!active->good&&!held->mainOwns&&held->good,"changed Main owner relinquishes producer without losing shared history");
        f.end();check(!held->composed,"rejected inner Main cannot compose before outer owner");MainBackdrop::boundary();
    }
    {
        MainFixture f;f.begin();auto held=active->backdrop;f.clear();f.styledPrimitive(800,600,0,0,0x07E0,true);
        f.styledPrimitive(13,7,700,580,0xF800);coverageLost("fixture_gap_after_background_and_glyph");
        check(!active->good&&!held->good&&!held->mainOwns,"coverage gap invalidates shared outer/inner foreground ownership");
        f.end();check(!held->composed&&!publication.scaledPredicatesAllowed(),"incomplete shared foreground cannot publish after gap");
    }
    {
        MainFixture f;f.begin();auto held=active->backdrop;active.reset();coverageLost("fixture_gap_after_inner_owner_retired");
        check(!held->good&&!held->mainOwns&&MainBackdrop::current.expired(),"gap retires outer layer even when inner active pass is gone");
    }
    {
        MainFixture f;f.begin();auto held=active->backdrop;
        std::thread gap([]{coverageLost("fixture_foreign_gap");});gap.join();
        check(held->good&&held->mainOwns&&MainBackdrop::coverageGapPending.load(),"foreign gap defers shared mutation to owner");
        observe(2,f.draws,f.arguments,0,reinterpret_cast<void*>(&MainFixture::paletteBlit));
        check(!held->good&&!held->mainOwns&&!active->good&&!MainBackdrop::coverageGapPending.load(),"owner consumes foreign gap and retires incomplete foreground");
        f.end();check(!held->composed&&!publication.scaledPredicatesAllowed(),"foreign gap cannot publish incomplete frame");
    }
    MainBackdrop::enabled=false;MainBackdrop::artwork.reset();MainBackdrop::current.reset();
}
}
int main(){try{tests();check(assertions==193,"inherited selection regression count remains193");mainTests();nativeMainTests();std::cout<<"PASS "<<assertions<<" actual controller assertions with mock memory/COM/blits; no game/hardware proof\n";}
catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}}
