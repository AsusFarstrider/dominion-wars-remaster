#pragma once
// Source-resolution history for the bounded 5C5 HD research profile.
// No native Lock pointer is retained here. A skipped conversion cannot seed it.
#include "OpeningCore.h"
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <memory>
namespace OpeningFlow {
struct HdHistory {
    static constexpr size_t guard=64;
    static constexpr uint16_t sentinel=0xA55A;
    unsigned width,height,pitch;
    std::vector<uint16_t> pixels;
    bool initialized=false,quarantined=false;
    uint64_t revision=0;
    std::unique_ptr<MenuCanvas::Plan> outputPlan;
    unsigned outputWidth=0,outputHeight=0;
    HdHistory(unsigned w,unsigned h):width(w),height(h),pitch(w*2) {
        if(!MenuCanvas::validSize(w,h)||w%4||h%2||uint64_t(w)*h>8ull*1024*1024)
            throw std::invalid_argument("HD history extent");
        pixels.assign(size_t(w)*h+2*guard,sentinel);
        std::fill(pixels.begin()+guard,pixels.end()-guard,uint16_t{});
    }
    void* data() noexcept {return pixels.data()+guard;}
    bool intact() const noexcept {
        return std::all_of(pixels.begin(),pixels.begin()+guard,[](auto v){return v==sentinel;})&&
            std::all_of(pixels.end()-guard,pixels.end(),[](auto v){return v==sentinel;});
    }
    void reset() noexcept {initialized=false;++revision;}
    unsigned copyFlags() const noexcept {return initialized?10u:0x8000000Au;}
    bool converted(int result) noexcept {
        if(!intact()||(result!=0&&result!=1)){quarantined=true;initialized=false;return false;}
        if(result==0)initialized=true;
        return initialized&&!quarantined;
    }
    bool identity(MenuCanvas::WriteView dst) const noexcept {
        if(!initialized||quarantined||!intact()||dst.width!=width||dst.height!=height||
           !MenuCanvas::validView(dst.pixels,dst.bytes,dst.pitch,width,height))return false;
        const auto a=reinterpret_cast<uintptr_t>(pixels.data()+guard),b=reinterpret_cast<uintptr_t>(dst.pixels);
        if(a<b+(height-1)*dst.pitch+width*2&&b<a+size_t(pitch)*height)return false;
        for(unsigned y=0;y<height;++y)std::memcpy(static_cast<uint8_t*>(dst.pixels)+y*dst.pitch,
            pixels.data()+guard+size_t(y)*width,pitch);
        return true;
    }
    bool present(MenuCanvas::WriteView dst) {
        if(dst.width==width&&dst.height==height)return identity(dst);
        if(!initialized||quarantined||!intact()||!MenuCanvas::validView(dst.pixels,dst.bytes,dst.pitch,dst.width,dst.height))return false;
        if(!outputPlan||outputWidth!=dst.width||outputHeight!=dst.height){
            auto next=MenuCanvas::Plan::fit(width,height,dst.width,dst.height);if(!next)return false;
            outputPlan=std::make_unique<MenuCanvas::Plan>(std::move(*next));outputWidth=dst.width;outputHeight=dst.height;
        }
        return outputPlan->compose({pixels.data()+guard,size_t(pitch)*height,pitch,width,height},dst)&&intact();
    }
};
}
