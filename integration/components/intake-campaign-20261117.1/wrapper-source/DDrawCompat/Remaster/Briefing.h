#pragma once
#include "BriefingPins.h"
#include "OutputMode.h"
#include <Research/MenuCanvas/MainBackdrop.h>
#include <Common/Hook.h>
#include <intrin.h>
#include <set>

namespace Briefing {
inline constexpr uint32_t constructorAddress=0x415550;
struct Layout {
    unsigned width=800;
    unsigned extra() const noexcept {return width-800;}
    static Layout forOutput(unsigned w,unsigned h) noexcept {
        if(w<800||h<600)return {};
        const uint64_t logical=(uint64_t(w)*600+h/2)/h;
        if(logical<=800||logical>1600||logical>w)return {};
        return {unsigned(logical)};
    }
    int shift(uint32_t caller) const noexcept {
        switch(caller) {
        case 0x415BCF:case 0x415C17:case 0x415C41:return int((extra()+1)/2);
        case 0x415C78:case 0x415B04:case 0x4159EF:case 0x4159E3:
        case 0x415CC3:case 0x415CB7:case 0x415CFC:case 0x415CF0:return int(extra());
        default:return 0;
        }
    }
    bool objective(uint32_t caller,std::array<int16_t,4>& rect,int line,int alignment,int skip) const noexcept {
        if(caller!=0x415A9A||rect[0]!=133||rect[2]!=315||rect[3]!=25||
            (rect[1]!=355&&rect[1]!=421&&rect[1]!=487)||line!=15||alignment||skip)return false;
        rect[2]=int16_t(315+extra());return true;
    }
};
struct Entry {
    uint32_t key=0;int16_t ox=0,oy=0;uint16_t w=0,h=0;
    std::array<uint8_t,32> identity{};
    Remaster::SpriteFrame frame,left,middle,right;
};
struct Assets {
    std::vector<Entry> entries;
    std::vector<uint16_t> background;
    unsigned targetW=0,targetH=0,logicalW=0;
    const Entry* get(uint32_t key) const noexcept {
        for(const auto& e:entries)if(e.key==key)return &e;
        return nullptr;
    }
    static Remaster::SpriteFrame crop(const Remaster::SpriteFrame& f,unsigned x,unsigned width) {
        auto pixels=std::make_shared<std::vector<uint8_t>>(size_t(width)*f.height*4);
        for(unsigned y=0;y<f.height;++y)memcpy(pixels->data()+size_t(y)*width*4,
            f.pixels()+(size_t(y)*f.width+x)*4,size_t(width)*4);
        return {pixels,0,width,f.height};
    }
    static std::shared_ptr<Assets> parse(std::vector<uint8_t> bytes) {
        if(bytes.size()<12||bytes.size()>64*1024*1024||memcmp(bytes.data(),"DWBRIEF1",8))return {};
        auto storage=std::make_shared<std::vector<uint8_t>>(std::move(bytes));size_t pos=8;
        const auto take=[&](void* dst,size_t n){if(n>storage->size()-pos)return false;memcpy(dst,storage->data()+pos,n);pos+=n;return true;};
        uint32_t count=0;if(!take(&count,4)||count!=20)return {};
        auto result=std::make_shared<Assets>();std::set<uint32_t> seen;
        for(unsigned i=0;i<count;++i) {
            Entry e;uint32_t pw=0,ph=0;
            if(!take(&e.key,4)||!take(&e.ox,2)||!take(&e.oy,2)||!take(&e.w,2)||!take(&e.h,2)||
                !take(&pw,4)||!take(&ph,4)||!take(e.identity.data(),32))return {};
            if(e.key<0xB101||e.key>0xB114||!seen.insert(e.key).second||e.ox||e.oy||!e.w||!e.h||e.w>800||e.h>600)return {};
            const unsigned density=e.key==0xB101?4:8;
            if(pw!=e.w*density||ph!=e.h*density)return {};
            const uint64_t size=uint64_t(pw)*ph*4;
            if(size>storage->size()-pos)return {};
            e.frame={storage,pos,pw,ph};pos+=size_t(size);
            if(e.key==0xB111) {
                if(e.w!=363||e.h!=64)return {};
                e.left=crop(e.frame,0,150*8);e.middle=crop(e.frame,150*8,8);
                e.right=crop(e.frame,151*8,(363-151)*8);
            }
            result->entries.push_back(std::move(e));
        }
        if(pos!=storage->size())return {};
        const auto* bg=result->get(0xB101);if(!bg||bg->w!=800||bg->h!=600)return {};
        return result;
    }
    bool prepare(const MenuCanvas::Plan& plan,unsigned w,unsigned h,unsigned logical) {
        if(w==targetW&&h==targetH&&logical==logicalW&&!background.empty())return true;
        const auto* bg=get(0xB101);if(!bg)return false;
        const auto bounds=plan.geometry().content_bounds();
        std::vector<uint16_t> next(size_t(w)*h);
        const double sx=double(logical)/double(bounds.right-bounds.left);
        const double sy=600.0/double(bounds.bottom-bounds.top);
        for(auto y=bounds.top;y<bounds.bottom;++y) {
            const unsigned yy=std::min(2399u,unsigned((double(y-bounds.top)+0.5)*sy*4));
            for(auto x=bounds.left;x<bounds.right;++x) {
                double xx=(double(x-bounds.left)+0.5)*sx;
                if(xx>=384+logical-800)xx-=logical-800;
                else if(xx>=384)xx=383.875;
                const unsigned sourceX=std::min(3199u,unsigned(xx*4));
                const auto* pixel=bg->frame.pixels()+(size_t(yy)*3200+sourceX)*4;
                next[size_t(y)*w+size_t(x)]=uint16_t((pixel[0]>>3)<<11|(pixel[1]>>2)<<5|(pixel[2]>>3));
            }
        }
        background=std::move(next);targetW=w;targetH=h;logicalW=logical;return true;
    }
};
inline std::shared_ptr<Assets> assets;
inline Layout layout;
inline DWORD thread=0;
inline bool active=false;
inline uint64_t composed=0,refused=0;
inline uint64_t diagnosticFailures=0;
inline std::set<std::string> reported;
inline void note(const char* event,std::string_view detail) noexcept {
    try{if(reported.size()<64&&reported.insert(std::string(event)+std::string(detail)).second)Remaster::note(event,std::string(detail));}
    catch(...){++diagnosticFailures;}
}
inline bool admitted() noexcept {
    return active&&assets&&thread==GetCurrentThreadId()&&MainBackdrop::value<uint32_t>(0x71386C)==0;
}
using Construct=void(__cdecl*)();
inline Construct originalConstruct=nullptr;
inline void __cdecl construct() {
    const DWORD before=GetLastError();
    try {
        const auto next=assets&&Remaster::Output::bound&&thread==GetCurrentThreadId()&&MainBackdrop::value<uint32_t>(0x71386C)==0?
            Layout::forOutput(Remaster::Output::session.width,Remaster::Output::session.height):Layout{};
        std::array<int16_t,4> rect{};
        if(!MainBackdrop::read(0x4C7F38,rect.data(),sizeof(rect))||rect[0]!=121||rect[1]!=110||rect[3]!=196||
            (rect[2]!=240&&rect[2]!=int(240+layout.extra())))throw std::runtime_error("briefing body rectangle identity");
        const int16_t width=int16_t(240+next.extra());
        MainBackdrop::write(0x4C7F3C,width);layout=next;active=next.extra()!=0;
        note("briefing_layout_construct","logical_width="+std::to_string(layout.width)+"; body_width="+std::to_string(width)+"; recount=original constructor");
    }catch(const std::exception& e) {
        active=false;note("briefing_layout_refused",e.what());
    }
    SetLastError(before);originalConstruct();const DWORD after=GetLastError();
    try{if(admitted())note("briefing_body_layout","width="+std::to_string(240+layout.extra())+
        "; lines="+std::to_string(MainBackdrop::value<uint32_t>(0x5EFB98))+
        "; whole_scroll="+std::to_string(MainBackdrop::value<uint32_t>(0x5EFBBC)));}
    catch(...){++diagnosticFailures;}
    SetLastError(after);
}
struct Layer {
    MainBackdrop::Layer shadow;
    Layout geometry;
    uint32_t fontCaller=0;
    unsigned dense=0;
    bool background=false,good=true;
    explicit Layer(MainBackdrop::Target target):shadow(target,0,layout.width),geometry(layout){shadow.ordered=true;shadow.reportErrors=false;}
    void fail(const char* reason) {
        if(good){++refused;note("briefing_layout_reject",reason);}good=false;
    }
    bool captionRectangle(uint32_t caller,uint32_t node,std::array<int16_t,4>& rect) {
        if(caller!=0x41EC64||node!=0x4C7BD0||!admitted())return false;
        const auto value=[](uint32_t address){return MainBackdrop::value<uint32_t>(address);};
        const auto refuse=[](const char* why){note("briefing_caption_refused",why);return false;};
        if(value(node)!=0x3EC||value(node+0x77)!=0x4C7AB0||
            MainBackdrop::value<uint16_t>(node+0x10)!=0xB102||MainBackdrop::value<uint16_t>(node+0x5C)!=0x1406)
            return refuse("Exit widget identity");
        // 41EA00 builds this rectangle before 41E7F0 copies the caption to a
        // temporary line buffer. Match that owner/rectangle, not the later copy.
        const int x=MainBackdrop::value<int16_t>(node+8),y=MainBackdrop::value<int16_t>(node+10);
        const int w=MainBackdrop::value<uint16_t>(node+12),h=MainBackdrop::value<uint16_t>(node+14);
        const int dx=MainBackdrop::value<int16_t>(node+0x62),dy=MainBackdrop::value<int16_t>(node+0x64);
        if(x!=9||y!=556||w!=80||h!=34||dx!=15||dy!=16||rect[0]!=x+dx||
            rect[1]<y+dy-2||rect[1]>y+dy+2||rect[2]!=w-2*dx||rect[3]!=h-2*dy)
            return refuse("Exit caption rectangle");
        QFontStateResearch state{};QFontHeaderResearch face{};
        if(!MainBackdrop::read(value(0x4EC3B8),&state,sizeof(state))||!MainBackdrop::read(state.font,&face,sizeof(face))||
            state.effects||face.pixel_bits!=8||face.first_code!=32||face.last_code!=127||face.glyph_count!=96)
            return refuse("Exit font metrics");
        const auto text=value(node+0x58);
        int top=32767,bottom=-32768;bool ended=false;
        for(unsigned i=0;i<64;++i) {
            uint8_t c=0;if(!MainBackdrop::read(text+i,&c,1))return refuse("Exit caption unreadable");
            if(!c){ended=true;break;}
            if(c<face.first_code||c>face.last_code)return refuse("Exit caption character");
            const uint64_t address=uint64_t(state.font)+12+uint64_t(c-face.first_code)*12;
            QGlyphResearch glyph{};
            if(address>0xFFFFFFFFull||!MainBackdrop::read(uint32_t(address),&glyph,sizeof(glyph))||
                !glyph.width||!glyph.height||glyph.width>64||glyph.height>64)return refuse("Exit glyph metrics");
            top=std::min(top,int(glyph.origin_y));bottom=std::max(bottom,int(glyph.origin_y)+glyph.height);
        }
        const int inkHeight=bottom-top;
        if(!ended||inkHeight<=0||inkHeight>h)return refuse("Exit caption height");
        const int shift=(h-inkHeight)/2-dy-int(face.baseline_adjust)-top;
        if(shift>0||shift < -h)return refuse("Exit caption displacement");
        rect[1]=int16_t(rect[1]+shift);
        note("briefing_caption_alignment","Exit; caller=41EC64; ink_height="+std::to_string(inkHeight)+
            "; y_shift="+std::to_string(shift)+"; adjusted_rect_y="+std::to_string(rect[1])+"; original_x_and_hitbox=true");
        return true;
    }
    const Entry* identify(const QImageResearch& src) {
        if(!assets||src.pixel_bits!=8||src.origin_x||src.origin_y||src.width>800||src.height>600)return nullptr;
        if(std::none_of(assets->entries.begin(),assets->entries.end(),[&](const auto& e){return e.w==src.width&&e.h==src.height;}))return nullptr;
        std::vector<uint8_t> indices(size_t(src.width)*src.height);
        if(!MainBackdrop::read(src.pixels,indices.data(),indices.size()))return nullptr;
        std::array<uint8_t,32> hash{};
        if(BCryptHash(BCRYPT_SHA256_ALG_HANDLE,nullptr,0,indices.data(),ULONG(indices.size()),hash.data(),32)<0)return nullptr;
        for(const auto& e:assets->entries)if(e.w==src.width&&e.h==src.height&&e.identity==hash)return &e;
        return nullptr;
    }
    void mirror(const uint32_t* args,void* original,uint32_t node) {
        if(!good)return;
        if(!admitted()||geometry.width!=layout.width){fail("owner changed during paint");return;}
        QImageResearch src{};
        if(!MainBackdrop::read(args[1],&src,sizeof(src))){fail("source descriptor unreadable");return;}
        const auto* e=identify(src);
        if(e&&e->key==0xB101) {
            if(background||node!=0x4C7AB0||args[0]!=0x415B60||args[3]||args[4]||
                src.flags!=1||src.and_mask!=255||src.xor_value||src.surface){fail("background owner or order");return;}
            background=true;return;
        }
        if(!background){fail("foreground before original background identity");return;}
        std::array<uint32_t,5> shifted{};std::copy_n(args,5,shifted.data());
        const auto caller=args[0]==0x4A2B1B?fontCaller:args[0];
        int shift=geometry.shift(caller);
        shifted[3]=uint32_t(int32_t(args[3])+shift);
        const bool upper=node==0x4C7AB0&&args[0]==0x415CC3&&args[1]==MainBackdrop::value<uint32_t>(0x5EFBA4)&&args[3]==470&&args[4]==128;
        const bool lower=node==0x4C7AB0&&args[0]==0x415CFC&&args[1]==MainBackdrop::value<uint32_t>(0x5EFBA0)&&args[3]==528&&args[4]==351;
        const bool native16=(upper||lower)&&src.flags==1&&src.pixel_bits==16&&src.and_mask==255&&!src.xor_value&&!src.surface&&src.pixels>=0x10000;
        if(MainBackdrop::mirror(shadow,shifted.data(),original,node,e==nullptr,native16)==MainBackdrop::Mirror::failed) {
            note("briefing_producer_reject","caller="+std::to_string(args[0])+"; bits="+std::to_string(src.pixel_bits)+
                "; flags="+std::to_string(src.flags)+"; width="+std::to_string(src.width)+"; height="+std::to_string(src.height));
            fail("foreground mirror refused");return;
        }
        if(native16)note("briefing_native_image_draw",std::string(upper?"upper":"lower")+"; bits=16; width="+std::to_string(src.width)+"; height="+std::to_string(src.height));
        if(!e)return;
        const int64_t x=int32_t(shifted[3]),y=int32_t(shifted[4]);
        std::array<int16_t,4> sourceClip{};
        const auto clipPointer=MainBackdrop::value<uint32_t>(0xA39B78);
        if(!MainBackdrop::read(clipPointer?clipPointer:shadow.target.outer+0x46,sourceClip.data(),sizeof(sourceClip))){fail("replacement clip unreadable");return;}
        const dw::ui::Rect clip=(src.flags&1)?dw::ui::Rect{sourceClip[0],sourceClip[1],int(sourceClip[0])+sourceClip[2],int(sourceClip[1])+sourceClip[3]}:
            dw::ui::Rect{0,0,geometry.width,600};bool ok=false;
        if(e->key==0xB111) {
            ok=shadow.commands.sprite(e->left,{x,y,x+150,y+64},clip)&&
                shadow.commands.sprite(e->middle,{x+150,y,x+151+geometry.extra(),y+64},clip)&&
                shadow.commands.sprite(e->right,{x+151+geometry.extra(),y,x+e->w+geometry.extra(),y+64},clip);
        }else ok=shadow.commands.sprite(e->frame,{x,y,x+e->w,y+e->h},clip);
        if(!ok){fail("replacement command outside layout");return;}++dense;
        note("briefing_art_draw","resource="+std::to_string(e->key));
    }
    bool compose(const MenuCanvas::Plan& plan,MenuCanvas::WriteView dst) {
        if(!good||!background||!shadow.intact()||!assets||geometry.width!=plan.geometry().logical_bounds().right)return false;
        if(!assets->prepare(plan,dst.width,dst.height,geometry.width))return false;
        for(unsigned y=0;y<dst.height;++y)memcpy(static_cast<uint8_t*>(dst.pixels)+size_t(y)*dst.pitch,
            assets->background.data()+size_t(y)*dst.width,size_t(dst.width)*2);
        if(!shadow.commands.compose(plan.geometry(),dst))return false;
        ++composed;note("briefing_fullwidth_commit","width="+std::to_string(dst.width)+"; height="+std::to_string(dst.height)+
            "; logical_width="+std::to_string(geometry.width)+"; dense_draws="+std::to_string(dense));return true;
    }
};
inline std::shared_ptr<Layer> begin(MainBackdrop::Target t) {
    return admitted()?std::make_shared<Layer>(t):nullptr;
}
inline void install() {
    if(!Remaster::enabled())return;
    wchar_t exe[32768]{};if(!GetModuleFileNameW(nullptr,exe,32768))return;
    const auto path=std::filesystem::path(exe).parent_path()/L"rs/Remaster/Briefing/briefing.dwbrief";
    if(!std::filesystem::exists(path))return;
    std::ifstream stream(path,std::ios::binary|std::ios::ate);
    if(!stream||stream.tellg()<=0||uint64_t(stream.tellg())>64*1024*1024)throw std::runtime_error("briefing asset size");
    std::vector<uint8_t> bytes(size_t(stream.tellg()));stream.seekg(0);
    if(!stream.read(reinterpret_cast<char*>(bytes.data()),bytes.size()))throw std::runtime_error("briefing asset read");
    std::array<uint8_t,32> hash{};
    if(BCryptHash(BCRYPT_SHA256_ALG_HANDLE,nullptr,0,bytes.data(),ULONG(bytes.size()),hash.data(),32)<0)throw std::runtime_error("briefing asset hash");
    std::string actual;const char* hex="0123456789abcdef";
    for(auto b:hash){actual+=hex[b>>4];actual+=hex[b&15];}
    if(actual!=assetHash)throw std::runtime_error("briefing asset identity");
    auto next=Assets::parse(std::move(bytes));if(!next)throw std::runtime_error("briefing asset format");
    std::array<uint8_t,6> ctor{};
    const uint8_t ctorBytes[]={0x8B,0x0D,0x6C,0x38,0x71,0x00};
    if(!MainBackdrop::read(constructorAddress,ctor.data(),ctor.size())||memcmp(ctor.data(),ctorBytes,ctor.size()))throw std::runtime_error("briefing constructor signature");
    void* constructor=reinterpret_cast<void*>(constructorAddress);
    Compat::hookFunction(constructor,reinterpret_cast<void*>(&construct),"briefing_layout_constructor");
    originalConstruct=reinterpret_cast<Construct>(constructor);
    if(constructor==reinterpret_cast<void*>(constructorAddress))throw std::runtime_error("briefing constructor hook");
    assets=std::move(next);thread=GetCurrentThreadId();note("briefing_fullwidth_ready",assetHash);
}
}
