#pragma once
#include "SelectionCatalog.h"
#include "SelectionAssetPins.h"
#include <Research/MenuCanvas/MainBackdrop.h>
#include <bitset>

namespace SelectionRestoration {
inline std::shared_ptr<Catalog> catalog;
inline std::shared_ptr<OpeningFlow::CardImage> map;
inline unsigned flags=0;
inline DWORD thread=0;
inline std::set<uint64_t> receipts;
inline uint64_t passes=0,composed=0,rawFallbacks=0,denseGlyphs=0,denseBodies=0,denseFactions=0,failed=0;
struct Control {uint32_t node,body;};
inline constexpr Control controls[]={{0x4E50E0,0x802},{0x4E5170,0x802},{0x4E5200,0x15},{0x4E5290,0x15},{0x4E5320,0x15},{0x4E53B0,0x12}};
inline const Control* control(uint32_t node)noexcept{for(const auto& c:controls)if(c.node==node)return &c;return nullptr;}
struct Faction {uint32_t key,caller;int x,y;};
inline constexpr Faction factions[]={{0x809,0x47280D,195,523},{0x80A,0x47280D,195,523},{0x80B,0x47291F,153,456},{0x80C,0x47291F,153,456},
    {0x80D,0x472868,516,439},{0x80E,0x472868,516,439},{0x80F,0x4728C3,132,187},{0x810,0x4728C3,132,187}};
inline std::vector<uint16_t> fittedMap;
inline unsigned fittedW=0,fittedH=0;
inline dw::ui::Rect fittedCore{};
inline void receipt(uint64_t key,const std::string& text){if(receipts.size()<40&&receipts.insert(key).second)Remaster::note("selection_restore_draw",text);}
inline bool read(uint32_t a,void* p,size_t n){return MainBackdrop::read(a,p,n);}
template<class T>inline bool read(uint32_t a,T& out){return read(a,&out,sizeof(out));}
inline std::vector<uint8_t> file(const std::filesystem::path& path,size_t limit,const char* expected=nullptr){
    std::ifstream f(path,std::ios::binary|std::ios::ate);if(!f||f.tellg()<=0||uint64_t(f.tellg())>limit)throw std::runtime_error("selection asset missing/size");
    std::vector<uint8_t> b(size_t(f.tellg()));f.seekg(0);if(!f.read(reinterpret_cast<char*>(b.data()),b.size()))throw std::runtime_error("selection asset incomplete");
    if(expected){unsigned char digest[32]{};if(BCryptHash(BCRYPT_SHA256_ALG_HANDLE,nullptr,0,b.data(),ULONG(b.size()),digest,32)<0)throw std::runtime_error("selection asset SHA failed");
        std::string actual;const char* hex="0123456789abcdef";for(auto c:digest){actual+=hex[c>>4];actual+=hex[c&15];}
        if(actual!=expected)throw std::runtime_error("selection asset identity");}
    return b;
}
inline void install()noexcept {
    if(!Remaster::enabled())return;
    try{wchar_t exe[32768]{};if(!GetModuleFileNameW(nullptr,exe,32768))return;
        const auto folder=std::filesystem::path(exe).parent_path()/L"rs/Remaster/Selection";
        if(!std::filesystem::exists(folder/L"selection.flags"))return; // Complete original fallback when absent.
        const auto options=file(folder/L"selection.flags",1);if(options.size()!=1||options[0]>15)throw std::runtime_error("selection flags invalid");
        if(!options[0])return;
        auto next=Catalog::parse(file(folder/L"selection.dwselect",32*1024*1024,catalogHash));
        if(!next)throw std::runtime_error("selection catalog format");
        std::shared_ptr<OpeningFlow::CardImage> nextMap;
        if(options[0]&1){nextMap=OpeningFlow::CardImage::parse(file(folder/L"selection.dwcard",16*1024*1024,mapHash));if(!nextMap)throw std::runtime_error("selection map format");}
        catalog=std::move(next);map=std::move(nextMap);flags=options[0];thread=GetCurrentThreadId();
        Remaster::note("selection_restore_ready",std::string("catalog=")+catalogHash+"; map="+mapHash+"; flags="+std::to_string(flags)+"; six exact controls; font ArtPass2; faction Candidate02 with derived ink registration; independent cursor opt-in");
    }catch(const std::exception& e){flags=0;catalog.reset();map.reset();Remaster::note("selection_restore_original_retained",e.what());}
    catch(...){flags=0;catalog.reset();map.reset();Remaster::note("selection_restore_original_retained","unknown initialization error");}
}
struct Layer {
    Remaster::OrderedPaint commands;
    std::vector<uint16_t> originalBackground;
    bool background=false,good=true,faceChecked=false,faceValid=false;
    uint32_t checkedFace=0;
    Layer(){++passes;}
    bool fail(const char* why){good=false;receipt(2,std::string("original-frame fallback; ")+why);return false;}
    bool validateFace(uint32_t face){
        if(faceChecked)return face==checkedFace&&faceValid;faceChecked=true;checkedFace=face;
        QFontHeaderResearch h{};std::array<QGlyphResearch,96> glyphs{};
        if(!read(face,h)||h.first_code!=32||h.last_code!=127||h.pixel_bits!=8||h.tracking!=1||h.unknown_04!=1||h.max_y_extent!=24||h.baseline_adjust||h.glyph_count!=96||
            !read(face+12,glyphs.data(),sizeof(glyphs)))return false;
        for(unsigned i=0;i<96;++i){const auto* e=catalog->get(2,i+32);const auto& g=glyphs[i];
            if(!e||g.origin_x!=e->ox||g.origin_y!=e->oy||g.width!=e->w||g.height!=e->h)return false;
            std::vector<uint8_t> pixels(size_t(e->w)*e->h);if(!read(g.pixels,pixels.data(),pixels.size())||!originalPixels(*e,pixels.data(),pixels.size()))return false;}
        faceValid=true;return true;
    }
    const Entry* glyph(uint32_t node,uint32_t caller,uint32_t source,uint32_t glyphRecord,const QImageResearch& src) {
        if(!(flags&4)||!control(node)||caller!=0x4A2B1B||source!=0x4EC3C0)return nullptr;
        uint16_t fontID=0;uint32_t statePtr=0;QFontStateResearch state{};
        if(!read(node+0x5C,fontID)||fontID!=0x1407||!read(0x4EC3B8,statePtr)||!read(statePtr,state)||state.effects||state.and_mask!=255||state.xor_value||
            !validateFace(state.font)||glyphRecord<state.font+12)return nullptr;
        const auto offset=glyphRecord-state.font-12;if(offset%12||offset/12>=96)return nullptr;
        QGlyphResearch g{};const auto* e=catalog->get(2,32+offset/12);
        if(!read(glyphRecord,g)||!e||g.pixels!=src.pixels||g.origin_x!=src.origin_x||g.origin_y!=src.origin_y||g.width!=src.width||g.height!=src.height||!shape(*e,src))return nullptr;
        return e;
    }
    bool record(uint32_t node,uint32_t caller,uint32_t parentCaller,uint32_t source,uint32_t glyphRecord,uint32_t savedNode,const QImageResearch& src,int64_t x,int64_t y,dw::ui::Rect clip,const uint16_t* logical,bool isBackground) {
        if(!good||!catalog||thread!=GetCurrentThreadId())return fail("catalog or thread changed");
        std::vector<uint8_t> indices(size_t(src.width)*src.height);
        if(!read(src.pixels,indices.data(),indices.size()))return fail("source pixels unreadable");
        if(isBackground){const auto* e=catalog->get(3,0x801);
            if(background||node!=0x4E5050||!e||!shape(*e,src)||!originalPixels(*e,indices.data(),indices.size()))return fail("original map identity did not match");
            background=true;originalBackground.assign(logical,logical+800*600);return true;}
        const Entry* replacement=nullptr;
        if((flags&2)&&control(node)){
            uint16_t base=0;const auto start=control(node)->body;
            if(read(node+0x10,base)&&base==start&&caller==0x40F984&&parentCaller==0x41E328&&savedNode==node&&glyphRecord==source)
                for(unsigned i=0;i<3;++i){const auto* e=catalog->get(1,start+i);if(e&&shape(*e,src)&&originalPixels(*e,indices.data(),indices.size())){replacement=e;break;}}
        }
        if(replacement){
            if(!commands.sprite(replacement->body,{x,y,x+src.width,y+src.height},clip))return fail("body command rejected");
            ++denseBodies;receipt((uint64_t(node)<<32)|replacement->key,"body; node="+std::to_string(node)+"; resource="+std::to_string(replacement->key));return true;}
        if((flags&8)&&node==0x4E5050){
            for(const auto& f:factions){if(caller!=f.caller||x!=f.x||y!=f.y)continue;
                const auto* e=catalog->get(4,f.key);if(!e||!shape(*e,src)||!originalPixels(*e,indices.data(),indices.size()))continue;
                if(!commands.sprite(e->body,{x,y,x+src.width,y+src.height},clip))return fail("faction command rejected");
                ++denseFactions;receipt((uint64_t(node)<<32)|f.key,"faction; node="+std::to_string(node)+"; resource="+std::to_string(f.key)+"; original anchor="+std::to_string(f.x)+","+std::to_string(f.y));return true;
            }
        }
        replacement=glyph(node,caller,source,glyphRecord,src);
        if(replacement&&originalPixels(*replacement,indices.data(),indices.size())){
            uint32_t palette=0;std::array<uint16_t,256> ramp{};
            if(read(0x4EC390,palette)&&read(palette,ramp)){
                auto art=colorize(*replacement,ramp);
                if(!commands.sprite(art,{x,y,x+src.width,y+src.height},clip))return fail("glyph command rejected");
                ++denseGlyphs;receipt((uint64_t(node)<<32)|0x1407,"font014/007; node="+std::to_string(node)+"; original metrics/glyph-record/content verified; live RGB565 ramp + fractional coverage");return true;}
        }
        if((flags&4)&&caller==0x4A2B1B)receipt((uint64_t(node)<<32)|0xFFFF,"original glyph retained; node="+std::to_string(node)+"; source="+std::to_string(source)+"; record="+std::to_string(glyphRecord)+"; join/style/content unsupported");
        ++rawFallbacks;
        return good=commands.raw(logical,indices.data(),indices.size(),src.width,src.height,x,y,(src.flags&2)!=0,false,clip);
    }
    bool compose(const MenuCanvas::Plan& plan,MenuCanvas::WriteView dst){
        if(!good||!background||originalBackground.size()!=800*600)return false;
        // All optional composition is prepared in owned memory. Failure leaves
        // the physical target untouched and the caller replays its original fit.
        std::vector<uint16_t> output(size_t(dst.width)*dst.height);
        MenuCanvas::WriteView work{output.data(),output.size()*2,size_t(dst.width)*2,dst.width,dst.height};
        bool painted=false;
        if(flags&1){const auto core=plan.geometry().content_bounds();
            if(fittedW!=dst.width||fittedH!=dst.height||fittedCore.left!=core.left||fittedCore.top!=core.top||fittedCore.right!=core.right||fittedCore.bottom!=core.bottom){
                std::vector<uint16_t> next(output.size());MenuCanvas::WriteView mapped{next.data(),next.size()*2,size_t(dst.width)*2,dst.width,dst.height};
                if(!map||!composeMap(*map,plan.geometry(),mapped))return fail("map fit rejected");
                fittedMap=std::move(next);fittedW=dst.width;fittedH=dst.height;fittedCore=core;
            }
            if(fittedMap.size()!=output.size())return fail("map cache size changed");
            std::copy(fittedMap.begin(),fittedMap.end(),output.begin());painted=true;
        }else painted=plan.compose({originalBackground.data(),800*600*2,1600,800,600},work);
        if(!painted||!commands.compose(plan.geometry(),work))return false;
        if(!MenuCanvas::validView(dst.pixels,dst.bytes,dst.pitch,dst.width,dst.height))return false;
        for(unsigned y=0;y<dst.height;++y)memcpy(static_cast<uint8_t*>(dst.pixels)+size_t(y)*dst.pitch,output.data()+size_t(y)*dst.width,size_t(dst.width)*2);
        ++composed;receipt(1,"composition; output="+std::to_string(dst.width)+"x"+std::to_string(dst.height)+"; exact original map resource + ordered foreground");return true;
    }
};
inline std::unique_ptr<Layer> begin(){return flags&&catalog&&GetCurrentThreadId()==thread?std::make_unique<Layer>():nullptr;}
inline void finish()noexcept{try{if(flags)Remaster::note("selection_restore_end","passes="+std::to_string(passes)+"; composed="+std::to_string(composed)+"; bodies="+std::to_string(denseBodies)+"; glyphs="+std::to_string(denseGlyphs)+"; factions="+std::to_string(denseFactions)+"; originalOps="+std::to_string(rawFallbacks)+"; fallbackFrames="+std::to_string(failed));}catch(...){}}
}
