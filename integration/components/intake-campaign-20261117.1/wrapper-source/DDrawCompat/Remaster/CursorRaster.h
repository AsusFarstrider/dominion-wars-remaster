#pragma once
#include <Remaster/OrderedPaint.h>
#include <array>
namespace CMainCursor {
inline constexpr unsigned physicalSize=44;
struct Art {
    std::array<Remaster::SpriteFrame,2> frames;
    static std::shared_ptr<Art> parse(std::vector<uint8_t> bytes){
        constexpr size_t frameBytes=88*88*4,total=16+2*(4+frameBytes);
        if(bytes.size()!=total||memcmp(bytes.data(),"DWCMCUR1",8))return {};
        uint32_t size=0,count=0;memcpy(&size,bytes.data()+8,4);memcpy(&count,bytes.data()+12,4);if(size!=88||count!=2)return {};
        auto a=std::make_shared<Art>();auto owned=std::make_shared<const std::vector<uint8_t>>(std::move(bytes));
        for(unsigned i=0;i<2;++i){const auto p=16+size_t(i)*(4+frameBytes);uint32_t id=0;memcpy(&id,owned->data()+p,4);
            if(id!=(i?0xC087u:0xAF87u))return {};a->frames[i]={owned,p+4,88,88};}
        return a;
    }
};
inline uint8_t channel(uint16_t v,unsigned n){return n==0?uint8_t(((v>>11)<<3)|(v>>13)):n==1?uint8_t((((v>>5)&63)<<2)|((v>>9)&3)):uint8_t(((v&31)<<3)|((v&31)>>2));}
inline uint16_t blend(const Remaster::SpriteFrame& f,unsigned x,unsigned y,uint16_t old)noexcept{
    const double sx=(x+0.5)*f.width/physicalSize-0.5,sy=(y+0.5)*f.height/physicalSize-0.5;
    const auto x0=unsigned(std::clamp(sx,0.0,double(f.width-1))),y0=unsigned(std::clamp(sy,0.0,double(f.height-1)));
    const auto x1=std::min(x0+1,f.width-1),y1=std::min(y0+1,f.height-1);const double tx=sx-x0,ty=sy-y0;
    const double weights[]={(1-tx)*(1-ty),tx*(1-ty),(1-tx)*ty,tx*ty};
    const size_t at[]={size_t(y0)*f.width+x0,size_t(y0)*f.width+x1,size_t(y1)*f.width+x0,size_t(y1)*f.width+x1};
    double a=0,c[3]{};const auto* pixels=f.pixels();
    for(unsigned i=0;i<4;++i){const auto* p=pixels+at[i]*4;const auto alpha=p[3]/255.0;a+=weights[i]*alpha;for(unsigned n=0;n<3;++n)c[n]+=weights[i]*alpha*p[n];}
    if(a<=0)return old;unsigned rgb[3]{};
    for(unsigned n=0;n<3;++n)rgb[n]=unsigned(std::clamp(std::floor(c[n]+(1-a)*channel(old,n)+0.5),0.0,255.0));
    return uint16_t((rgb[0]>>3)<<11|(rgb[1]>>2)<<5|(rgb[2]>>3));
}
struct Patch {
    dw::ui::Rect rect{};unsigned width=0,height=0;std::vector<uint16_t> before,after;
    bool prepare(MenuCanvas::WriteView target,const Remaster::SpriteFrame& art,int x,int y){
        if(!art.valid()||!MenuCanvas::validView(target.pixels,target.bytes,target.pitch,target.width,target.height)||x<-int(physicalSize)||y<-int(physicalSize))return false;
        auto clip=dw::ui::intersect({x,y,int64_t(x)+physicalSize,int64_t(y)+physicalSize},{0,0,target.width,target.height});if(!clip)return false;
        rect=*clip;width=target.width;height=target.height;const auto w=size_t(rect.right-rect.left),h=size_t(rect.bottom-rect.top);
        before.resize(w*h);after.resize(w*h);
        for(size_t row=0;row<h;++row)for(size_t col=0;col<w;++col){const auto xx=rect.left+col,yy=rect.top+row;uint16_t pixel=0;
            memcpy(&pixel,static_cast<uint8_t*>(target.pixels)+size_t(yy)*target.pitch+size_t(xx)*2,2);before[row*w+col]=pixel;
            after[row*w+col]=blend(art,unsigned(xx-x),unsigned(yy-y),pixel);}
        return true;
    }
    bool apply(MenuCanvas::WriteView target,bool restore=false)const noexcept{
        if(target.width!=width||target.height!=height||!MenuCanvas::validView(target.pixels,target.bytes,target.pitch,target.width,target.height))return false;
        const auto w=size_t(rect.right-rect.left),h=size_t(rect.bottom-rect.top);const auto& pixels=restore?before:after;
        if(rect.left<0||rect.top<0||rect.right>width||rect.bottom>height||!w||!h||pixels.size()!=w*h)return false;
        for(size_t y=0;y<h;++y)memcpy(static_cast<uint8_t*>(target.pixels)+size_t(rect.top+y)*target.pitch+size_t(rect.left)*2,pixels.data()+y*w,w*2);
        return true;
    }
};
}
