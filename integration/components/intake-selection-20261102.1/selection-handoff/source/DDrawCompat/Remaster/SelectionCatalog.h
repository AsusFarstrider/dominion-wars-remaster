#pragma once
#include <Remaster/OrderedPaint.h>
#include <Research/MenuCanvas/quantum_font_research.h>
#include <Research/OpeningFlow/CardImage.h>
#include <array>
#include <set>
#include <Windows.h>
#include <bcrypt.h>

namespace SelectionRestoration {
struct Entry {
    uint32_t kind=0,key=0;int16_t ox=0,oy=0;uint16_t w=0,h=0;unsigned pw=0,ph=0;
    std::array<uint8_t,32> identity{},classes{};
    Remaster::SpriteFrame body;
    std::vector<uint8_t> indexedCoverage;
    mutable Remaster::SpriteFrame colored;
    mutable std::array<uint16_t,256> ramp{};
    mutable bool hasRamp=false;
};
struct Catalog {
    std::vector<Entry> entries;
    static std::shared_ptr<Catalog> parse(std::vector<uint8_t> bytes) {
        if(bytes.size()<12||bytes.size()>32*1024*1024||memcmp(bytes.data(),"DWSSEL2\0",8))return {};
        size_t pos=8;auto take=[&](void* out,size_t n){if(n>bytes.size()-pos)return false;memcpy(out,bytes.data()+pos,n);pos+=n;return true;};
        uint32_t count=0;if(!take(&count,4)||count!=114)return {};
        auto c=std::make_shared<Catalog>();std::set<std::pair<uint32_t,uint32_t>> keys;
        for(uint32_t i=0;i<count;++i){Entry e;
            if(!take(&e.kind,4)||!take(&e.key,4)||!take(&e.ox,2)||!take(&e.oy,2)||!take(&e.w,2)||!take(&e.h,2)||!take(&e.pw,4)||!take(&e.ph,4))return {};
            if(!keys.insert({e.kind,e.key}).second||!e.w||!e.h||e.w>800||e.h>600)return {};
            if(e.kind==3){if(e.key!=0x801||e.w!=800||e.h!=600||e.ox||e.oy||e.pw!=800||e.ph!=600)return {};}
            else if(e.pw!=e.w*4u||e.ph!=e.h*4u)return {};
            if(e.kind==1){if(!((e.key>=0x802&&e.key<=0x804)||(e.key>=0x12&&e.key<=0x17))||e.h!=32||
                e.w!=(e.key>=0x802?570:e.key<0x15?140:180)||e.ox||e.oy)return {};}
            else if(e.kind==4){if(e.key<0x809||e.key>0x810||e.ox||e.oy)return {};
                const unsigned expectedWidths[]={82,82,101,101,153,153,95,95};
                if(e.w!=expectedWidths[e.key-0x809]||e.h!=(e.key<0x80B?21:22))return {};}
            else if(e.kind==2){if(e.key<32||e.key>127||e.w>17||e.h>19||e.ox<0||e.ox>17||e.oy<0||e.oy>24)return {};}
            else if(e.kind!=3)return {};
            if(!take(e.identity.data(),e.identity.size())||!take(e.classes.data(),e.classes.size()))return {};
            if(std::all_of(e.identity.begin(),e.identity.end(),[](uint8_t v){return !v;}))return {};
            if(e.classes[0]&1)return {}; // Index0 is transparent, never a covered glyph class.
            if(e.kind!=2&&std::any_of(e.classes.begin(),e.classes.end(),[](uint8_t v){return v!=0;}))return {};
            if(e.kind==3){c->entries.push_back(std::move(e));continue;}
            const auto n=size_t(e.pw)*e.ph*(e.kind==2?2:4);if(n>bytes.size()-pos)return {};
            if(e.kind==1||e.kind==4){auto p=std::make_shared<std::vector<uint8_t>>(n);if(!take(p->data(),n))return {};e.body={p,0,e.pw,e.ph};}
            else {e.indexedCoverage.resize(n);if(!take(e.indexedCoverage.data(),n))return {};
                for(size_t k=0;k<n;k+=2)if(e.indexedCoverage[k+1]&&(e.indexedCoverage[k]==0||!(e.classes[e.indexedCoverage[k]/8]&(1u<<(e.indexedCoverage[k]%8)))))return {};}
            c->entries.push_back(std::move(e));
        }
        if(pos!=bytes.size())return {};return c;
    }
    const Entry* get(uint32_t kind,uint32_t key)const noexcept {
        for(const auto& e:entries)if(e.kind==kind&&e.key==key)return &e;return nullptr;
    }
};
inline bool shape(const Entry& e,const QImageResearch& src)noexcept {
    return src.origin_x==e.ox&&src.origin_y==e.oy&&src.width==e.w&&src.height==e.h;
}
inline bool originalPixels(const Entry& e,const uint8_t* pixels,size_t size)noexcept {
    if(!pixels||size!=size_t(e.w)*e.h||!size||size>800*600)return false;
    std::array<uint8_t,32> actual{};
    return BCryptHash(BCRYPT_SHA256_ALG_HANDLE,nullptr,0,const_cast<PUCHAR>(pixels),ULONG(size),actual.data(),ULONG(actual.size()))>=0&&actual==e.identity;
}
inline Remaster::SpriteFrame colorize(const Entry& e,const std::array<uint16_t,256>& ramp) {
    if(e.kind!=2)return {};
    if(e.hasRamp&&e.ramp==ramp)return e.colored;
    auto p=std::make_shared<std::vector<uint8_t>>(size_t(e.pw)*e.ph*4);
    for(size_t i=0;i<size_t(e.pw)*e.ph;++i){const auto value=ramp[e.indexedCoverage[i*2]];
        (*p)[i*4]=uint8_t(((value>>11)<<3)|(value>>13));
        (*p)[i*4+1]=uint8_t((((value>>5)&63)<<2)|((value>>9)&3));
        (*p)[i*4+2]=uint8_t(((value&31)<<3)|((value&31)>>2));
        (*p)[i*4+3]=e.indexedCoverage[i*2+1];}
    e.ramp=ramp;e.hasRamp=true;e.colored={p,0,e.pw,e.ph};return e.colored;
}
// The authored map is a wider canvas around the original UI core. Align that
// core to the authoritative adaptive UI rectangle; crop wings on narrow outputs
// and bound the artwork on wider outputs. Never fit the core twice.
inline bool composeMap(const OpeningFlow::CardImage& map,const dw::ui::Canvas& ui,MenuCanvas::WriteView dst)noexcept {
    if(!MenuCanvas::validView(dst.pixels,dst.bytes,dst.pitch,dst.width,dst.height)||map.width()!=2560||map.height()!=1440)return false;
    const auto core=ui.content_bounds();const double sx=double(core.right-core.left)/1920.0,sy=double(core.bottom-core.top)/1440.0;
    if(sx<=0||sy<=0)return false;
    const auto* source=static_cast<const uint16_t*>(map.data());
    for(unsigned y=0;y<dst.height;++y){auto* row=static_cast<uint8_t*>(dst.pixels)+size_t(y)*dst.pitch;
        const auto yy=int64_t(std::floor((double(y)+0.5-double(core.top))/sy));
        for(unsigned x=0;x<dst.width;++x){const auto xx=int64_t(std::floor((double(x)+0.5-double(core.left))/sx+320.0));
            const uint16_t value=xx>=0&&xx<2560&&yy>=0&&yy<1440?source[size_t(yy)*2560+size_t(xx)]:0;
            memcpy(row+size_t(x)*2,&value,2);}}
    return true;
}
}
