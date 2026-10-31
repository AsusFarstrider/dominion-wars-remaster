// Synthetic CPU-only composition benchmark; not measured gameplay frame time.
#include <Remaster/OrderedPaint.h>
#include <chrono>
#include <iostream>
int main(){
 auto plan=dw::ui::Canvas::fit({0,0,800,600},{0,0,2560,1440});
 std::vector<uint16_t> logical(800*600,0x39E7);std::vector<uint8_t> index(400*450,1);
 std::vector<uint8_t> output(2560*1440*2,0);MenuCanvas::WriteView dst{output.data(),output.size(),5120,2560,1440};
 Remaster::OrderedPaint raw;raw.raw(logical.data(),index.data(),index.size(),400,450,200,75,false,false,{0,0,800,600});
 auto storage=std::make_shared<std::vector<uint8_t>>(size_t(720*400*4),uint8_t{255});Remaster::SpriteFrame frame{storage,0,720,400};
 Remaster::OrderedPaint sprites;for(int y:{50,115,185,255,325,395})sprites.sprite(frame,{320,y,500,y+100},{0,0,800,600});
 auto measure=[&](const auto& q){
  q.compose(*plan,dst);const auto begin=std::chrono::steady_clock::now();
  for(unsigned i=0;i<4;++i)if(!q.compose(*plan,dst))return -1.0;
  return std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-begin).count()/4;
 };
 const auto panel=measure(raw),buttons=measure(sprites);
 std::cout<<"{\"synthetic_panel_ms\":"<<panel<<",\"synthetic_six_sprites_ms\":"<<buttons<<",\"gameplay_timing\":false}\n";
 return panel<0||buttons<0;
}
