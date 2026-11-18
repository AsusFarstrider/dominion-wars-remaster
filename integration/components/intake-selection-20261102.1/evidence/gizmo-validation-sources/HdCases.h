// Included after the retained fixture; exercises the actual HD provider.
namespace {
constexpr uint32_t hdHandle=0xF0006000;
unsigned codecCalls=0,closeCalls=0,gotoCalls=0,observedFlags=0;
int codecResult=0,inject=0;
void* nativePixels=nullptr;
void* __stdcall fakeOpen(const void*,unsigned){return reinterpret_cast<void*>(hdHandle);}
void __stdcall fakeClose(void*){++closeCalls;}
void __stdcall fakeGoto(void*,unsigned,unsigned){++gotoCalls;}
int __stdcall fakeCopy(void* handle,void* out,int pitch,unsigned h,unsigned x,unsigned y,unsigned flags) {
    ++codecCalls;observedFlags=flags;
    check(handle==reinterpret_cast<void*>(hdHandle)&&out!=nativePixels,"decoder never sees native pointer");
    check(pitch==5120&&h==1440&&!x&&!y,"source-sized copy pitch height XY");
    if(codecResult==0) {
        auto* p=static_cast<uint16_t*>(out);
        if(flags&0x80000000)std::fill_n(p,2560*1440,uint16_t{0x07E0});
        else p[0]=0xF800; // Only one changed pixel; all other history must survive.
        p[2560*1440-1]=0x001F;
    }
    if(inject==1)static_cast<uint16_t*>(out)[2560*1440]=0;
    if(inject==2)resourceBoundary("fixture_reset_during_copy");
    if(inject==3)throw std::runtime_error("fixture codec exception");
    if(inject==4)binkClose(handle);
    if(inject==5)binkGoto(handle,1,0);
    SetLastError(343);return codecResult;
}
struct HdFixture:Fixture {
    static constexpr unsigned width=2560,height=1440,stride=5128;
    HdFixture() {
        using namespace MenuCanvasTest;
        hdRequested=safetyInstalled=true;codecCalls=closeCalls=gotoCalls=observedFlags=0;codecResult=inject=0;
        originalOpen=fakeOpen;originalClose=fakeClose;originalGoto=fakeGoto;
        spec=&specs[0];put(0x4D0A24,uint32_t{4});put(0x4D0A28,uint32_t{4});
        reserve(spec->root,0x90);put(spec->root,spec->id);put(spec->root+12,uint16_t{800});put(spec->root+14,uint16_t{600});
        put(spec->root+0x48,spec->input);put(spec->root+0x4C,spec->draw);put(spec->root+0x77,uint32_t{0x5F7ED8});put(0x5F7ED8+0x73,spec->root);
        QImageResearch image{};read(native,image);image.width=width;image.height=height;put(native,image);
        surface.pixels.assign(stride*height,0x33);nativePixels=surface.pixels.data();
        reserve(hdHandle,0x200);put(hdHandle,width);put(hdHandle+4,height);put(hdHandle+8,uint32_t{225});
        put(hdHandle+12,uint32_t{1});put(hdHandle+0x14,uint32_t{15});put(hdHandle+0x18,uint32_t{1});
        put(hdHandle+0x1E4,width);put(hdHandle+0x1E8,height);
        reserve(0x74A9F0,4*0x24);put(0x74AA08,uint32_t{2});put(0x74AA0C,hdHandle);put(0x74A9F0,uint32_t{37});
        scalar(0x749020+4*0x5C5,37);scalar(0x4E47B8,0);scalar(0x5DE6A8,1);scalar(0x5DE69C,10);
        scalar(0x5DE6A0,80);scalar(0x5DE6A4,60);
        arm();frame[1]=spec->root;physicalPoint(100,100);
        check(context()==spec,"HD exact original startup root");
    }
    ~HdFixture(){hdRequested=safetyInstalled=false;movies.clear();lease={};}
    void arm(){lease={true,false,pointer(&surface.iface),0,epoch,++locks,{}};
        lease.d.dwSize=sizeof(lease.d);lease.d.dwWidth=width;lease.d.dwHeight=height;lease.d.lPitch=stride;lease.d.lpSurface=nativePixels;
        lease.d.ddpfPixelFormat.dwFlags=DDPF_RGB;lease.d.ddpfPixelFormat.dwRGBBitCount=16;
        lease.d.ddpfPixelFormat.dwRBitMask=0xF800;lease.d.ddpfPixelFormat.dwGBitMask=0x7E0;lease.d.ddpfPixelFormat.dwBBitMask=0x1F;}
    void physicalPoint(int x,int y){int32_t z=0;std::memcpy(stack.data()+0x14,&x,4);std::memcpy(stack.data()+0x10,&y,4);
        std::memcpy(stack.data()+0x34,&z,4);std::memcpy(stack.data()+0x38,&z,4);}
    void open(){check(binkOpen(nullptr,0)==reinterpret_cast<void*>(hdHandle),"profile staged before handle escapes");}
    int drawHd(){return copy(fakeCopy,0x40F2ED,reinterpret_cast<void*>(hdHandle),nativePixels,stride,height,80,60,10);}
    void unlock(HRESULT hr=DD_OK){uintptr_t args[]={0};apiResult("IDirectDrawSurface4Vtbl::Unlock",pointer(&surface.iface),0x4A7B4E,args,1,hr);}
    uint16_t pixel(size_t offset=0){uint16_t v=0;std::memcpy(&v,surface.pixels.data()+offset,2);return v;}
};
void runHdCases() {
    {HdFixture f;f.open();codecResult=1;check(f.drawHd()==1&&codecCalls==1,"initial late copy forwarded once");
        check(observedFlags==0x8000000A&&!movies[hdHandle]->hd->initialized,"skipped conversion still needs full initialization");
        check(!lease.written&&f.pixel()==0x3333&&!moviePublication.generation,"skip cannot publish invented history");
        codecResult=0;check(f.drawHd()==0&&codecCalls==2,"next actual conversion once");
        check(observedFlags==0x8000000A&&lease.written,"first real conversion still full");
        check(f.pixel()==0x07E0&&f.pixel((1439*HdFixture::stride)+2559*2)==0x001F,"first and last source pixels survive native identity");
        check(f.pixel(5120)==0x3333,"native pitch padding untouched");check(!moviePublication.interactive,"Copy alone not published");
        f.unlock();check(moviePublication.interactive,"matching Unlock publishes");
        for(int x:{0,100,319,2240,2559}){f.physicalPoint(x,700);check(f.route()==0x41DADC,"HD sidebands use full native skip root");}
        f.physicalPoint(2560,700);check(f.route()==0x41DC62,"outside client misses");
        f.arm();check(f.drawHd()==0&&observedFlags==10,"later copy selective");
        check(f.pixel()==0xF800&&f.pixel(2)==0x07E0,"selective update retains previous pixels");
        codecResult=1;check(f.drawHd()==1&&lease.written&&f.pixel(2)==0x07E0,"later late-copy repeats valid source history");
        check(GetLastError()==343,"codec LastError forwarded");f.unlock();
        binkGoto(reinterpret_cast<void*>(hdHandle),1,0);check(gotoCalls==1&&!movies[hdHandle]->hd->initialized&&!moviePublication.generation,"seek resets source validity and output publication");
        f.arm();codecResult=0;f.drawHd();check(observedFlags==0x8000000A,"seek forces fresh full conversion");
        binkClose(reinterpret_cast<void*>(hdHandle));check(closeCalls==1&&!movies.count(hdHandle)&&!moviePublication.generation,"close retires lifetime once");
        const auto life=lifetimes;f.open();check(lifetimes>life&&!movies[hdHandle]->hd->initialized,"reused handle has fresh lifetime/history");}
    {HdFixture f;f.open();f.drawHd();resourceBoundary("fixture_output_loss");check(movies[hdHandle]->hd->initialized,"output reset retains source history");
        const auto before=f.pixel();f.drawHd();check(codecCalls==2&&!lease.written&&f.pixel()==before,"missing lease decodes privately and withholds writes");
        f.arm();f.drawHd();check(observedFlags==10&&lease.written,"new output resumes from persistent source history");}
    {HdFixture f;f.open();inject=2;f.drawHd();check(codecCalls==1&&!moviePublication.generation,"reset during copy cannot publish old epoch");}
    {HdFixture f;f.open();inject=4;f.drawHd();check(codecCalls==1&&closeCalls==1&&!lease.written,"close during copy retires map but owned history stays alive");}
    {HdFixture f;f.open();inject=5;f.drawHd();check(codecCalls==1&&!lease.written,"seek during copy cannot publish pre-seek frame");}
    for(int mode:{0,1,2,3,4,5,6,7,8}){HdFixture f;f.open();using namespace MenuCanvasTest;
        if(mode==0)ownerThread=0;
        if(mode==1)enabled=false;
        if(mode==2)copyingVideo=true;
        if(mode==3)put(hdHandle+0x1E4,uint32_t{1280});
        if(mode==4)put(hdHandle+0x20,uint32_t{0x80000});
        if(mode==5)failedRead=hdHandle;
        if(mode==6)put(hdHandle,uint32_t{640});
        if(mode==7)safetyInstalled=false;
        if(mode==8)put(0x74A9F0,uint32_t{38});
        check(f.drawHd()==-1&&codecCalls==0&&f.pixel()==0x3333,"unsafe HD cannot reach native legacy fallback");}
    for(int mode:{1,3}){HdFixture f;f.open();inject=mode;check(f.drawHd()==(mode==1?0:-1)&&codecCalls==1,"bad guard/exception never makes second codec call");
        check(movies[hdHandle]->hd->quarantined&&!lease.written,"failed source quarantined without native writes");}
    {HdFixture f;f.open();codecResult=2;check(f.drawHd()==2&&codecCalls==1&&!lease.written,"unexpected return forwarded once without publication");}
    {HdFixture f;f.open();f.drawHd();f.unlock(DDERR_SURFACELOST);check(!moviePublication.interactive,"failed Unlock refuses input publication");}
    for(int mode:{0,1,2,3,4}){HdFixture f;using namespace MenuCanvasTest;
        if(mode==0)enabled=false;
        if(mode==1)put(hdHandle+8,uint32_t{226});
        if(mode==2)ownerThread=0;
        if(mode==3)failedRead=hdHandle;
        if(mode==4)failAllocation=true;
        check(!binkOpen(nullptr,0)&&closeCalls==1&&!movies.count(hdHandle),"Open rejection closes once and does not expose unsafe handle");}
    {bool threw=false;try{HdHistory invalid(0xFFFFFFFF,1440);}catch(...){threw=true;}check(threw,"overflow/invalid allocation rejected before resize");}
    {HdFixture f;f.open();f.drawHd();f.unlock();const auto failures=errors;int result=0;
        std::thread worker([&]{result=f.drawHd();});worker.join();
        check(result==-1&&codecCalls==1,"actual foreign thread refuses without entering codec");
        check(errors==failures&&moviePublication.interactive,"foreign refusal leaves owner state and diagnostics untouched");}
}
}
