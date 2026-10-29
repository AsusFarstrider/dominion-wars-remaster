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
template<class T>void put(uint32_t a,T v){if(!write(a,&v,sizeof(v)))throw std::logic_error("test memory write");}
void scalar(uint32_t a,uint32_t v){reserve(a,4);put(a,v);}
}
#pragma warning(disable:4505)
#define HUD_ASSEMBLY_TEST
#include <Research/HudAssembly/HudAssembly.cpp>
namespace Qfr {std::string quote(const std::string& s){return '"'+s+'"';}void event(const char*,const std::string&)noexcept{}}
using namespace HudAssembly;
unsigned assertions=0;
void check(bool condition,const char* why){++assertions;if(!condition)throw std::runtime_error(why);}
struct Vertex{float x,y,z,rhw;uint32_t color,specular;float u,v;};
struct Fixture {
    static constexpr uint32_t outer=0xF0001000,native=0xF0002000,surface=0xF0003000,slot=0x4E7BD0,circle=0x4DF708,mask=0x4E69D0;
    uint32_t r[9]{},args[9]{};std::array<uint8_t,128> stack{};
    Fixture(){using namespace MenuCanvasTest;memory.clear();requested=enabled=true;ready=false;ownerThread=GetCurrentThreadId();epoch=1;generation=gpu=cpu=errors=groups=radarPasses=previewPasses=queued=released=stripClears=radarClears=previewClears=geometryCommits=0;
        inputDepth=radarDepth=admissionDepth=0;foreignBoundary=false;snapshot={};frame={};scopes.clear();points.clear();retained.clear();retainedBytes=0;inputSamples.clear();reported.clear();withdraw();
        scalar(0x4D0A24,3);scalar(0x71383C,0x42A7B0);scalar(0x713840,0x4022A0);scalar(0x4E3134,3);scalar(0x713898,1);scalar(0x7180B6,0);scalar(0xA387B4,surface);scalar(0xA38C88,0xF0004000);scalar(0xA38D74,0xF0005000);scalar(0xA39B78,0);
        scalar(0xA39B64,0xF0000000);scalar(0xF0000000,outer);reserve(outer,0x90);put(outer,uint16_t{0x8000});put(outer+0x20,native);put(outer+0x46,std::array<int16_t,4>{0,0,2560,1440});
        reserve(native,24);QImageResearch image{};image.flags=0x201;image.pixel_bits=16;image.width=2560;image.height=1440;image.surface=surface;put(native,image);
        reserve(0x5FF670,0x80);put(0x5FF670,uint32_t{2560});put(0x5FF674,uint32_t{1440});put(0x5FF694,uint32_t{154});put(0x5FF698,uint32_t{1286});put(0x5FF6A4,uint32_t{2560});put(0x5FF6A8,uint32_t{1286});put(0x5FF6AC,uint32_t{643});put(0x5FF6B0,uint32_t{1280});
        put(0x5FF6BC,uint32_t{78});put(0x5FF6C0,uint32_t{82});put(0x5FF6C4,uint32_t{1355});put(0x5FF6C8,uint32_t{0});put(0x5FF6CC,uint32_t{1270});put(0x5FF6D0,uint32_t{164});put(0x5FF6D4,uint32_t{1440});put(0x5FF6D8,uint32_t{160});put(0x5FF6DC,uint32_t{103});put(0x5FF6E0,uint32_t{164});put(0x5FF6E4,uint32_t{1337});
        reserve(0x4F01A8,44*4);Viewport v={44,0,0,2560,1286,0xBF800000,0x3F800000,0x40000000,0x40000000,0,0x3F800000};for(unsigned i=0;i<4;++i)put(0x4F01A8+i*44,v);
        reserve(0x4F0308,64);std::array<float,16> matrix{};matrix[0]=1.802130103111267f;matrix[5]=3.5f;matrix[10]=1;matrix[11]=1;matrix[14]=-1;put(0x4F0308,matrix);
        reserve(0x5F7ED8,0x90);put(0x5F7ED8+4,uint32_t{4});reserve(0x4DC530,0x90);put(0x4DC530,uint32_t{0x63});put(0x4DC534,uint32_t{0x40});put(0x4DC53C,uint16_t{2560});put(0x4DC53E,uint16_t{1286});put(0x4DC530+0x77,uint32_t{0x5F7ED8});
        const std::array<std::array<int16_t,4>,8> geometry={{{164,1337,160,103},{324,1337,178,99},{502,1337,53,103},{164,1286,492,51},{626,1261,168,179},{555,1337,71,99},{626,1261,168,179},{0,1270,165,170}}};
        uint32_t previous=0x4DC530;
        for(unsigned i=0;i<8;++i){const auto a=roots[i].address;reserve(a,0x90);put(a,roots[i].id);put(a+4,uint32_t{4});put(a+8,geometry[i]);put(a+0x48,roots[i].input);put(a+0x4C,roots[i].draw);put(a+0x77,uint32_t{0x5F7ED8});put(a+0x6B,previous);previous=a;}
        put(0x5F7ED8+0x73,previous);put(0x4E5FBE,uint16_t{51});
        reserve(slot,0x90);put(slot,uint32_t{0x59});put(slot+4,uint32_t{1});put(slot+12,uint16_t{77});put(slot+14,uint16_t{51});put(slot+0x77,uint32_t{0x4E5FB0});put(0x4E5FB0+0x73,slot);
        reserve(circle,0x90);put(circle,uint32_t{0x64});put(circle+4,uint32_t{2});put(circle+8,int16_t{82});put(circle+10,int16_t{85});put(circle+12,uint16_t{78});put(circle+0x77,uint32_t{0x4DF678});put(0x4DF678+0x73,circle);
        reserve(mask,0x90);put(mask,uint32_t{0x52});put(mask+4,uint32_t{0x400001});put(mask+12,uint16_t{25});put(mask+14,uint16_t{25});put(mask+0x77,uint32_t{0x4E6040});put(0x4E6040+0x73,mask);
    }
    void beginGroup(int root){r[1]=roots[root].address;observe(1,r,args,0x1000);check(frame.active,"known HUD subtree begins");}
    void complete(){observe(2,r,args,0x1000);observe(3,r,args,0x1000);}
    void prime(int root){beginGroup(root);Vertex v{};v.x=300;v.y=1350;Draw d(0x40F800,0x1C4,&v,1);check(d.adapted,"owned sprite transforms");complete();check(published[size_t(root)].valid,"matching subtree publication");}
    void input(uint32_t node,int x,int y,int px,int py){r[1]=node;r[3]=ptr(stack.data())-4;const int32_t xx=x-px,yy=y-py;memcpy(stack.data()+0x14,&xx,4);memcpy(stack.data()+0x10,&yy,4);memcpy(stack.data()+0x34,&px,4);memcpy(stack.data()+0x38,&py,4);}
    int cached(int offset){int result=0;memcpy(&result,stack.data()+offset,4);return result;}
};
unsigned originalCalls=0;uint32_t submittedPointer=0;std::vector<uint8_t> submittedPixels;
uint32_t __cdecl originalPlain(uint32_t context,uint32_t,uint32_t format,const void* v,uint32_t count,uint32_t){++originalCalls;submittedPointer=ptr(v);Draw nested(0x4A5EE9,format,v,count);check(!nested.adapted,"admitted primitive is never transformed twice");
    if(value(context+0xAC)){const auto command=value(context+0x10)+value(context+0x14)*192;write(command+8,ptr(v));write(context+0x14,value(context+0x14)+1);}return 1;}
uint32_t __cdecl originalBegin(uint32_t context){write(context+0x14,uint32_t{0});return 123;}
uint32_t __cdecl originalBlit(QImageResearch* src,QImageResearch*,int,int){++originalCalls;submittedPixels.assign(reinterpret_cast<uint8_t*>(src->pixels),reinterpret_cast<uint8_t*>(src->pixels)+size_t(src->width)*src->height);SetLastError(456);return 789;}
D3DMATRIX deviceProjection{};unsigned binds=0,sceneCalls=0;float seenProjection=0;
HRESULT STDMETHODCALLTYPE getProjection(IDirect3DDevice3*,D3DTRANSFORMSTATETYPE,D3DMATRIX* out){*out=deviceProjection;return S_OK;}
HRESULT STDMETHODCALLTYPE setProjection(IDirect3DDevice3*,D3DTRANSFORMSTATETYPE,D3DMATRIX* in){deviceProjection=*in;++binds;return S_OK;}
uint32_t __cdecl originalScene(void*,void*,void*){++sceneCalls;memcpy(&seenProjection,&deviceProjection,4);check(value(0x4F0308)!=0,"persistent world projection remains available to picker");SetLastError(999);return 42;}
int main(){try{
    {auto l=Layout::fit(2560,1440,165,170,51);check(bool(l),"native layout fits");check(l->transform.n==1440&&l->transform.d==600,"one common scale2.4");check(l->transform.dx==320&&l->scene.bottom==1070&&l->radar.top==1032,"shared policy matches reference edges");check(l->preview.left==714&&l->preview.top==1193&&l->preview.right==1098&&l->preview.bottom==1440,"preview native viewport edges");
        for(int x=0;x<800;++x)check(l->transform.cell(l->transform.edge(x,true),true)==x,"integer-edge input inverse matches source cells");
        const auto t=l->group(164,1286,164,446);check(t.edge(164,true)==714&&t.edge(1286,false)==1070,"group subtracts native source anchor exactly once");}
    for(const auto [w,h]:{std::pair{1024,768},std::pair{1280,800},std::pair{1366,768},std::pair{1920,1080},std::pair{1920,1200},std::pair{2560,1440},std::pair{3840,2160},std::pair{3440,1440}}){const auto l=Layout::fit(unsigned(w),unsigned(h),165,170,51);check(l&&l->scene.bottom>0&&l->preview.bottom==h&&l->radar.right<=w&&l->reserve.bottom==h,"simulated display derives coherent role bounds");}
    {Fixture f;prepareFrame();check(ready&&errors==0,"actual geometry adapter prepares");check(value(0x5FF694)==370&&value(0x5FF698)==1070&&value(0x5FF6A8)==1070&&value(0x5FF6AC)==535,"reserve scene clipping and picker globals agree");uint16_t height=0;read(0x4DC53E,height);check(height==1070,"world input bounds stay physical");
        check(value(0x5FF6CC)==1270&&value(0x5FF6E4)==1337&&value(0x5FF6D8)==160,"radar and preview source globals retained");MenuProbe::Node node{};MenuProbe::node(reader,0x4E5FB0,node);check(node.x==164&&node.y==1286,"packed source HUD origins unchanged");
        const auto before=value(0x4F0308);prepareFrame();check(value(0x4F0308)==before,"projection never compounds per frame");
        f.prime(3);const auto tr=snapshot.layout.transform;f.input(Fixture::slot,tr.edge(174,true),tr.edge(1296,false),164,1286);const auto x=f.cached(0x14),y=f.cached(0x10);inputScope(1);check(pointBefore(f.r,0x12345678)==0x12345678,"original shape dispatch is retained");check(f.cached(0x14)==10&&f.cached(0x10)==10,"rectangle receives private logical point");pointAfter(ptr(f.stack.data()));check(f.cached(0x14)==x&&f.cached(0x10)==y,"next sibling sees original cached point");inputScope(2);
        f.prime(7);f.input(Fixture::circle,tr.edge(82,true),tr.edge(1355,false),0,1270);inputScope(1);check(pointBefore(f.r,0x23456789)==0x23456789,"circle takes original radius-test branch");check(f.cached(0x14)==82&&f.cached(0x10)==85,"circle sees source center");pointAfter(ptr(f.stack.data()));inputScope(2);
        f.prime(4);f.input(Fixture::mask,tr.edge(636,true),tr.edge(1271,false),626,1261);inputScope(1);check(pointBefore(f.r,0x34567890)==0x34567890,"masked control keeps original transparent-pixel test");check(f.cached(0x14)==10&&f.cached(0x10)==10,"masked resource receives original source coordinates");pointAfter(ptr(f.stack.data()));inputScope(2);
        MenuCanvasTest::put(0x7180B6,uint32_t{1});inputScope(1);check(pointBefore(f.r,0x12345678)==0x41DC62,"Escape blocks HUD input");inputScope(2);MenuCanvasTest::put(0x7180B6,uint32_t{0});
        MenuCanvasTest::put(0x4D0A24,uint32_t{2});MenuCanvasTest::put(0x71383C,uint32_t{0x42AB00});boundary("pause");prepareFrame();f.prime(3);check(errors==0,"paused geometry and rendering retain one layout");
        MenuCanvasTest::put(0x4D0A24,uint32_t{7});MenuCanvasTest::put(0x713840,uint32_t{0});prepareFrame();check(!ready&&value(0x5FF698)==1286,"leaving mission restores original scene geometry");}
    {Fixture f;prepareFrame();f.beginGroup(3);const Vertex v={164,1286,0.5f,1,0x80445566,0x12345678,0.125f,0.75f};const auto before=v;Draw d(0x40F800,0x1C4,&v,1);check(d.adapted&&memcmp(&before,&v,sizeof(v))==0,"GPU private copy leaves original unchanged");check(memcmp(reinterpret_cast<const uint8_t*>(&v)+8,d.vertices.data()+8,24)==0,"depth RHW color and UV bytes preserved");f.complete();
        Draw world(0x401000,0x1C4,&v,1);check(!world.adapted,"world draw remains native");}
    {Fixture f;prepareFrame();f.beginGroup(1);std::array<uint8_t,6> pixels={0,1,2,2,1,0};QImageResearch src{};src.flags=3;src.pixel_bits=8;src.and_mask=255;src.width=3;src.height=2;src.pitch=5;src.pixels=ptr(pixels.data());originalCalls=0;
        check(software(originalBlit,&src,reinterpret_cast<QImageResearch*>(Fixture::outer),330,1340)==789&&GetLastError()==456,"original CPU result and LastError retained");check(originalCalls==1&&src.pitch==3&&src.width==3&&value(0xA39B78)==0,"CPU palette blit called once and clip restored");check(!submittedPixels.empty()&&std::find(submittedPixels.begin(),submittedPixels.end(),uint8_t{0})!=submittedPixels.end(),"source zero index is not invented alpha");f.complete();}
    {Fixture f;prepareFrame();D3DRECT r{164,1070,2396,1440};Clear strip(0x4023F2,0xF0004000,1,&r,1,0,1,0);check(strip.adapted&&strip.rectangle.x1==0&&strip.rectangle.y1==1070&&strip.rectangle.x2==2560,"existing strip clear covers new full reserve");check(r.x1==164&&r.x2==2396,"caller strip rectangle unchanged");
        r={0,1270,164,1440};Clear radarClear(0x459107,0xF0004000,1,&r,1,0,1,0);check(radarClear.adapted&&radarClear.rectangle.x1==320&&radarClear.rectangle.y1==1032,"radar private clear matches physical viewport");
        r={164,1337,324,1440};Clear previewClear(0x4027C8,0xF0004000,1,&r,3,0,1,0);check(previewClear.adapted&&previewClear.rectangle.x1==714&&previewClear.rectangle.y1==1193,"preview private clear matches native viewport");Clear world(0x40248E,0xF0004000,1,&r,3,0,1,0);check(!world.adapted,"unrelated scene clear unchanged");}
    {Fixture f;prepareFrame();using namespace MenuCanvasTest;const uint32_t context=0xF0010000,commands=0xF0020000;reserve(context,0xC0);put(context+0xAC,uint32_t{1});put(context+0x10,commands);reserve(commands,192);originals[3]=reinterpret_cast<void*>(&originalPlain);originals[5]=reinterpret_cast<void*>(&originalBegin);
        Vertex v{82,1355,0.5f,1,0xFFFFFFFF,0,0,0};originalCalls=0;radarDepth=1;check(plain(context,3,0x1C4,&v,1,0)==1&&originalCalls==1,"original queued submission called once");radarDepth=0;check(retained.size()==1&&queued==1&&submittedPointer!=ptr(&v),"queued pointer retains owned copy after return");
        Vertex copy{};check(bytes(submittedPointer,&copy,sizeof(copy))&&copy.x==float(snapshot.layout.transform.vertex(82,true)),"queued copy retains transformed detail");check(copy.z==v.z&&copy.color==v.color&&v.x==82,"queued copy preserves depth and source cache");
        check(begin(context)==123&&retained.empty()&&retainedBytes==0&&released==1,"copy released only after original queue reset");}
    {Fixture f;prepareFrame();IDirect3DDevice3Vtbl table{};table.GetTransform=getProjection;table.SetTransform=setProjection;IDirect3DDevice3 device{&table};MenuCanvasTest::put(0xA38D74,ptr(&device));float physical=snapshot.physicalP00;memcpy(&deviceProjection,&physical,4);binds=sceneCalls=0;
        check(preview(originalScene,0x4027F2,&device,reinterpret_cast<void*>(0xF0004000),nullptr)==42&&GetLastError()==999,"preview original callback once with native result");float restored=0;memcpy(&restored,&deviceProjection,4);check(sceneCalls==1&&binds==2&&seenProjection==snapshot.originalP00&&restored==physical,"preview processing projection isolated and restored");check(value(0x4F0308)==*reinterpret_cast<uint32_t*>(&physical),"picker projection is never temporarily rewritten");}
    std::cout<<"PASS "<<assertions<<" combined HUD geometry/primitive/input/lifetime assertions; no game or hardware proof\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
