namespace {
void runNativeButtonCases(){using namespace MenuCanvasTest;using namespace MainBackdrop;
 Fixture f;MainBackdrop::enabled=true;MainBackdrop::thread=GetCurrentThreadId();artwork=syntheticCard(2560,1440);
 Remaster::MainButtons::available=true;Remaster::MainButtons::thread=GetCurrentThreadId();Remaster::MainButtons::foreign=false;
 auto framePixels=std::make_shared<std::vector<uint8_t>>(720*400*4);
 for(unsigned y=0;y<400;++y)for(unsigned x=4;x<720;++x){const auto at=(size_t(y)*720+x)*4;(*framePixels)[at+1]=255;(*framePixels)[at+3]=255;}
 auto bank=std::make_shared<Remaster::ButtonBank>();bank->idle=0x202;for(auto& frame:bank->frames)frame={framePixels,0,720,400};Remaster::MainButtons::banks[0]=bank;
 scalar(0x4EC390,0xF0010000);reserve(0xF0010000,512);put(0xF0010000+2,uint16_t{0xF800});reserve(0x4E47C0,0x90);put(0x4E47C0+0x50,uint32_t{0x201});
 put(Fixture::outer+0x46,std::array<int16_t,4>{0,0,1366,768});scalar(0xA39B78,0);
 for(auto address:{0xA39B50u,0xA39B54u,0xA39B58u,0xA39B5Cu,0xA39B60u,0xA39B68u,0xA39B6Cu,0xA39B70u,0xA39B74u,0xA39B7Cu,0xA39D80u})scalar(address,0);
 constexpr uint32_t source=0xF0011000,pixels=0xF0020000;reserve(source,24);reserve(pixels,800*600);std::vector<uint8_t> indices(800*600,1);MenuCanvasTest::write(pixels,indices.data(),indices.size());
 auto paintPlan=MenuCanvas::Plan::fit(800,600,1366,768);check(bool(paintPlan),"native button controller uses shared output paintPlan");
 auto drawFrame=[&](bool modal){
  auto layer=begin({Fixture::outer,pointer(&f.surface.iface),1366,768},100);check(layer&&layer->ordered,"actual shared layer selects ordered native button capture");
  QImageResearch src{};src.unknown_02=255;src.pixel_bits=8;src.width=1366;src.height=768;put(source,src);uint32_t args[]={0x4A1774,source,Fixture::outer,0,0};
  check(mirror(*layer,args,reinterpret_cast<void*>(backdropBlit),0x4E47C0)==Mirror::clear,"native button frame preserves witnessed native clear");
  src.flags=1;src.and_mask=255;src.width=800;src.height=600;src.pixels=pixels;put(source,src);args[0]=0x46AE63;
  check(mirror(*layer,args,reinterpret_cast<void*>(backdropBlit),0x4E47C0)==Mirror::background,"original indexed background still executes privately");
  src.flags=3;src.width=180;src.height=100;put(source,src);args[0]=0x46B57E;args[3]=320;args[4]=50;
  Remaster::MainButtons::witness(layer->scope,0x4E4850,0x21C,source);
  check(mirror(*layer,args,reinterpret_cast<void*>(backdropBlit),0x4E4850)==Mirror::foreground,"exact lookup/source/caller selects native activation sprite");
  if(modal){put(0xF0010000+2,uint16_t{0x001F});src.flags=0x13;src.width=src.height=1;put(source,src);args[0]=0x40F984;args[3]=322;args[4]=52;
   check(mirror(*layer,args,reinterpret_cast<void*>(backdropBlit),0x4DB6F8)==Mirror::foreground,"actual modal glyph joins after native sprite");}
  return layer;
 };
 std::vector<uint8_t> reference(f.surface.pixels.size());check(artwork->compose({reference.data(),reference.size(),Surface::pitch,1366,768}),"native background reference");
 auto first=drawFrame(true);check(compose(*first,*paintPlan,{f.surface.pixels.data(),f.surface.pixels.size(),Surface::pitch,1366,768}),"actual mixed shared controller commits native source-density frame");
 const auto bounds=paintPlan->geometry().map_rect({320,50,500,150});const auto holeAt=size_t(bounds->top)*Surface::pitch+size_t(bounds->left)*2;
 check(!memcmp(f.surface.pixels.data()+holeAt,reference.data()+holeAt,2),"old paletted button pixels do not survive under replacement alpha holes");
 uint16_t pixelColor=0;memcpy(&pixelColor,f.surface.pixels.data()+size_t(bounds->top+50)*Surface::pitch+size_t(bounds->left+100)*2,2);check(pixelColor==0x07E0,"replacement RGBA uses its source color independently of original palette");
 const auto modal=paintPlan->geometry().map_rect({322,52,323,53});memcpy(&pixelColor,f.surface.pixels.data()+size_t(modal->top)*Surface::pitch+size_t(modal->left)*2,2);check(pixelColor==0x001F,"modal stays over HD sprite in real shared capture path");
 artwork.reset();put(0xF0010000+2,uint16_t{0xF800});auto second=drawFrame(false);
 check(compose(*second,*paintPlan,{f.surface.pixels.data(),f.surface.pixels.size(),Surface::pitch,1366,768}),"native buttons work with original main background");
 memcpy(&pixelColor,f.surface.pixels.data()+holeAt,2);check(pixelColor==0xF800,"replacement hole exposes freshly captured original background when native background absent");
 Remaster::MainButtons::available=false;Remaster::MainButtons::banks={};Remaster::MainButtons::pending={};MainBackdrop::enabled=false;MainBackdrop::current.reset();
}
}
