#include <Windows.h>
#include <map>
#include <vector>
#include <array>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include "RuntimeTestMemory.h"
namespace MenuCanvasTest {
std::map<uint32_t,std::vector<uint8_t>> memory;
auto region(uint32_t a){auto i=memory.upper_bound(a);if(i==memory.begin())return memory.end();--i;return uint64_t(a)-i->first<i->second.size()?i:memory.end();}
bool managed(uint32_t a,size_t){return region(a)!=memory.end();}
bool read(uint32_t a,void* p,size_t n){auto i=region(a);if(i==memory.end()||uint64_t(a)-i->first+n>i->second.size())return false;memcpy(p,i->second.data()+a-i->first,n);return true;}
bool write(uint32_t a,const void* p,size_t n){auto i=region(a);if(i==memory.end()||uint64_t(a)-i->first+n>i->second.size())return false;memcpy(i->second.data()+a-i->first,p,n);return true;}
void reserve(uint32_t a,size_t n){memory[a]=std::vector<uint8_t>(n);}
template<class T>void put(uint32_t a,T v){if(!write(a,&v,sizeof(v)))throw std::logic_error("test address");}
void scalar(uint32_t a,uint32_t v){reserve(a,4);put(a,v);}
}
#define HUD_CAPTAIN_TEST
#include <Research/HudCaptain/HudCaptain.cpp>
namespace Qfr {std::string quote(const std::string& s){return '"'+s+'"';}void event(const char*,const std::string&)noexcept{}}
using namespace HudCaptain;
unsigned assertions=0;
void check(bool b,const char* why){++assertions;if(!b)throw std::runtime_error(why);}
uint32_t ptr(const void* p){return uint32_t(reinterpret_cast<uintptr_t>(p));}
struct Vertex {float x,y,z,rhw;uint32_t color,specular;float u,v;};
struct Fixture {
    static constexpr uint32_t outer=0xF0001000,native=0xF0002000,surface=0xF0003000,slot=0x4E7BD0,aux=0xF0008000,unrelated=0xF000A000;
    uint32_t r[9]{},s[9]{};std::array<uint8_t,128> stack{};
    Fixture(){using namespace MenuCanvasTest;memory.clear();enabled=requested=true;thread=GetCurrentThreadId();epoch=1;generation=groups=gpu=cpu=errors=stripAttempts=stripClears=pausedGroups=0;frame={};publication={};scopes.clear();members.clear();reported.clear();inputSamples.clear();inputDepth=0;mayBeTransformed=originalRepaint=false;foreignBoundary=false;
        scalar(0x4D0A24,3);scalar(0x4E3134,3);scalar(0x7180B6,0);scalar(0xA38C88,0xF000C000);
        scalar(0x5FF670,2560);scalar(0x5FF674,1440);scalar(0x5FF694,154);scalar(0x5FF6D0,164);
        reserve(0x4F022C,20);put(0x4F022C,std::array<uint32_t,5>{44,0,0,2560,1440});
        scalar(0x71383C,0x42A7B0);scalar(0x713840,0x4022A0);scalar(0x4E5E80,1);scalar(0xA387B4,surface);scalar(0xA39B78,0);
        scalar(0xA39B64,0xF0000000);scalar(0xF0000000,outer);reserve(outer,0x90);put(outer,uint16_t{0x8000});put(outer+0x20,native);
        put(outer+0x46,std::array<int16_t,4>{0,0,2560,1440});reserve(native,24);QImageResearch image{};image.flags=0x201;image.pixel_bits=16;image.width=2560;image.height=1440;image.surface=surface;put(native,image);
        reserve(0x4F01A8,20);put(0x4F01A8,std::array<uint32_t,5>{44,0,0,2560,1286});
        reserve(0x5F7ED8,0x90);put(0x5F7ED8+4,uint32_t{4});put(0x5F7ED8+0x73,rootAddress);
        reserve(rootAddress,0x90);put(rootAddress,uint32_t{0x46});put(rootAddress+4,uint32_t{0x800004});put(rootAddress+8,int16_t{164});put(rootAddress+10,int16_t{1286});put(rootAddress+12,uint16_t{492});put(rootAddress+14,uint16_t{51});put(rootAddress+0x77,uint32_t{0x5F7ED8});put(rootAddress+0x73,uint32_t{0x4E7EA0});
        for(unsigned i=0;i<6;++i){const auto a=slot+i*0x90;reserve(a,0x90);put(a,0x59+i);put(a+4,uint32_t{1});put(a+8,int16_t(i*77));put(a+12,uint16_t{77});put(a+14,uint16_t{51});put(a+0x77,rootAddress);put(a+0x6B,i?a-0x90:0);put(a+0x4C,uint32_t{0x47B610});put(a+0x48,uint32_t{0x47B1A0});}
        reserve(aux,0x90);put(aux,uint32_t{0x6A});put(aux+4,uint32_t{1});put(aux+10,int16_t{-90});put(aux+12,uint16_t{25});put(aux+14,uint16_t{30});put(aux+0x77,slot);put(slot+0x73,aux);
        for(auto a:{0x4E5E90u,0x4E5F20u,0x4E6040u,0x4E60D0u,0x4E6160u}){reserve(a,0x90);put(a+4,uint32_t{4});put(a+8,int16_t{626});put(a+10,int16_t{1261});put(a+12,uint16_t{168});put(a+14,uint16_t{179});}
        reserve(unrelated,0x90);put(unrelated+0x77,uint32_t{0x5F7ED8});
    }
    void begin(){r[1]=rootAddress;observe(1,r,s,0x1000);check(frame.active,"known group begins");observe(2,r,s,0x1000);r[1]=slot;observe(1,r,s,0xF00);}
    void complete(){observe(2,r,s,0xF00);observe(3,r,s,0x1000);}
    uint32_t route(uint32_t node,int x,int y,int parentX,int parentY){r[1]=node;r[3]=ptr(stack.data())-4;
        const int32_t xx=x-parentX,yy=y-parentY;memcpy(stack.data()+0x14,&xx,4);memcpy(stack.data()+0x10,&yy,4);memcpy(stack.data()+0x34,&parentX,4);memcpy(stack.data()+0x38,&parentY,4);
        inputScope(1);const auto result=predicate(r,0x12345678);inputScope(2);return result;}
};
unsigned blits=0;QImageResearch submitted{};std::vector<uint8_t> submittedPixels;
uint32_t __cdecl blit(QImageResearch* src,QImageResearch*,int x,int y){++blits;submitted=*src;submittedPixels.assign(reinterpret_cast<uint8_t*>(src->pixels),reinterpret_cast<uint8_t*>(src->pixels)+size_t(src->width)*src->height);
    std::array<int16_t,4> clip{};check(read(value(0xA39B78),clip)&&clip[2]>0&&clip[3]>0,"private transformed CPU clip");check(x>794&&y>=1286,"CPU primitive moved into free reserved strip");SetLastError(456);return 0xAABBCCDD;}
int main(){try{
    {auto t=Transform::fit(2560,1440,164,1286,794,1286);check(bool(t),"native1440 trial fits");check(t->n*10==t->d*24,"derived scale2.4");check(t->edge(1286+64,false)<=1440,"padded frame stays in reserve");
        for(int x=164;x<164+492;++x){const int p=t->edge(x,true);check(t->cell(p,true)==x,"edge and private inverse agree");}
        check(t->edge(1286-90,false)<1286,"auxiliary icon row extends above baseline without scene copy");
        check(!Transform::fit(800,600,164,446,794,446),"no free strip retains original fallback");
        auto four=Transform::fit(3840,2160,164,2006,794,2006);check(four&&four->edge(2070,false)<=2160,"4K simulated reserve clamp");}
    for(const auto [w,h]:{std::pair{1024,768},std::pair{1280,800},std::pair{1366,768},std::pair{1920,1080},std::pair{1920,1200},std::pair{2560,1440},std::pair{3840,2160},std::pair{3440,1440}}){
        const auto t=Transform::fit(unsigned(w),unsigned(h),164,h-154,794,h-154);
        check(!t||(t->edge(164,true)>794&&t->edge(164+544,true)<=w&&t->edge(h-154+64,false)<=h),"simulated display derives safe trial bounds or retains original");}
    {Fixture f;f.begin();const std::array<Vertex,3> original={{{164,1286,0.5f,1,0x80445566,0x12345678,0,0},{292,1286,0.5f,1,0x80445566,0x12345678,1,0},{164,1350,0.5f,1,0x80445566,0x12345678,0,1}}};const auto before=original;
        Draw d(0x1C4,original.data(),3);check(d.adapted,"TL primitive transformed");check(memcmp(original.data(),before.data(),sizeof(original))==0,"game vertex array unchanged");
        for(unsigned i=0;i<3;++i)check(memcmp(reinterpret_cast<const uint8_t*>(&original[i])+8,d.vertices.data()+i*32+8,24)==0,"Z/RHW/color/specular/UV preserved");
        d.result(0);f.complete();check(publication.valid&&gpu==1,"actual group publishes after last descendant");
        const auto t=publication.transform;check(f.route(Fixture::slot,t.edge(174,true),t.edge(1296,false),164,1286)==0x41DADC,"scaled slot selects original hit branch");
        check(f.route(Fixture::slot,174,1296,164,1286)==0x41DC62,"old footprint no longer hits");
        check(f.route(Fixture::aux,t.edge(174,true),t.edge(1206,false),164,1286)==0x41DADC,"negative-Y auxiliary control maps correctly");
        check(f.route(Fixture::unrelated,1000,1000,0,0)==0x12345678,"world/unrelated input untouched");}
    {Fixture f;f.begin();std::array<Vertex,3> v{};for(auto& p:v){p.x=164;p.y=1196;p.rhw=1;}Draw d(0x1C4,v.data(),3);check(d.adapted,"visible auxiliary sprites transform above scene border");
        inputScope(1);inputScope(2);f.complete();check(!publication.valid,"reentrant input revokes candidate");check(!enabled,"bad candidate requests original fallback");
        f.r[1]=rootAddress;observe(1,f.r,f.s,0x1000);observe(2,f.r,f.s,0x1000);observe(3,f.r,f.s,0x1000);check(!mayBeTransformed,"complete original repaint retires transformed input");}
    {Fixture f;f.begin();std::array<uint8_t,6> pixels={0,1,2,2,1,0};QImageResearch src{};src.flags=3;src.unknown_02=255;src.and_mask=255;src.pixel_bits=8;src.width=3;src.height=2;src.pitch=5;src.pixels=ptr(pixels.data());blits=0;
        const auto old=src;SetLastError(123);const auto result=software(blit,&src,reinterpret_cast<QImageResearch*>(Fixture::outer),167,1288);
        check(result==0xAABBCCDD&&GetLastError()==456,"CPU result and LastError retained");check(blits==1,"original CPU renderer called once");check(src.width==old.width&&src.height==old.height&&src.pixels==old.pixels&&src.pitch==3,"descriptor retained with original raw-pitch side effect");
        check(submitted.flags==old.flags&&submitted.and_mask==old.and_mask&&submitted.xor_value==old.xor_value,"palette/mask/color-key semantics delegated unchanged");
        check(std::find(submittedPixels.begin(),submittedPixels.end(),uint8_t{0})!=submittedPixels.end()&&std::find(submittedPixels.begin(),submittedPixels.end(),uint8_t{1})!=submittedPixels.end(),"zero and nonzero indices survive without black-as-alpha");
        check(value(0xA39B78)==0,"CPU clip restored");f.complete();check(publication.valid&&cpu==1,"CPU-only group publication");}
    {Fixture f;f.begin();Vertex v{};v.x=164;v.y=1286;Draw d(0x112,&v,1);check(!d.adapted&&!frame.good,"unknown GPU format fails explicitly");f.complete();check(!publication.valid,"unsupported producer cannot publish input");}
    {Fixture f;using namespace MenuCanvasTest;put(0x4D0A24,uint32_t{2});put(0x71383C,uint32_t{0x42AB00});put(0x7180B6,uint32_t{1});
        f.begin();Vertex v{};v.x=164;v.y=1286;Draw d(0x1C4,&v,1);check(d.adapted,"paused mission renders enlarged captain group");f.complete();
        check(publication.valid&&pausedGroups==1,"paused graphics publish without activating modal input");const auto t=publication.transform;
        check(f.route(Fixture::slot,t.edge(174,true),t.edge(1296,false),164,1286)==0x41DC62,"Escape open flag blocks captain input");
        put(0x7180B6,uint32_t{0});const uint32_t modal=0xF000D000;reserve(modal,0x90);put(modal+4,uint32_t{0x80});put(modal+0x77,uint32_t{0x5F7ED8});put(modal+0x6B,rootAddress);put(0x5F7ED8+0x73,modal);
        f.begin();Draw again(0x1C4,&v,1);f.complete();check(publication.valid,"modal does not shrink underlying captain graphics");
        check(f.route(Fixture::slot,t.edge(174,true),t.edge(1296,false),164,1286)==0x41DC62,"visible blocking head keeps confirmation/preferences priority");
        put(0x5F7ED8+0x73,rootAddress);put(0x4D0A24,uint32_t{3});put(0x71383C,uint32_t{0x42A7B0});boundary("test_resume");
        f.begin();Draw resumed(0x1C4,&v,1);f.complete();check(publication.valid,"fresh resume republishes transformed group");
        check(f.route(Fixture::slot,t.edge(174,true),t.edge(1296,false),164,1286)==0x41DADC,"resume restores captain input after modal removed");
        put(0x4D0A24,uint32_t{2});put(0x71383C,uint32_t{0x42AB00});put(0x4E3134,uint32_t{11});MenuProbe::Node n{};check(!root(n),"fleet-origin paused owner cannot enter mission HUD adapter");}
    {Fixture f;D3DRECT original{164,1286,2396,1440};SetLastError(789);StripClear c(0x4023F2,0xF000C000,1,&original,1,0,1.0f,0);
        check(c.adapted&&stripClears==1,"known original bottom strip clear admitted");check(original.x2==2396,"game clear rectangle remains untouched");
        check(c.rectangle.x1==164&&c.rectangle.y1==1286&&c.rectangle.x2==2560&&c.rectangle.y2==1440,"only right edge extends; scene and left panel untouched");check(GetLastError()==789,"clear adapter preserves LastError");
        c.result(0);check(errors==0,"successful original clear result retained");
        StripClear other(0x40248E,0xF000C000,1,&original,1,0,1.0f,0);check(!other.adapted&&other.data(&original)==&original,"scene clear is not overridden");
        StripClear wrong(0x4023F2,0xF000C001,1,&original,1,0,1.0f,0);check(!wrong.adapted,"wrong viewport leaves original arguments");
        original.x2=2395;StripClear extent(0x4023F2,0xF000C000,1,&original,1,0,1.0f,0);check(!extent.adapted,"unexpected rectangle is explicitly rejected");
        original.x2=2396;using namespace MenuCanvasTest;put(0x4D0A24,uint32_t{2});put(0x71383C,uint32_t{0x42AB00});StripClear paused(0x4023F2,0xF000C000,1,&original,1,0,1.0f,0);check(paused.adapted,"paused mission retains same strip correction");
        put(0xA387B4,uint32_t{0xF000C008});StripClear retired(0x4023F2,0xF000C000,1,&original,1,0,1.0f,0);check(!retired.adapted,"retired target alias cannot be adapted");}
    std::cout<<"PASS "<<assertions<<" captain geometry/primitive/input assertions\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
