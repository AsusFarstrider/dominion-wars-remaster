// Actual HD provider fixture with source-sized history and variable native lease.
namespace {
struct AdaptiveHdFixture:HdFixture {
    unsigned outW,outH,outStride;
    AdaptiveHdFixture(unsigned w,unsigned h):outW(w),outH(h),outStride(w*2+8){
        QImageResearch image{};MenuCanvasTest::read(native,&image,sizeof(image));image.width=uint16_t(w);image.height=uint16_t(h);MenuCanvasTest::put(native,image);
        surface.pixels.assign(size_t(outStride)*h,0x33);nativePixels=surface.pixels.data();armOutput();
    }
    void armOutput(){arm();lease.d.dwWidth=outW;lease.d.dwHeight=outH;lease.d.lPitch=int(outStride);lease.d.lpSurface=nativePixels;}
    int drawOutput(){return copy(fakeCopy,0x40F2ED,reinterpret_cast<void*>(hdHandle),nativePixels,int(outStride),1440,80,60,10);}
};
void runAdaptiveCases(){
    for(auto size:std::array<std::array<unsigned,2>,8>{{{1024,768},{1280,800},{1366,768},{1920,1080},{1920,1200},{2560,1440},{3840,2160},{3440,1440}}}){
        AdaptiveHdFixture f(size[0],size[1]);f.open();check(f.drawOutput()==0,"HD actual provider presents at launch-selected target dimensions");
        auto& history=*movies[hdHandle]->hd;
        check(history.width==2560&&history.height==1440&&history.pitch==5120&&history.pixels.size()==2560*1440+2*HdHistory::guard,"decode history remains full source-sized at every target");
        check(moviePublication.targetW==size[0]&&moviePublication.targetH==size[1]&&lease.written&&!moviePublication.interactive,"publication stores actual output dimensions before Unlock");
        f.unlock();f.physicalPoint(int(size[0])-1,int(size[1])-1);
        const auto skip=f.route();
        check(skip==0x41DADC,"HD skip covers committed output including far edge");
        auto videoPlan=MenuCanvas::Plan::fit(2560,1440,size[0],size[1]);const auto r=videoPlan->geometry().content_bounds();
        check(f.pixel(size_t((r.top+r.bottom)/2)*f.outStride+size_t((r.left+r.right)/2)*2)==0x07E0,"fitted video content is preserved");
        if(r.left||r.top)check(f.pixel()==0,"fitted video margins cleared without stretching");
        f.armOutput();check(f.drawOutput()==0&&observedFlags==10&&history.pixels[HdHistory::guard+100]==0x07E0,"selective update retains unmodified source history");
        const auto prior=f.surface.pixels;f.armOutput();codecResult=1;check(f.drawOutput()==1&&f.surface.pixels==prior,"result1 reuses full source frame at target size");
        f.unlock(E_FAIL);check(!moviePublication.interactive,"failed native Unlock never publishes fitted input");
        check(history.intact(),"source history guards survive all presentation sizes");
    }
}
}
