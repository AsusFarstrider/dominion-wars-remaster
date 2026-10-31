// Actual production CompatVtable, visitor, enumeration and dispatch. Only logging,
// recorder/probe and adapter bodies are fixture callees; no game/GPU is loaded.
#include <Windows.h>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <Common/CompatVtable.h>
#include <Direct3d/Visitors/Direct3dDeviceVtblVisitor.h>
#include <Direct3d/Visitors/Direct3dViewportVtblVisitor.h>
namespace {
unsigned checks=0,locks=0,draws=0,indexed=0,clears=0,compatCalls=0,assemblyDraw=0,captainDraw=0,assemblyClear=0,captainClear=0,results=0;
bool adapt=true;void* originalData=nullptr;D3DRECT* originalRect=nullptr;WORD* originalIndices=nullptr;
void check(bool v,const char* why){++checks;if(!v)throw std::runtime_error(why);}
struct Lock {Lock(){++locks;}};
HRESULT STDMETHODCALLTYPE primitive(IDirect3DDevice3*,D3DPRIMITIVETYPE kind,DWORD format,void* data,DWORD count,DWORD flags){
 ++draws;check(kind==D3DPT_TRIANGLELIST&&format==D3DFVF_TLVERTEX&&count==1&&flags==17,"primitive arguments preserved");
 check(adapt?data!=originalData&&static_cast<D3DTLVERTEX*>(data)->sx==1012:data==originalData,"primitive adapter data reaches original");return E_FAIL;}
HRESULT STDMETHODCALLTYPE indexedPrimitive(IDirect3DDevice3*,D3DPRIMITIVETYPE kind,DWORD format,void* data,DWORD count,WORD* indices,DWORD n,DWORD flags){
 ++indexed;check(kind==D3DPT_TRIANGLELIST&&format==D3DFVF_TLVERTEX&&count==1&&indices==originalIndices&&n==3&&flags==19,"indexed arguments and indices preserved");
 check(adapt?data!=originalData&&static_cast<D3DTLVERTEX*>(data)->sx==1012:data==originalData,"indexed adapter data reaches original");return S_FALSE;}
HRESULT STDMETHODCALLTYPE clear(IDirect3DViewport3*,DWORD n,D3DRECT* r,DWORD flags,D3DCOLOR color,D3DVALUE z,DWORD stencil){
 ++clears;check(n==1&&flags==D3DCLEAR_TARGET&&color==0x123456&&z==0.5f&&stencil==7,"clear scalar arguments preserved");
 check(adapt?r!=originalRect&&r->x1==1012:r==originalRect,"clear adapter rectangle reaches original");return E_INVALIDARG;}
HRESULT STDMETHODCALLTYPE begin(IDirect3DDevice3*){return S_OK;}
HRESULT STDMETHODCALLTYPE oldDraw(IDirect3DDevice2*,D3DPRIMITIVETYPE,D3DVERTEXTYPE,void*,DWORD,DWORD){return S_OK;}
HRESULT STDMETHODCALLTYPE oldClear(IDirect3DViewport3*,DWORD,D3DRECT*,DWORD){return S_OK;}
HRESULT STDMETHODCALLTYPE renderTarget(IDirect3DDevice3*,IDirectDrawSurface4*,DWORD){return E_FAIL;}
HRESULT STDMETHODCALLTYPE compatTarget(IDirect3DDevice3*,IDirectDrawSurface4*,DWORD){++compatCalls;return S_OK;}
}
namespace {
template<class T> constexpr void setCompatVtable(T& t){
 if constexpr(std::is_same_v<T,IDirect3DDevice3Vtbl>)t.SetRenderTarget=&compatTarget;
}
}
namespace HudAssembly {
Draw::Draw(uintptr_t,uint32_t,const void* data,uint32_t count) noexcept{
 ++assemblyDraw;if(adapt&&count==1){vertices.resize(sizeof(D3DTLVERTEX));memcpy(vertices.data(),data,vertices.size());
 reinterpret_cast<D3DTLVERTEX*>(vertices.data())->sx+=1000;adapted=true;}}
void Draw::result(int64_t) noexcept{++results;}
Clear::Clear(uintptr_t,uintptr_t,uint32_t,const D3DRECT* r,uint32_t,uint32_t,float,uint32_t) noexcept{
 ++assemblyClear;if(adapt){rectangle=*r;rectangle.x1+=1000;adapted=true;}}
void Clear::result(int64_t) noexcept{++results;}
}
namespace HudCaptain {
Draw::Draw(uint32_t,const void*,uint32_t) noexcept{++captainDraw;}
void Draw::result(int64_t) noexcept{++results;}
StripClear::StripClear(uintptr_t,uintptr_t,uint32_t,const D3DRECT*,uint32_t,uint32_t,float,uint32_t) noexcept{++captainClear;}
void StripClear::result(int64_t) noexcept{++results;}
}
int main(){try{
 check(!(getCompatVtable<IDirect3DDevice3Vtbl>().DrawPrimitive)&&!(getCompatVtable<IDirect3DDevice3Vtbl>().DrawIndexedPrimitive)&&!(getCompatVtable<IDirect3DViewport3Vtbl>().Clear2),"all functional slots lack Compat implementations");
 for(unsigned mode=0;mode<3;++mode){
  Qfr::recording=mode==1;Compat::Log::level=mode==2?Config::Settings::LogLevel::DEBUG:Config::Settings::LogLevel::INFO;
  IDirect3DDevice3Vtbl d{};d.DrawPrimitive=&primitive;d.DrawIndexedPrimitive=&indexedPrimitive;d.BeginScene=&begin;d.SetRenderTarget=&renderTarget;
  IDirect3DViewport3Vtbl v{};v.Clear2=&clear;v.Clear=&oldClear;
  // Run the real installation path, including original-table capture, actual
  // Device3/Viewport3 enumeration, slot guards and replacement function pointers.
  CompatVtable<IDirect3DDevice3Vtbl>::hookVtable<Lock>(d);
  CompatVtable<IDirect3DViewport3Vtbl>::hookVtable<Lock>(v);
  check(d.DrawPrimitive!=&primitive&&d.DrawIndexedPrimitive!=&indexedPrimitive&&v.Clear2!=&clear,"three functional slots installed at INFO/record OFF and diagnostics modes");
  check((d.BeginScene!=&begin)==(mode!=0)&&(v.Clear!=&oldClear)==(mode!=0),"unrelated no-Compat slots retain original diagnostics policy");
  check(d.SetRenderTarget!=&renderTarget,"existing Compat slot remains installed");
  check(d.EndScene==nullptr&&v.SetViewport==nullptr,"null slots are never invented");
  IDirect3DDevice3 device{&d};IDirect3DViewport3 viewport{&v};
  auto c=compatCalls;check(d.SetRenderTarget(&device,nullptr,0)==S_OK&&compatCalls==c+1,"Compat implementation used rather than fallback");
  for(bool value:{false,true}){
   adapt=value;D3DTLVERTEX vertex{};vertex.sx=12;WORD indices[3]{0,0,0};D3DRECT rect{12,20,30,40};
   originalData=&vertex;originalIndices=indices;originalRect=&rect;
   auto a=draws,b=indexed,e=clears,l=locks,ad=assemblyDraw,cd=captainDraw,ac=assemblyClear,cc=captainClear,r=results;
   check(d.DrawPrimitive(&device,D3DPT_TRIANGLELIST,D3DFVF_TLVERTEX,&vertex,1,17)==E_FAIL,"primitive HRESULT preserved");
   check(d.DrawIndexedPrimitive(&device,D3DPT_TRIANGLELIST,D3DFVF_TLVERTEX,&vertex,1,indices,3,19)==S_FALSE,"indexed HRESULT preserved");
   check(v.Clear2(&viewport,1,&rect,D3DCLEAR_TARGET,0x123456,0.5f,7)==E_INVALIDARG,"clear HRESULT preserved");
   check(draws==a+1&&indexed==b+1&&clears==e+1&&locks==l+3,"each original and lock called exactly once");
   check(assemblyDraw==ad+2&&captainDraw==cd+1&&assemblyClear==ac+1&&captainClear==cc+1&&results==r+5,"all functional adapter constructors and result paths run");
   check(vertex.sx==12&&rect.x1==12,"caller-owned vertices and rectangles stay unchanged");
  }
 }
 // At INFO/OFF, a null functional slot stays null and older interfaces are
 // not accidentally caught by signature similarities or method names.
 Qfr::recording=false;Compat::Log::level=Config::Settings::LogLevel::INFO;
 IDirect3DDevice3Vtbl nullDevice{};VtableHookVisitor<IDirect3DDevice3Vtbl,Lock> nv(nullDevice);nv.visit<&IDirect3DDevice3Vtbl::DrawPrimitive>("DrawPrimitive");
 check(nullDevice.DrawPrimitive==nullptr,"null functional route remains null");
 IDirect3DDevice2Vtbl old{};old.DrawPrimitive=&oldDraw;VtableHookVisitor<IDirect3DDevice2Vtbl,Lock> ov(old);ov.visit<&IDirect3DDevice2Vtbl::DrawPrimitive>("DrawPrimitive");
 check(old.DrawPrimitive==&oldDraw,"older Device2 DrawPrimitive remains outside exception");
 std::cout<<"PASS "<<checks<<" actual vtable install/dispatch assertions; INFO/OFF, INFO/ON, DEBUG/OFF; no game/GPU\n";
 }catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}}
