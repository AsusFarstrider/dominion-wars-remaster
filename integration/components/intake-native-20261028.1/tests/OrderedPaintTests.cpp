#include <Remaster/OrderedPaint.h>
#include <Remaster/MainButtons.h>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <thread>
namespace {unsigned checks=0;
void check(bool v,const char* why){++checks;if(!v)throw std::runtime_error(why);}
Remaster::SpriteFrame image(unsigned w,unsigned h,std::array<uint8_t,4> color){auto b=std::make_shared<std::vector<uint8_t>>(size_t(w)*h*4);for(size_t i=0;i<b->size();i+=4)memcpy(b->data()+i,color.data(),4);return {b,0,w,h};}
struct Output {unsigned w,h,pitch;std::vector<uint8_t> bytes;
 Output(unsigned width,unsigned height,uint16_t c=0x001F):w(width),h(height),pitch(width*2+8),bytes(size_t(pitch)*height,0xA5){for(unsigned y=0;y<h;++y)for(unsigned x=0;x<w;++x)memcpy(bytes.data()+size_t(y)*pitch+x*2,&c,2);}
 MenuCanvas::WriteView view(){return {bytes.data(),bytes.size(),pitch,w,h};}
 uint16_t pixel(unsigned x,unsigned y){uint16_t v=0;memcpy(&v,bytes.data()+size_t(y)*pitch+x*2,2);return v;}
 bool padding(){for(unsigned y=0;y<h;++y)for(unsigned x=w*2;x<pitch;++x)if(bytes[size_t(y)*pitch+x]!=0xA5)return false;return true;}
};
void tests(){using Remaster::OrderedPaint;
 auto plan=dw::ui::Canvas::fit({0,0,800,600},{0,0,800,600});check(bool(plan),"original logical plan");
 auto red=image(4,4,{255,0,0,255});auto holes=std::make_shared<std::vector<uint8_t>>(*red.storage);std::fill_n(holes->data(),4,uint8_t{0});red.storage=holes;
 {OrderedPaint q;Output out(800,600);check(q.sprite(red,{10,10,14,14},{0,0,800,600}),"explicit RGBA sprite accepted");check(q.compose(*plan,out.view()),"native RGBA composition");
  check(out.pixel(10,10)==0x001F&&out.pixel(11,10)==0xF800,"new alpha holes reveal background, opaque pixels replace it");check(out.padding(),"native target row padding untouched");}
 {OrderedPaint q;Output out(800,600);auto half=image(1,1,{255,0,0,128});q.sprite(half,{20,20,21,21},{0,0,800,600});q.compose(*plan,out.view());check(out.pixel(20,20)==0x800F,"partial alpha blends in RGB before final565 quantization");}
 {OrderedPaint q;Output out(800,600);std::vector<uint16_t> logical(800*600,0x07E0);uint8_t index=1;
  q.sprite(image(1,1,{255,0,0,255}),{20,20,21,21},{0,0,800,600});q.raw(logical.data(),&index,1,1,1,20,20,true,false,{0,0,800,600});q.compose(*plan,out.view());
  check(out.pixel(20,20)==0x07E0,"later modal operation covers earlier HD sprite in original order");}
 {OrderedPaint q;Output out(800,600);std::vector<uint16_t> logical(800*600,0);const uint8_t index[]={0,1};
  check(q.raw(logical.data(),index,2,2,1,10,10,true,false,{0,0,800,600}),"keyed raw foreground copied with owned colors/coverage");logical.assign(800*600,0xFFFF);
  q.compose(*plan,out.view());check(out.pixel(10,10)==0x001F&&out.pixel(11,10)==0,"raw source zero differs from nonzero palette-black; colors owned immediately");}
 {OrderedPaint q;Output out(800,600);q.sprite(image(4,4,{255,0,0,255}),{30,30,34,34},{31,31,33,33});q.compose(*plan,out.view());
  check(out.pixel(30,30)==0x001F&&out.pixel(31,31)==0xF800&&out.pixel(33,33)==0x001F,"logical clip affects drawing without changing source fit or hitboxes");}
 for(const auto dims:std::array<std::array<unsigned,2>,3>{{{1366,768},{2560,1440},{1920,1200}}}){Output out(dims[0],dims[1]);auto fit=dw::ui::Canvas::fit({0,0,800,600},{0,0,dims[0],dims[1]});OrderedPaint q;
  check(q.sprite(image(720,400,{0,255,0,255}),{320,50,500,150},{0,0,800,600})&&q.compose(*fit,out.view()),"source-density sprite derives physical placement from output dimensions");
  const auto r=fit->map_rect({320,50,500,150});check(r&&out.pixel(unsigned(r->left),unsigned(r->top))==0x07E0&&out.padding(),"transformed sprite edge and padding remain exact");}
 {Output out(2560,1440,0);auto fit=dw::ui::Canvas::fit({0,0,800,600},{0,0,2560,1440});auto fine=image(720,400,{0,0,0,255});auto p=std::make_shared<std::vector<uint8_t>>(*fine.storage);
  for(unsigned y=0;y<400;++y)for(unsigned x=0;x<720;++x)(*p)[(size_t(y)*720+x)*4]=uint8_t((x&1)?255:0);fine.storage=p;OrderedPaint q;q.sprite(fine,{320,50,500,150},{0,0,800,600});q.compose(*fit,out.view());
  const auto r=fit->map_rect({320,50,500,150});unsigned changes=0;uint16_t last=0;for(auto x=r->left;x<r->right;++x){auto c=out.pixel(unsigned(x),unsigned(r->top));if(c!=last)++changes;last=c;}
  check(changes>180,"additional high-density source stripes survive beyond180 logical columns");}
 {OrderedPaint q;check(!q.sprite(red,{-1,0,3,4},{0,0,800,600}),"unsupported outside-canvas sprite is refused");
  std::vector<uint16_t> logical(800*600);uint8_t v=1;check(!q.raw(logical.data(),&v,1,2,1,0,0,true,false,{0,0,800,600}),"short source coverage cannot read past bounds");}
 {using namespace Remaster::MainButtons;available=true;thread=GetCurrentThreadId();foreign=false;
  auto bank=std::make_shared<Remaster::ButtonBank>();bank->idle=table[0].idle;for(auto& f:bank->frames)f=image(2,2,{255,0,0,255});banks[0]=bank;
  witness(1,table[0].node,table[0].idle+34,0xF0010000);check(bool(consume(1,table[0].node,0xF0010000,0x46B57E)),"actual final activation resource is selected despite callback flag changes");
  check(!consume(1,table[0].node,0xF0010000,0x46B57E),"lookup receipt is single-use");
  witness(2,table[0].node,table[0].idle+1,0xF0010000);check(!consume(3,table[0].node,0xF0010000,0x46B57E),"old-frame resource receipt cannot survive surface/pass reuse");
  witness(4,table[0].node,table[0].idle,0xF0010000);std::thread t([&]{witness(4,table[0].node,table[0].idle,0xF0010000);});t.join();check(!consume(4,table[0].node,0xF0010000,0x46B57E),"foreign lookup invalidates admission without mutating owned receipt");
  available=false;banks={};pending={};}
}
}
namespace Qfr {void event(const char*,const std::string&)noexcept{}}
namespace Remaster {void note(const char*,const std::string&)noexcept{}}
int main(int argc,char** argv){try{tests();
 if(argc==2){const auto folder=std::filesystem::path(argv[1]);for(const auto& b:Remaster::MainButtons::table){std::ifstream f(folder/b.file,std::ios::binary);std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(f)),{});auto parsed=Remaster::ButtonBank::parse(std::move(bytes),b.idle);check(bool(parsed),"actual converted bank has exact dimensions/resource IDs/length");for(const auto& frame:parsed->frames)check(frame.valid(),"actual converted RGBA frame bounded");}}
 std::cout<<"PASS "<<checks<<" ordered source-density/alpha/resource assertions; no game or hardware proof\n";
 }catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}}
