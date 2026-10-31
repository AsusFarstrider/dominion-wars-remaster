namespace {
std::shared_ptr<CardImage> syntheticCard(unsigned w,unsigned h) {
    std::vector<unsigned char> bytes(16+size_t(w)*h*2);
    memcpy(bytes.data(),"DWCARD1\0",8);memcpy(bytes.data()+8,&w,4);memcpy(bytes.data()+12,&h,4);
    for(size_t i=0;i<size_t(w)*h;++i){const uint16_t color=uint16_t((i*53)^0x1234);memcpy(bytes.data()+16+i*2,&color,2);}
    return CardImage::parse(bytes);
}
struct CardFixture:Fixture {
    CardFixture(unsigned state=5) {
        using namespace MenuCanvasTest;spec=state==5?&specs[1]:&specs[2];
        scalar(0x4D0A24,state);scalar(0x4D0A28,state);reserve(spec->root,0x90);
        put(spec->root,spec->id);put(spec->root+12,uint16_t{800});put(spec->root+14,uint16_t{600});
        put(spec->root+0x48,spec->input);put(spec->root+0x4C,spec->draw);put(spec->root+0x77,uint32_t{0x5F7ED8});
        put(spec->root+0x50,uint32_t{state==5?0x101u:0x103u});put(0x5F7ED8+0x73,spec->root);
        frame[1]=spec->root;pointNative(10,10);
        cardsEnabled=true;for(auto& c:cards){c.image=syntheticCard(2560,1440);c.pixelReceipt=false;}
    }
    ~CardFixture(){cardsEnabled=false;for(auto& c:cards)c.image.reset();}
    void pointNative(int x,int y){const int z=0;memcpy(stack.data()+0x14,&x,4);memcpy(stack.data()+0x10,&y,4);
        memcpy(stack.data()+0x34,&z,4);memcpy(stack.data()+0x38,&z,4);}
};
void runCardCases() {
    {const auto image=syntheticCard(2560,1440);check(bool(image)&&image->width()==2560&&image->height()==1440,"card keeps source dimensions");
        constexpr size_t pitch=5128;std::vector<unsigned char> out(pitch*1440,0x55);
        check(image->compose({out.data(),out.size(),pitch,2560,1440}),"card identity uses native row pitch");
        bool equal=true,padding=true;for(unsigned y=0;y<1440;++y){equal=equal&&!memcmp(out.data()+y*pitch,static_cast<const unsigned char*>(image->data())+y*5120,5120);
            for(size_t x=5120;x<pitch;++x)padding=padding&&out[y*pitch+x]==0x55;}
        check(equal&&padding,"all source pixels retained; target padding untouched");
        check(!image->compose({out.data(),out.size()-20,pitch,2560,1440}),"short native allocation rejected");
        check(!image->compose({const_cast<void*>(image->data()),image->bytes(),5120,2560,1440}),"source/native alias rejected");}
    {auto image=syntheticCard(1672,941);for(auto wh:{std::pair{1024u,768u},std::pair{1366u,768u},std::pair{1920u,1200u},std::pair{3840u,2160u}}){
        std::vector<unsigned char> dst(size_t(wh.first)*wh.second*2);check(image->compose({dst.data(),dst.size(),wh.first*2,wh.first,wh.second}),"runtime source/output fit supports simulated dimensions");}}
    for(unsigned state:{5u,6u}) {
        CardFixture f(state);f.draw();check(uiPublication.cardResource==(state==5?0x101u:0x103u)&&uiPublication.interactive,"correct resource/root publishes native card");
        for(int x:{0,30,1335,1365}){f.pointNative(x,400);check(f.route()==0x41DADC,"card click native sidebands admitted");}
        f.pointNative(-1,400);check(f.route()==0x41DC62,"card outside target misses");
        MenuCanvasTest::put(0x4D0A28,state+1);f.draw();check(frames==2&&uiPublication.cardResource&&!uiPublication.interactive,"known departing card still replaced but input retired");}
    {CardFixture f;cardsEnabled=false;f.draw();check(!uiPublication.cardResource&&uiPublication.interactive,"no override retains original completed-screen path");}
    {CardFixture f;cards[0].image.reset();f.draw();check(!uiPublication.cardResource&&uiPublication.interactive,"missing supported asset safely falls back");}
    {CardFixture f;MenuCanvasTest::put(f.spec->root+0x50,uint32_t{0x999});f.draw();check(!uiPublication.cardResource,"same dimensions cannot identify different resource");}
    {CardFixture f;f.surface.unlockResult=DDERR_SURFACELOST;f.draw();check(!uiPublication.interactive,"failed card Unlock never publishes input");}
    {CardFixture f;Target t{pointer(&f.surface.iface),Surface::w,Surface::h};check(planFor(t),"prior menu plan exists before native card");
        f.draw();MenuCanvasTest::put(f.spec->root+0x50,uint32_t{0x103});check(f.route()==0x41DC62,"resource change cannot use stale native input publication");}
    {CardFixture f;f.draw();resourceBoundary("card_reset_fixture");check(!uiPublication.interactive,"reset revokes card publication");
        f.draw();check(uiPublication.interactive&&uiPublication.cardResource==0x101,"fresh output repairs card publication");}
    for(int mode:{0,1,2,3}) {
        std::vector<unsigned char> bytes(24);memcpy(bytes.data(),"DWCARD1\0",8);uint32_t w=2,h=2;memcpy(bytes.data()+8,&w,4);memcpy(bytes.data()+12,&h,4);
        if(mode==0)bytes[0]='X';if(mode==1)bytes.pop_back();if(mode==2){w=0;memcpy(bytes.data()+8,&w,4);}if(mode==3){w=0xFFFFFFFF;memcpy(bytes.data()+8,&w,4);}
        check(!CardImage::parse(bytes),"malformed/truncated/overflow card safely rejected");
    }
}
}
