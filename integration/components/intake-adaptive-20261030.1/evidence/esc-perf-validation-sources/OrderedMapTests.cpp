#include <Remaster/OrderedPaint.h>
#include "ReferenceOrderedPaint.h"
#include <cstdlib>
#include <iostream>
static int failAllocationAfter=-1;
void* operator new(size_t n){if(failAllocationAfter==0)throw std::bad_alloc();if(failAllocationAfter>0)--failAllocationAfter;if(void* p=std::malloc(n?n:1))return p;throw std::bad_alloc();}
void* operator new[](size_t n){return ::operator new(n);}
void operator delete(void* p) noexcept{std::free(p);} void operator delete(void* p,size_t) noexcept{std::free(p);}
void operator delete[](void* p) noexcept{std::free(p);} void operator delete[](void* p,size_t) noexcept{std::free(p);}
namespace {unsigned checks=0;
void check(bool v,const char* why){++checks;if(!v)throw std::runtime_error(why);}
struct Output{unsigned w,h,pitch;std::vector<uint8_t> bytes;
 Output(unsigned x,unsigned y):w(x),h(y),pitch(x*2+8),bytes(size_t(pitch)*y,0xA5){}
 MenuCanvas::WriteView view(){return {bytes.data(),bytes.size(),pitch,w,h};}};
Remaster::SpriteFrame sprite(){auto p=std::make_shared<std::vector<uint8_t>>(size_t(7*9*4));for(size_t i=0;i<p->size();++i)(*p)[i]=uint8_t((i*31+77)%256);return {p,0,7,9};}
template<class Paint> void populate(Paint& p,const std::vector<uint16_t>& logical,const std::vector<uint8_t>& indices,Remaster::SpriteFrame frame){
 check(p.raw(logical.data(),indices.data(),400*450,400,450,200,75,true,false,{0,0,800,600}),"keyed Preferences-sized raw operation admitted");
 check(p.sprite(frame,{399,80,409,96},{400,81,408,95}),"interleaved clipped alpha sprite admitted");
 check(p.raw(logical.data(),indices.data(),200*12,200,12,55,581,false,false,{60,580,250,590}),"later clipped raw glyph row admitted");
 check(p.raw(logical.data(),nullptr,0,3,11,201,90,false,true,{202,92,203,94}),"tiny solid fractional/collapsed cell admitted");
 check(p.raw(logical.data(),indices.data(),400*450,400,450,200,75,true,false,{222,110,556,444}),"later modal coverage preserves operation order");}
}
int main(){try{
 std::vector<uint16_t> logical(800*600);std::vector<uint8_t> indices(400*450);
 for(size_t i=0;i<logical.size();++i)logical[i]=uint16_t((i*2137)%65536);
 for(size_t i=0;i<indices.size();++i)indices[i]=uint8_t(i%7?1:0);
 const auto frame=sprite();
 for(auto dims:std::array<std::array<unsigned,2>,12>{{{1,1},{3,2},{320,240},{800,600},{801,601},{1280,800},{1366,768},{1920,1080},{1920,1200},{2560,1440},{3440,1440},{3840,2160}}}){
  auto plan=dw::ui::Canvas::fit({0,0,800,600},{0,0,dims[0],dims[1]});check(bool(plan),"simulated dimension plan valid");
  Remaster::OrderedPaint actual;Remaster::ReferenceOrderedPaint original;populate(actual,logical,indices,frame);populate(original,logical,indices,frame);
  Output a(dims[0],dims[1]),b(dims[0],dims[1]);size_t aw=0,bw=0;
  check(actual.compose(*plan,a.view(),&aw)&&original.compose(*plan,b.view(),&bw),"both actual composition implementations succeed");
  check(aw==bw&&a.bytes==b.bytes,"whole output including margins and row padding is byte-identical to original per-pixel search");
 }
 auto plan=dw::ui::Canvas::fit({0,0,800,600},{0,0,1366,768});Remaster::OrderedPaint raw;raw.raw(logical.data(),indices.data(),400*450,400,450,200,75,true,false,{0,0,800,600});
 for(int failure:{0,1}){Output a(1366,768);const auto saved=a.bytes;failAllocationAfter=failure;const auto result=raw.compose(*plan,a.view());failAllocationAfter=-1;
  check(!result&&a.bytes==saved,"each failed map allocation returns before destination writes");}
 {Output a(1366,768);Remaster::OrderedPaint onlySprite;onlySprite.sprite(frame,{10,10,17,19},{0,0,800,600});failAllocationAfter=0;const auto result=onlySprite.compose(*plan,a.view());const auto untouched=failAllocationAfter;failAllocationAfter=-1;
  check(result&&untouched==0,"sprite-only composition does not allocate inverse maps");}
 std::cout<<"PASS "<<checks<<" ordered inverse-map differential/clip/alpha/rounding/padding/allocation assertions; simulated dimensions only\n";
 }catch(const std::exception& e){failAllocationAfter=-1;std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}}
