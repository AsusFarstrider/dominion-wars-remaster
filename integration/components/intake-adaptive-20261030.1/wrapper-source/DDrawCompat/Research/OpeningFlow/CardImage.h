#pragma once
#include <Research/MenuCanvas/Canvas565.h>
#include <memory>
#include <stdexcept>
namespace OpeningFlow {
// Private converted source artwork. The importer preserves PNG dimensions and
// quantizes once to the native565 format; this image never visits logical800.
class CardImage {
    unsigned width_=0,height_=0,targetW_=0,targetH_=0;
    std::vector<uint16_t> pixels_;
    std::unique_ptr<MenuCanvas::Plan> plan_;
public:
    static std::shared_ptr<CardImage> parse(const std::vector<unsigned char>& bytes) {
        if(bytes.size()<16||memcmp(bytes.data(),"DWCARD1\0",8))return {};
        uint32_t w=0,h=0;memcpy(&w,bytes.data()+8,4);memcpy(&h,bytes.data()+12,4);
        if(!MenuCanvas::validSize(w,h)||w>8192||h>8192||uint64_t(w)*h>16*1024*1024||
           16+uint64_t(w)*h*2!=bytes.size())return {};
        auto image=std::make_shared<CardImage>();image->width_=w;image->height_=h;
        image->pixels_.resize(size_t(w)*h);memcpy(image->pixels_.data(),bytes.data()+16,bytes.size()-16);return image;
    }
    unsigned width()const noexcept{return width_;}
    unsigned height()const noexcept{return height_;}
    const void* data()const noexcept{return pixels_.data();}
    size_t bytes()const noexcept{return pixels_.size()*2;}
    bool identity(unsigned w,unsigned h)const noexcept{return w==width_&&h==height_;}
    bool compose(MenuCanvas::WriteView dst) {
        if(!MenuCanvas::validView(dst.pixels,dst.bytes,dst.pitch,dst.width,dst.height))return false;
        const auto a=reinterpret_cast<uintptr_t>(pixels_.data()),b=reinterpret_cast<uintptr_t>(dst.pixels);
        if(a<b+(dst.height-1)*dst.pitch+dst.width*2&&b<a+bytes())return false;
        if(identity(dst.width,dst.height)) {
            for(unsigned y=0;y<height_;++y)memcpy(static_cast<unsigned char*>(dst.pixels)+size_t(y)*dst.pitch,
                pixels_.data()+size_t(y)*width_,size_t(width_)*2);
            return true;
        }
        if(!plan_||targetW_!=dst.width||targetH_!=dst.height) {
            auto p=MenuCanvas::Plan::fit(width_,height_,dst.width,dst.height);if(!p)return false;
            plan_=std::make_unique<MenuCanvas::Plan>(std::move(*p));targetW_=dst.width;targetH_=dst.height;
        }
        return plan_->compose({data(),bytes(),size_t(width_)*2,width_,height_},dst);
    }
};
}
