#include <Remaster/Runtime.h>
#pragma once
#include "ForegroundMask.h"
#include "quantum_font_research.h"
#include <Remaster/MainButtons.h>
#include <Remaster/OrderedPaint.h>
#include <Research/OpeningFlow/CardImage.h>
#include <Research/FlightRecorder/Recorder.h>
#include <Windows.h>
#include <bcrypt.h>
#include <array>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
namespace MainBackdrop {
struct Target {uint32_t outer=0,surface=0;unsigned w=0,h=0;};
inline bool same(Target a,Target b){return a.outer==b.outer&&a.surface==b.surface&&a.w==b.w&&a.h==b.h;}
inline bool enabled=false;inline DWORD thread=0;
inline std::shared_ptr<OpeningFlow::CardImage> artwork;
inline bool read(uint32_t address,void* out,size_t bytes) {
#if defined(MENU_CANVAS_TEST)||defined(OPENING_FLOW_TEST)
    if(MenuCanvasTest::managed(address,bytes))return MenuCanvasTest::read(address,out,bytes);
#endif
    if(address<0x10000||uint64_t(address)+bytes>0x100000000ull||bytes>1024*1024)return false;
    SIZE_T count=0;return ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(address),out,bytes,&count)&&count==bytes;
}
template<class T>inline T value(uint32_t address){T out{};read(address,&out,sizeof(out));return out;}
template<class T>inline void write(uint32_t address,const T& data) {
#if defined(MENU_CANVAS_TEST)||defined(OPENING_FLOW_TEST)
    if(MenuCanvasTest::managed(address,sizeof(data))){MenuCanvasTest::write(address,&data,sizeof(data));return;}
#endif
    SIZE_T count=0;if(!WriteProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(address),&data,sizeof(data),&count)||count!=sizeof(data))throw std::runtime_error("Main private state write");
}
inline void event(const char* kind,const std::string& fields={}){Qfr::event(kind,fields);}
struct Layer {
    Target target;uint64_t scope=0;
    bool good=true,mainOwns=false,background=false,nativeClear=false,nativeWritten=false,composed=false;
    uint32_t lastNode=0,lastCaller=0,lastSourceAddress=0,lastDestination=0;bool sourceRead=false;QImageResearch lastSource{};
    unsigned draws=0;std::vector<uint16_t> pixels,originalBackground;
    bool ordered=Remaster::MainButtons::available;Remaster::OrderedPaint commands;
    MenuCanvas::ForegroundMask mask{800,600};QImageResearch image{};
    Layer(Target t,uint64_t s):target(t),scope(s),pixels(800*600+64,0xA55A) {
        std::fill(pixels.begin()+32,pixels.end()-32,uint16_t{});
        image.unknown_02=255;image.and_mask=255;image.pixel_bits=16;image.width=800;image.height=600;image.pitch=1600;
        image.pixels=uint32_t(reinterpret_cast<uintptr_t>(pixels.data()+32));
    }
    void fail(const char* reason) {
        if(good)event("native_main_background_reject",",\"reason\":"+Qfr::quote(reason)+",\"scope\":"+std::to_string(scope)+
            ",\"node\":"+std::to_string(lastNode)+",\"caller\":"+std::to_string(lastCaller)+",\"source\":"+std::to_string(lastSourceAddress)+
            ",\"destination\":"+std::to_string(lastDestination)+",\"source_read\":"+(sourceRead?"true":"false")+
            ",\"flags\":"+std::to_string(lastSource.flags)+",\"tag\":"+std::to_string(lastSource.unknown_02)+
            ",\"bits\":"+std::to_string(lastSource.pixel_bits)+",\"and_mask\":"+std::to_string(lastSource.and_mask)+
            ",\"xor_value\":"+std::to_string(lastSource.xor_value)+",\"width\":"+std::to_string(lastSource.width)+",\"height\":"+std::to_string(lastSource.height));good=false;
    }
    bool intact()const noexcept {for(size_t i=0;i<32;++i)if(pixels[i]!=0xA55A||pixels[pixels.size()-1-i]!=0xA55A)return false;return true;}
};
inline std::atomic<bool> coverageGapPending{false};
inline std::weak_ptr<Layer> current;inline uint64_t frames=0;inline bool pixelReceipt=false;
inline std::shared_ptr<Layer> begin(Target t,uint64_t scope) {
    if(!enabled||thread!=GetCurrentThreadId()||(!artwork&&!Remaster::MainButtons::available))return {};
    auto layer=std::make_shared<Layer>(t,++frames);(void)scope;current=layer;return layer;
}
inline std::shared_ptr<Layer> borrow(Target t,uint64_t scope) {
    if(!enabled||thread!=GetCurrentThreadId())return {};
    // The outer controller can still own a completed layer after inner Main
    // releases it. Never recycle that completed coverage into another frame.
    if(auto layer=current.lock();layer&&same(t,layer->target)&&layer->good&&!layer->composed)return layer;
    return begin(t,scope);
}
inline void boundary() {if(auto layer=current.lock()){layer->good=false;layer->mainOwns=false;}current.reset();Remaster::MainButtons::invalidate();}
// Foreign hooks only set an atomic signal; all shared layer mutation stays on the owner.
inline bool drainCoverageGap() {if(!coverageGapPending.exchange(false))return false;boundary();return true;}
enum class Mirror {failed,clear,background,foreground};
inline Mirror mirror(Layer& layer,const uint32_t* args,void* original,uint32_t node) {
    if(!layer.good)return Mirror::failed;
    layer.lastNode=node;layer.lastCaller=args[0];layer.lastSourceAddress=args[1];layer.lastDestination=args[2];layer.sourceRead=false;layer.lastSource={};
    if(!original||args[2]!=layer.target.outer){layer.fail("unsupported_destination_or_missing_trampoline");return Mirror::failed;}
    QImageResearch src{};if(!read(args[1],&src,sizeof(src))){layer.fail("unreadable_source");return Mirror::failed;}
    layer.sourceRead=true;layer.lastSource=src;
    const bool nativeClear=node==0x4E47C0&&args[0]==0x4A1774&&src.flags==0&&src.unknown_02==255&&src.and_mask==0&&src.xor_value==0&&
        src.pixel_bits==8&&!src.origin_x&&!src.origin_y&&src.width==layer.target.w&&src.height==layer.target.h&&!args[3]&&!args[4]&&layer.target.w>800;
    if(nativeClear){if(layer.nativeClear||layer.background||layer.draws){layer.fail("late_or_repeated_clear");return Mirror::failed;}layer.nativeClear=true;return Mirror::clear;}
    const bool solid=(src.flags==0||src.flags==1)&&src.and_mask==0&&src.pixel_bits==8;
    const bool operation=src.and_mask==0||(src.and_mask==255&&src.xor_value==0);
    // Live Preferences uses flags0x13. Original49F050 does not branch on
    // low0x10: with storage0x200 clear this is the same clipped keyed P8 path.
    // Admit this observed value explicitly; do not mask away unknown flags.
    if((src.flags!=1&&src.flags!=3&&src.flags!=0x13&&!solid)||!operation||src.pixel_bits!=8||src.unknown_02!=255||(!solid&&src.surface)||!src.width||!src.height||uint64_t(src.width)*src.height>800*600) {
        layer.fail("unsupported_raw_producer");return Mirror::failed;
    }
    uint32_t palette=value<uint32_t>(0x4EC390);std::array<unsigned char,512> ramp{};
    if(!palette||!read(palette,ramp.data(),ramp.size())){layer.fail("invalid_live_palette");return Mirror::failed;}
    const int64_t x=int32_t(args[3])+int64_t(src.origin_x),y=int32_t(args[4])+int64_t(src.origin_y);
    if(x < -32768||y < -32768||x+src.width>32767||y+src.height>32767||
        (solid&&src.flags==0&&(x<0||y<0||x+src.width>800||y+src.height>600))){layer.fail("unsafe_unclipped_producer");return Mirror::failed;}
    uint32_t previous=value<uint32_t>(0xA39B78);std::array<int16_t,4> clip{},local{0,0,800,600};
    if(!read(previous?previous:layer.target.outer+0x46,clip.data(),sizeof(clip))||clip[2]<0||clip[3]<0){layer.fail("unreadable_clip");return Mirror::failed;}
    auto intersection=dw::ui::intersect({0,0,800,600},{clip[0],clip[1],int32_t(clip[0])+clip[2],int32_t(clip[1])+clip[3]});
    if(intersection)local={int16_t(intersection->left),int16_t(intersection->top),int16_t(intersection->right-intersection->left),int16_t(intersection->bottom-intersection->top)};
    else local={0,0,0,0};
    const bool background=node==0x4E47C0&&args[0]==0x46AE63&&value<uint32_t>(node+0x50)==0x201&&src.flags==1&&src.and_mask==255&&src.xor_value==0&&
        src.width==800&&src.height==600&&!src.origin_x&&!src.origin_y&&!args[3]&&!args[4]&&!previous&&local[0]==0&&local[1]==0&&local[2]==800&&local[3]==600;
    if(background&&layer.background){layer.fail("repeated_201_background");return Mirror::failed;}
    std::vector<uint8_t> indices;
    if(!solid){indices.resize(size_t(src.width)*src.height);if(!read(src.pixels,indices.data(),indices.size())){layer.fail("unreadable_indices");return Mirror::failed;}}
    constexpr std::array<uint32_t,11> scratch={0xA39B50,0xA39B54,0xA39B58,0xA39B5C,0xA39B60,0xA39B68,0xA39B6C,0xA39B70,0xA39B74,0xA39B7C,0xA39D80};
    std::array<uint32_t,11> saved{};for(size_t i=0;i<scratch.size();++i)if(!read(scratch[i],&saved[i],sizeof(saved[i]))){layer.fail("unreadable_original_scratch");return Mirror::failed;}
    struct Restore {uint32_t source,previous;int16_t pitch;const std::array<uint32_t,11>& addresses;const std::array<uint32_t,11>& saved;
        ~Restore(){write(source+0x16,pitch);write(0xA39B78,previous);for(size_t i=0;i<addresses.size();++i)write(addresses[i],saved[i]);}} restore{args[1],previous,src.pitch,scratch,saved};
    write(0xA39B78,uint32_t(reinterpret_cast<uintptr_t>(local.data())));
    using Blit=uint32_t(__cdecl*)(QImageResearch*,QImageResearch*,int,int);
    reinterpret_cast<Blit>(original)(reinterpret_cast<QImageResearch*>(args[1]),&layer.image,int32_t(args[3]),int32_t(args[4]));
    ++layer.draws;
    if(background){layer.background=true;if(layer.ordered&&!artwork)layer.originalBackground.assign(layer.pixels.begin()+32,layer.pixels.end()-32);}
    else if(!layer.mask.mark(indices.data(),indices.size(),src.width,src.height,x,y,(src.flags&2)!=0,solid,
        (src.flags&1)?dw::ui::Rect{local[0],local[1],int32_t(local[0])+local[2],int32_t(local[1])+local[3]}:dw::ui::Rect{0,0,800,600})){layer.fail("invalid_coverage");return Mirror::failed;}
    if(!background&&layer.ordered){
        const auto clipRect=(src.flags&1)?dw::ui::Rect{local[0],local[1],int32_t(local[0])+local[2],int32_t(local[1])+local[3]}:dw::ui::Rect{0,0,800,600};
        if(Remaster::MainButtons::index(node)>=0){
            const auto i=Remaster::MainButtons::index(node);const auto& b=Remaster::MainButtons::table[i];
            auto frame=Remaster::MainButtons::consume(layer.scope,node,args[1],args[0]);
            if(!frame||src.flags!=3||src.and_mask!=255||src.xor_value||src.width!=180||src.height!=100||src.origin_x||src.origin_y||x!=320||y!=b.y||
               !layer.commands.sprite(*frame,{x,y,x+180,y+100},clipRect)){layer.fail("native_button_draw_witness_or_geometry");return Mirror::failed;}
        }else if(!layer.commands.raw(layer.pixels.data()+32,indices.data(),indices.size(),src.width,src.height,x,y,(src.flags&2)!=0,solid,clipRect)){
            layer.fail("ordered_foreground_limit_or_shape");return Mirror::failed;}
    }
    return background?Mirror::background:Mirror::foreground;
}
inline bool compose(Layer& layer,const MenuCanvas::Plan& uiPlan,MenuCanvas::WriteView dst) {
    if(drainCoverageGap()){layer.good=false;layer.mainOwns=false;return false;}
    if(!layer.good||layer.composed||!layer.background||!layer.intact()||(layer.target.w>800&&!layer.nativeClear)||(!artwork&&!layer.ordered))return false;
    std::vector<unsigned char> packed;
    const bool receipt=artwork&&Qfr::enabled()&&!pixelReceipt&&artwork->identity(dst.width,dst.height);
    if(receipt)packed.resize(artwork->bytes());
    layer.nativeWritten=true;
    if(artwork){if(!artwork->compose(dst))return false;}
    else if(layer.originalBackground.size()!=800*600||!uiPlan.compose({layer.originalBackground.data(),800*600*2,1600,800,600},dst))return false;
    if(receipt) {
        for(unsigned y=0;y<dst.height;++y)memcpy(packed.data()+size_t(y)*dst.width*2,static_cast<unsigned char*>(dst.pixels)+size_t(y)*dst.pitch,size_t(dst.width)*2);
        unsigned char a[32]{},b[32]{};
        const bool hashed=BCryptHash(BCRYPT_SHA256_ALG_HANDLE,nullptr,0,static_cast<PUCHAR>(const_cast<void*>(artwork->data())),ULONG(artwork->bytes()),a,32)>=0&&
            BCryptHash(BCRYPT_SHA256_ALG_HANDLE,nullptr,0,packed.data(),ULONG(packed.size()),b,32)>=0;
        const bool equal=hashed&&!memcmp(a,b,32);std::string sa,sb;const char* hex="0123456789abcdef";
        for(unsigned i=0;i<32;++i){sa+=hex[a[i]>>4];sa+=hex[a[i]&15];sb+=hex[b[i]>>4];sb+=hex[b[i]&15];}
        event("native_main_background_pixels",",\"scope\":"+std::to_string(layer.scope)+",\"source_sha256\":"+Qfr::quote(sa)+",\"target_sha256\":"+Qfr::quote(sb)+",\"equal\":"+(equal?"true":"false"));
        if(!equal){layer.fail("native_underlay_pixel_mismatch");return false;}pixelReceipt=true;
    }
    size_t overlaid=0;
    if(layer.ordered){if(!layer.commands.compose(uiPlan.geometry(),dst,&overlaid))return false;}
    else if(!uiPlan.overlay({layer.pixels.data()+32,800*600*2,1600,800,600},layer.mask.data(),layer.mask.bytes(),dst,&overlaid))return false;
    layer.composed=true;
    event("native_main_background_composed",",\"scope\":"+std::to_string(layer.scope)+",\"surface\":"+std::to_string(layer.target.surface)+
        ",\"source_width\":"+std::to_string(artwork?artwork->width():800)+",\"source_height\":"+std::to_string(artwork?artwork->height():600)+
        ",\"target_width\":"+std::to_string(dst.width)+",\"target_height\":"+std::to_string(dst.height)+
        ",\"logical_coverage\":"+std::to_string(layer.mask.covered())+",\"physical_coverage\":"+std::to_string(overlaid)+
        ",\"ordered_foreground\":"+(layer.ordered?"true":"false")+",\"identity_background\":"+(artwork&&artwork->identity(dst.width,dst.height)?"true":"false"));
    return true;
}
inline void install() {
    const bool nativeArt=Remaster::feature(Remaster::Feature::mainBackground,L"QFR_MAIN_BACKGROUND");
    if(!nativeArt&&!Remaster::MainButtons::available)return;
    if(!nativeArt){thread=GetCurrentThreadId();enabled=true;return;}
    wchar_t exe[32768]{};if(!GetModuleFileNameW(nullptr,exe,32768))throw std::runtime_error("Main asset module path");
    const auto path=std::filesystem::path(exe).parent_path()/L"rs/Remaster/OpeningCards/main.dwcard";
    std::ifstream f(path,std::ios::binary);std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(f)),{});
    if(bytes.empty()||bytes.size()>64*1024*1024)throw std::runtime_error("Main asset missing/size");
    unsigned char digest[32]{};if(BCryptHash(BCRYPT_SHA256_ALG_HANDLE,nullptr,0,bytes.data(),ULONG(bytes.size()),digest,32)<0)throw std::runtime_error("Main asset hash");
    std::string hash;const char* hex="0123456789abcdef";for(auto b:digest){hash+=hex[b>>4];hash+=hex[b&15];}
    if(hash!="94a131d5b71bec90e3d747a6475c18ece4ad6efdc98b5310b9daa74c7d3c0bcd")throw std::runtime_error("Main asset identity");
    artwork=OpeningFlow::CardImage::parse(bytes);if(!artwork)throw std::runtime_error("Main asset format");
    thread=GetCurrentThreadId();enabled=true;event("native_main_background_asset",",\"resource\":513,\"width\":2560,\"height\":1440,\"asset_sha256\":"+Qfr::quote(hash));
}
}
