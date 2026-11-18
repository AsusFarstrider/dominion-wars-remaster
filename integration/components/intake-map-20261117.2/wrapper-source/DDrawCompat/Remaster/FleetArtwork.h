#pragma once
#include "FleetPins.h"
#include "FleetLayout.h"
#include <Research/MenuCanvas/MainBackdrop.h>
#include <Research/MenuProbe/Core.h>
#include <set>

namespace FleetArtwork {
using Ramp = std::array<uint8_t, 512>;
using Fingerprint = std::array<uint8_t, 32>;

#ifdef OPENING_FLOW_TEST
inline unsigned paletteHashes = 0;
#endif

struct Entry {
    uint32_t key=0,palette=0;uint16_t flags=0,w=0,h=0;int16_t ox=0,oy=0;
    std::array<uint8_t,32> identity{};Remaster::SpriteFrame frame;
};
struct Assets {
    struct Palette {uint32_t key=0;Fingerprint fingerprint{};};
    std::vector<Entry> entries;std::vector<Palette> palettes;
    unsigned targetW=0,targetH=0,logicalW=0;
    std::vector<uint16_t> background,scratch;
    static std::shared_ptr<Assets> parse(std::vector<uint8_t> bytes) {
        if(bytes.size()<16||bytes.size()>64*1024*1024||memcmp(bytes.data(),"DWFLEET2",8))return {};
        auto storage=std::make_shared<std::vector<uint8_t>>(std::move(bytes));size_t pos=8;
        const auto take=[&](void* dst,size_t n){if(n>storage->size()-pos)return false;memcpy(dst,storage->data()+pos,n);pos+=n;return true;};
        uint32_t count=0,paletteCount=0;if(!take(&count,4)||!take(&paletteCount,4)||count!=139||paletteCount!=2)return {};
        auto out=std::make_shared<Assets>();std::set<uint32_t> seenPalettes;std::set<uint64_t> seen;
        std::set<Fingerprint> seenFingerprints;
        for(unsigned i=0;i<paletteCount;++i){Palette p;if(!take(&p.key,4)||!take(p.fingerprint.data(),p.fingerprint.size())||
            (p.key!=0xB200&&p.key!=0xB272)||!seenPalettes.insert(p.key).second||
            p.fingerprint==Fingerprint{}||!seenFingerprints.insert(p.fingerprint).second)return {};out->palettes.push_back(p);}
        for(unsigned i=0;i<count;++i) {
            Entry e;uint32_t pw=0,ph=0;
            if(!take(&e.key,4)||!take(&e.palette,4)||!take(&e.flags,2)||!take(&e.ox,2)||!take(&e.oy,2)||
                !take(&e.w,2)||!take(&e.h,2)||!take(&pw,4)||!take(&ph,4)||!take(e.identity.data(),32))return {};
            if(!((e.key>=0xB201&&e.key<=0xB271)||(e.key>=0xB276&&e.key<=0xB278))||
                !seenPalettes.count(e.palette)||!seen.insert((uint64_t(e.key)<<32)|e.palette).second||
                (e.flags!=1&&e.flags!=3)||!e.w||!e.h||e.w>800||e.h>600||pw!=e.w*4u||ph!=e.h*4u)return {};
            const uint64_t n=uint64_t(pw)*ph*4;if(n>storage->size()-pos)return {};
            e.frame={storage,pos,pw,ph};pos+=size_t(n);out->entries.push_back(e);
        }
        if(pos!=storage->size())return {};
        return out;
    }
    std::optional<uint32_t> paletteKey(const Ramp& live)const noexcept {
        Fingerprint digest{};
#ifdef OPENING_FLOW_TEST
        ++paletteHashes;
#endif
        if(BCryptHash(BCRYPT_SHA256_ALG_HANDLE,nullptr,0,const_cast<PUCHAR>(live.data()),ULONG(live.size()),
            digest.data(),ULONG(digest.size()))<0)return {};
        for(const auto& p:palettes)if(p.fingerprint==digest)return p.key;
        return 0;
    }
    const Entry* identify(const QImageResearch& src,uint32_t palette,bool* ambiguous=nullptr)const {
        if(ambiguous)*ambiguous=false;
        if(!palette||src.pixel_bits!=8||src.surface||!src.pixels||src.and_mask!=255||src.xor_value)return nullptr;
        const auto shape=[&](const Entry& e){return e.palette==palette&&e.flags==src.flags&&e.ox==src.origin_x&&e.oy==src.origin_y&&e.w==src.width&&e.h==src.height;};
        if(std::none_of(entries.begin(),entries.end(),shape))return nullptr;
        std::vector<uint8_t> pixels(size_t(src.width)*src.height);
        if(!MainBackdrop::read(src.pixels,pixels.data(),pixels.size()))return nullptr;
        std::array<uint8_t,32> hash{};
        if(BCryptHash(BCRYPT_SHA256_ALG_HANDLE,nullptr,0,pixels.data(),ULONG(pixels.size()),hash.data(),32)<0)return nullptr;
        const Entry* found=nullptr;
        for(const auto& e:entries)if(shape(e)&&e.identity==hash){
            if(found){if(ambiguous)*ambiguous=true;return nullptr;}found=&e;
        }
        return found;
    }
    bool prepare(const MenuCanvas::Plan& plan,unsigned w,unsigned h,Layout layout) {
        if(w==targetW&&h==targetH&&layout.width==logicalW&&!background.empty())return true;
        const Entry* bg=nullptr;for(const auto& e:entries)if(e.key==0xB201&&e.palette==0xB200){bg=&e;break;}
        if(!bg||bg->w!=800||bg->h!=600)return false;
        std::vector<uint16_t> next(size_t(w)*h);
        if (layout.extra()) {
            const auto bounds = plan.geometry().content_bounds();
            const double scaleX = double(layout.width) / (bounds.right - bounds.left);
            const double scaleY = 600.0 / (bounds.bottom - bounds.top);
            for (auto y = bounds.top; y < bounds.bottom; ++y) {
                const auto sourceY = unsigned((y - bounds.top + 0.5) * scaleY * 4);
                for (auto x = bounds.left; x < bounds.right; ++x) {
                    const auto sourceX = unsigned(layout.backgroundX((x - bounds.left + 0.5) * scaleX) * 4);
                    const auto* p = bg->frame.pixels() + (size_t(sourceY) * 3200 + sourceX) * 4;
                    next[size_t(y) * w + size_t(x)] = uint16_t((p[0] >> 3) << 11 | (p[1] >> 2) << 5 | (p[2] >> 3));
                }
            }
        } else {
            Remaster::OrderedPaint painter;
            if(!painter.sprite(bg->frame,{0,0,800,600},{0,0,800,600})||
                !painter.compose(plan.geometry(),{next.data(),next.size()*2,size_t(w)*2,w,h}))return false;
        }
        scratch.resize(size_t(w)*h);
        background=std::move(next);targetW=w;targetH=h;logicalW=layout.width;return true;
    }
};
class PaletteCache {
    struct Match {Ramp live{};uint32_t key=0;};
    std::array<Match,4> entries{};
    size_t used=0,next=0;
public:
    uint32_t readKey(const Assets& catalog,uint32_t address) noexcept {
        Ramp live{};
        if(!MainBackdrop::read(address,live.data(),live.size()))return 0;
        for(size_t i=0;i<used;++i)
            if(entries[i].live==live)return entries[i].key;
        const auto key=catalog.paletteKey(live);
        if(!key)return 0;
        entries[next]={live,*key};
        next=(next+1)%entries.size();
        if(used<entries.size())++used;
        return *key;
    }
};

inline bool exactBank(const std::vector<uint8_t>& bytes) noexcept {
    if(bytes.empty()||bytes.size()>64*1024*1024)return false;
    Fingerprint digest{};
    if(BCryptHash(BCRYPT_SHA256_ALG_HANDLE,nullptr,0,const_cast<PUCHAR>(bytes.data()),ULONG(bytes.size()),
        digest.data(),ULONG(digest.size()))<0)return false;
    constexpr char hex[]="0123456789abcdef";
    for(size_t i=0;i<digest.size();++i)
        if(assetHash[2*i]!=hex[digest[i]>>4]||assetHash[2*i+1]!=hex[digest[i]&15])return false;
    return true;
}

inline std::shared_ptr<Assets> assets;
inline DWORD owner=0;
inline std::set<std::string> reported;
inline std::set<uint64_t> drawn;
inline std::string coverage;
inline unsigned pendingCoverage=0;
inline uint64_t composed=0,fallbacks=0,diagnosticOmissions=0;
inline void note(const char* kind,const std::string& detail)noexcept {
    try{const auto key=std::string(kind)+detail;if(reported.count(key))return;
        if(reported.size()>=96){++diagnosticOmissions;return;}
        reported.insert(key);Remaster::note(kind,detail);
    }catch(...){++diagnosticOmissions;}
}
inline void flushCoverage()noexcept {if(!pendingCoverage)return;note("fleet_art_coverage",coverage);coverage.clear();pendingCoverage=0;}
inline void recordDraw(const Entry& e)noexcept {
    try{if(drawn.insert((uint64_t(e.key)<<32)|e.palette).second){
        coverage+=std::to_string(e.key)+"@"+std::to_string(e.palette)+";";
        if(++pendingCoverage==8)flushCoverage();}}
    catch(...){++diagnosticOmissions;}
}
inline void finish()noexcept {
    try{if(!assets)return;flushCoverage();
        note("fleet_art_end","commits="+std::to_string(composed)+"; fallback_frames="+std::to_string(fallbacks)+
            "; drawn_variants="+std::to_string(drawn.size())+"; diagnostic_omissions="+std::to_string(diagnosticOmissions));
    }catch(...){++diagnosticOmissions;}
}
inline bool admitted()noexcept {
    return assets&&owner==GetCurrentThreadId()&&MainBackdrop::value<uint32_t>(0x71386C)==0;
}
inline int controlShift(const Layout& layout, const MenuProbe::Node& control) noexcept {
    return layout.shift(control.x, control.y, control.id, MainBackdrop::value<uint8_t>(0x7606F8));
}
struct Layer {
    MainBackdrop::Layer shadow;
    Layout layout;
    PaletteCache paletteCache;
    uint32_t lastOwner=0;
    std::optional<int> ownerShift;
    std::optional<int> fontShift;
    bool background=false,good=true;
    unsigned replacements=0;

    explicit Layer(MainBackdrop::Target target,Layout geometry={}):shadow(target,0),layout(geometry){
        shadow.commands = Remaster::OrderedPaint(layout.width);
        shadow.ordered=true;shadow.reportErrors=false;
    }

    std::optional<int> controlOffset(uint32_t node) {
        if (node == lastOwner) return ownerShift;
        lastOwner = node;
        ownerShift.reset();
        if (!node || node == 0x4E3378) return {};
        const auto chain = MenuProbe::ancestry(MainBackdrop::read, node);
        if (!chain.gap.empty()) return {};
        for (size_t i = 1; i < chain.nodes.size(); ++i) {
            if (chain.nodes[i].address != 0x4E3378) continue;
            const auto& control = chain.nodes[i - 1];
            ownerShift = controlShift(layout, control);
            break;
        }
        return ownerShift;
    }

    void font(uint32_t node, uint32_t caller) {
        if (!layout.extra()) return;
        if (node == 0x4E3378 && Layout::ownsPreview(caller)) {
            fontShift = 0;
            return;
        }
        fontShift = controlOffset(node);
        if (fontShift) return;
        QFontStateResearch fontState{};
        if (MainBackdrop::read(MainBackdrop::value<uint32_t>(0x4EC3B8), &fontState, sizeof(fontState)))
            fontShift = layout.shift(fontState.pen_x, fontState.pen_y);
    }
    void fail(const char* why){if(good){++fallbacks;note("fleet_art_fallback",why);}good=false;}
    void mirror(const uint32_t* args,void* original,uint32_t node) {
        if(!good)return;if(!admitted()){fail("owner changed");return;}
        QImageResearch src{};if(!MainBackdrop::read(args[1],&src,sizeof(src))){fail("source unreadable");return;}
        const auto paletteKey=paletteCache.readKey(*assets,MainBackdrop::value<uint32_t>(0x4EC390));
        bool ambiguous=false;const auto* e=assets->identify(src,paletteKey,&ambiguous);
        if(ambiguous)note("fleet_art_alias_retained","same original identity; original pixels retained");
        if(e&&e->key==0xB201) {
            if(background||node!=0x4E3378||args[0]!=0x46F0DF||args[3]||args[4]||e->palette!=0xB200){fail("background ownership");return;}
            background=true;recordDraw(*e);return;
        }
        if(!background){
            if(node==0x4E3378&&args[0]==0x46F0DF)
                note("fleet_art_background_unmatched","bits="+std::to_string(src.pixel_bits)+"; flags="+std::to_string(src.flags)+"; palette="+std::to_string(paletteKey));
            fail("background identity or palette not admitted");return;
        }
        // Original CPU conversion and ordered masks are the same mechanism as
        // Briefing. Untargeted content is retained, never globally overridden.
        const auto id=MainBackdrop::value<uint32_t>(node);
        const int x = int32_t(args[3]) + src.origin_x;
        const int y = int32_t(args[4]) + src.origin_y;
        int offset = 0;
        if (layout.extra()) {
            if (node != 0x4E3378 || !Layout::ownsPreview(args[0])) {
                const auto anchored = args[0] == 0x4A2B1B ? fontShift : controlOffset(node);
                offset = anchored.value_or(layout.shift(x, y));
            }
        }
        const bool nativePortrait=src.flags==1&&src.pixel_bits==16&&!src.surface&&src.and_mask==255&&!src.xor_value&&
            ((node==0x4E3378&&(args[0]==0x470B27||args[0]==0x470C7B||args[0]==0x470CE1))||
             ((id>=0x41F&&id<=0x424)&&(args[0]==0x46EB44||args[0]==0x46EC6B)));
        if(MainBackdrop::mirror(shadow,args,original,node,e==nullptr,nativePortrait,offset)==MainBackdrop::Mirror::failed){
            note("fleet_art_producer_refused","node="+std::to_string(node)+"; caller="+std::to_string(args[0])+
                "; flags="+std::to_string(src.flags)+"; bits="+std::to_string(src.pixel_bits)+"; w="+std::to_string(src.width)+"; h="+std::to_string(src.height));
            fail("unsupported producer");return;
        }
        if(nativePortrait)note("fleet_original_native_image","node="+std::to_string(node)+"; caller="+std::to_string(args[0]));
        if(!e)return;
        std::array<int16_t,4> clip{};const auto cp=MainBackdrop::value<uint32_t>(0xA39B78);
        if(!MainBackdrop::read(cp?cp:shadow.target.outer+0x46,clip.data(),sizeof(clip))){fail("clip unreadable");return;}
        const auto originalBounds = (src.flags&1) ?
            dw::ui::intersect({0,0,800,600},{clip[0],clip[1],int(clip[0])+clip[2],int(clip[1])+clip[3]}) :
            std::optional<dw::ui::Rect>{{0,0,800,600}};
        if (!originalBounds) return;
        const dw::ui::Rect bounds{originalBounds->left+offset,originalBounds->top,originalBounds->right+offset,originalBounds->bottom};
        if(!shadow.commands.sprite(e->frame,{x+offset,y,x+offset+e->w,y+e->h},bounds)){fail("sprite bounds");return;}
        ++replacements;recordDraw(*e);
    }
    // Compose into owned staging first. Any refusal leaves the live target
    // untouched so the established completed-screen fit remains a safe fallback.
    bool compose(const MenuCanvas::Plan& plan,MenuCanvas::WriteView dst) {
        if(!good||!background||!shadow.intact()||!admitted()||!assets->prepare(plan,dst.width,dst.height,layout))return false;
        memcpy(assets->scratch.data(),assets->background.data(),assets->background.size()*2);
        MenuCanvas::WriteView stage{assets->scratch.data(),assets->scratch.size()*2,size_t(dst.width)*2,dst.width,dst.height};
        if(!shadow.commands.compose(plan.geometry(),stage))return false;
        for(unsigned y=0;y<dst.height;++y)memcpy(static_cast<uint8_t*>(dst.pixels)+size_t(y)*dst.pitch,
            assets->scratch.data()+size_t(y)*dst.width,size_t(dst.width)*2);
        ++composed;note("fleet_art_commit","width="+std::to_string(dst.width)+"; height="+std::to_string(dst.height)+"; logical_width="+std::to_string(layout.width));return true;
    }
};
inline std::shared_ptr<Layer> begin(MainBackdrop::Target target){
    return admitted()?std::make_shared<Layer>(target,Layout::forOutput(target.w,target.h)):nullptr;
}
inline void install() {
    if(!Remaster::enabled())return;
    wchar_t exe[32768]{};if(!GetModuleFileNameW(nullptr,exe,32768))return;
    const auto path=std::filesystem::path(exe).parent_path()/L"rs/Remaster/Fleet/fleet.dwbank";
    if(!std::filesystem::exists(path))return;
    std::ifstream in(path,std::ios::binary|std::ios::ate);
    if(!in||in.tellg()<=0||uint64_t(in.tellg())>64*1024*1024)throw std::runtime_error("Fleet asset size");
    std::vector<uint8_t> bytes(size_t(in.tellg()));in.seekg(0);
    if(!in.read(reinterpret_cast<char*>(bytes.data()),bytes.size()))throw std::runtime_error("Fleet asset read");
    if(!exactBank(bytes))throw std::runtime_error("Fleet asset identity");
    auto parsed=Assets::parse(std::move(bytes));if(!parsed)throw std::runtime_error("Fleet asset format");
    assets=std::move(parsed);owner=GetCurrentThreadId();note("fleet_art_ready",assetHash);
}
}
