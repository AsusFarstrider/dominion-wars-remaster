namespace {
unsigned backdropCalls=0;
uint32_t __cdecl backdropBlit(QImageResearch* source,QImageResearch* dest,int dx,int dy) {
    using namespace MainBackdrop;QImageResearch s{};check(read(pointer(source),&s,sizeof(s)),"mirror source readable");++backdropCalls;
    const auto clip=value<std::array<int16_t,4>>(value<uint32_t>(0xA39B78));
    auto* out=reinterpret_cast<uint16_t*>(dest->pixels);
    const auto palette=value<uint32_t>(0x4EC390);
    for(unsigned sy=0;sy<s.height;++sy)for(unsigned sx=0;sx<s.width;++sx){
        const int x=dx+s.origin_x+int(sx),y=dy+s.origin_y+int(sy);
        if(x<0||y<0||x>=800||y>=600||((s.flags&1)&&(x<clip[0]||y<clip[1]||x>=clip[0]+clip[2]||y>=clip[1]+clip[3])))continue;
        unsigned char index=0;
        if(s.and_mask!=0||(s.flags&2)){if(!read(s.pixels+sy*s.width+sx,&index,1))throw std::runtime_error("inactive or unreadable source pixel");if((s.flags&2)&&!index)continue;}
        index=uint8_t((index&s.and_mask)^s.xor_value);
        out[y*800+x]=value<uint16_t>(palette+index*2);
    }
    write(pointer(source)+0x16,int16_t{777});write(0xA39B50,uint32_t{999});return 0;
}
void runBackdropCases() {
    using namespace MenuCanvasTest;
    {Fixture f;using namespace MainBackdrop;MainBackdrop::enabled=true;thread=GetCurrentThreadId();pixelReceipt=false;
        artwork=syntheticCard(2560,1440);backdropCalls=0;
        scalar(0x4EC390,0xF0010000);reserve(0xF0010000,512);
        put(0xF0010000,uint16_t{0xF81F});put(0xF0010000+2,uint16_t{0});put(0xF0010000+4,uint16_t{0xFFFF});put(0xF0010000+10,uint16_t{0x001F});
        reserve(0x4E47C0,0x90);put(0x4E47C0+0x50,uint32_t{0x201});
        const std::array<int16_t,4> clip={0,0,1366,768};put(Fixture::outer+0x46,clip);scalar(0xA39B78,0);scalar(0xA39B50,123);
        for(auto address:{0xA39B54u,0xA39B58u,0xA39B5Cu,0xA39B60u,0xA39B68u,0xA39B6Cu,0xA39B70u,0xA39B74u,0xA39B7Cu,0xA39D80u})scalar(address,0);
        constexpr uint32_t source=0xF0011000,pixels=0xF0020000;reserve(source,24);reserve(pixels,800*600);
        auto layer=begin({Fixture::outer,pointer(&f.surface.iface),1366,768},100);
        check(layer&&borrow(layer->target,101)==layer,"outer/Main borrow one owned layer");
        QImageResearch src{};src.unknown_02=255;src.pixel_bits=8;src.and_mask=0;src.width=1366;src.height=768;src.pixels=src.surface=0xDEADBEEF;
        put(source,src);uint32_t args[]={0x4A1774,source,Fixture::outer,0,0};
        check(mirror(*layer,args,reinterpret_cast<void*>(backdropBlit),0x4E47C0)==Mirror::clear&&backdropCalls==0,"native clear excluded without reading inactive fields");
        src={};src.flags=1;src.unknown_02=255;src.and_mask=255;src.pixel_bits=8;src.width=800;src.height=600;src.pixels=pixels;src.pitch=21;
        put(source,src);args[0]=0x46AE63;
        check(mirror(*layer,args,reinterpret_cast<void*>(backdropBlit),0x4E47C0)==Mirror::background,"exact201 background mirrored as reference but excluded coverage");
        check(layer->mask.covered()==0&&backdropCalls==1,"no background pixels flatten into foreground");
        check(value<int16_t>(source+0x16)==21&&value<uint32_t>(0xA39B50)==123&&value<uint32_t>(0xA39B78)==0,"source pitch clip scratch restored");
        src.flags=3;src.width=src.height=2;put(source,src);put(pixels,uint32_t{0x00020100});args[0]=0x46B57E;args[3]=100;args[4]=100;
        check(mirror(*layer,args,reinterpret_cast<void*>(backdropBlit),0x4E4850)==Mirror::foreground&&layer->mask.covered()==2,"key coverage before palette including nonzero black");
        src.flags=0x13;src.width=22;src.height=15;src.pitch=7;put(source,src);
        for(unsigned i=0;i<22*15;++i)put(pixels+i,uint8_t(i%3));args[0]=0x40F984;args[3]=110;args[4]=105;
        check(mirror(*layer,args,reinterpret_cast<void*>(backdropBlit),0x4DB6F8)==Mirror::foreground&&layer->mask.covered()==222,"observed Preferences0x13 preserves keyed22x15 coverage using width stride");
        check(!layer->mask.data()[105*800+110]&&layer->mask.data()[105*800+111]&&layer->pixels[32+105*800+111]==0,"Preferences keyed zero and nonzero palette black remain distinct");
        check(value<int16_t>(source+0x16)==7,"Preferences cached pitch restored after original private conversion");
        src.flags=3;args[0]=0x46B57E;args[4]=100;
        put(0xF0010000+2,uint16_t{0xF800});src.width=src.height=1;put(source,src);put(pixels,uint8_t{1});args[3]=103;
        mirror(*layer,args,reinterpret_cast<void*>(backdropBlit),0x4E4850);
        auto foregroundPlan=MenuCanvas::Plan::fit(800,600,1366,768);check(bool(foregroundPlan),"foreground plan derives actual output");
        std::vector<unsigned char> baseline(f.surface.pixels.size());
        check(artwork->compose({baseline.data(),baseline.size(),Surface::pitch,1366,768}),"independent native background reference");
        check(compose(*layer,*foregroundPlan,{f.surface.pixels.data(),f.surface.pixels.size(),Surface::pitch,1366,768}),"native underlay plus covered original foreground");
        auto sample=[&](int x,int y){const auto p=foregroundPlan->geometry().map_rect({x,y,x+1,y+1});uint16_t v=0;memcpy(&v,f.surface.pixels.data()+p->top*Surface::pitch+p->left*2,2);return v;};
        check(sample(101,100)==0&&sample(103,100)==0xF800,"prior palette conversion retained while later glyph uses new ramp");
        const auto p=foregroundPlan->geometry().map_rect({100,100,101,101});const auto at=p->top*Surface::pitch+p->left*2;
        check(!memcmp(f.surface.pixels.data()+at,baseline.data()+at,2),"transparent source zero reveals detailed native background even if palettezero is colored");
        check(!compose(*layer,*foregroundPlan,{f.surface.pixels.data(),f.surface.pixels.size(),Surface::pitch,1366,768}),"one composition per shared frame");
        auto next=begin(layer->target,102);check(next!=layer&&next->mask.covered()==0,"new frame has fresh coverage/history");
        src.flags=1;src.width=src.height=2;src.and_mask=0;src.xor_value=5;src.pixels=src.surface=0xDEADBEEF;put(source,src);args[3]=200;args[4]=200;
        check(mirror(*next,args,reinterpret_cast<void*>(backdropBlit),0x4DB6F8)==Mirror::foreground&&next->mask.covered()==4,"opaque constant fill does not inspect inactive source fields");
        src.flags=3;src.pixels=pixels;src.surface=0;put(source,src);put(pixels,uint32_t{0x00010000});args[3]=210;
        check(mirror(*next,args,reinterpret_cast<void*>(backdropBlit),0x4DB6F8)==Mirror::foreground&&next->mask.covered()==5,"constant-color glyph still keys on original indices");
        reserve(0xF0012000,8);put(0xF0012000,std::array<int16_t,4>{0,0,1,1});put(0xA39B78,uint32_t{0xF0012000});
        src.flags=0;src.pixels=src.surface=0xDEADBEEF;put(source,src);args[3]=220;
        check(mirror(*next,args,reinterpret_cast<void*>(backdropBlit),0x4DB6F8)==Mirror::foreground&&next->mask.covered()==9,"unclipped bounded fill ignores unrelated global clip in colors and coverage");
        put(0xA39B78,uint32_t{0});src.surface=0;
        src.flags=1;src.and_mask=3;src.xor_value=1;put(source,src);const auto calls=backdropCalls;
        check(mirror(*next,args,reinterpret_cast<void*>(backdropBlit),0x4DB6F8)==Mirror::failed&&backdropCalls==calls,"unproved general mask branch never gets privateRGB565 call");
        boundary();check(!next->good,"resource boundary retires shared layer");
        auto wrongTarget=begin(layer->target,104);args[2]=Fixture::outer+0x100;
        check(mirror(*wrongTarget,args,reinterpret_cast<void*>(backdropBlit),0x4DB6F8)==Mirror::failed&&!wrongTarget->good,"unknown target cannot silently omit foreground coverage");
        MainBackdrop::enabled=false;artwork.reset();current.reset();}
    {MenuCanvas::ForegroundMask mask(4,3);const uint8_t indices[]={0,1,2,0};
        check(mask.mark(indices,4,2,2,-1,1,true,false,{0,0,4,3})&&mask.covered()==1,"negative clipped coverage stays within buffer");
        check(!mask.mark(indices,3,2,2,0,0,true,false,{0,0,4,3}),"short source mask rejected");}
    {HdFixture f;put(hdHandle+8,uint32_t{601});put(hdHandle+0x14,uint32_t{2997});put(hdHandle+0x18,uint32_t{100});
        scalar(0x749020+4*0x5C6,42);put(0x74A9F0,uint32_t{42});f.open();
        check(movies[hdHandle]->profile==&hdProfiles[1],"Gizmo own601/2997:100 profile staged");
        check(copy(fakeCopy,0x40F2ED,reinterpret_cast<void*>(hdHandle),nativePixels,HdFixture::stride,1440,0,0,10)==0&&codecCalls==1,"Gizmo uses originalXY0 call plus private sourceXY0 exactly once");}
    {HdFixture f;put(hdHandle+8,uint32_t{601});put(hdHandle+0x14,uint32_t{2997});put(hdHandle+0x18,uint32_t{100});scalar(0x749020+4*0x5C6,42);f.open();
        check(copy(fakeCopy,0x40F2ED,reinterpret_cast<void*>(hdHandle),nativePixels,HdFixture::stride,1440,0,0,10)==-1&&!codecCalls,"Gizmo header cannot borrow Paramount resource binding");}
}
}
