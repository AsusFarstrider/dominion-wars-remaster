#pragma once
#include "ui_canvas.hpp"
#include <algorithm>
#include <cstdint>
#include <vector>
namespace MenuCanvas {
// P8 source-index coverage, before palette lookup/AND/XOR. This contains no
// game pointers. The original primitive remains the authority for RGB565 color.
class ForegroundMask {
    unsigned w_,h_;
    std::vector<uint8_t> pixels_;
public:
    ForegroundMask(unsigned w,unsigned h):w_(w),h_(h),pixels_(size_t(w)*h){}
    const uint8_t* data()const noexcept{return pixels_.data();}
    size_t bytes()const noexcept{return pixels_.size();}
    size_t covered()const noexcept{return size_t(std::count(pixels_.begin(),pixels_.end(),uint8_t{1}));}
    bool mark(const uint8_t* source,size_t bytes,unsigned sw,unsigned sh,int64_t x,int64_t y,
        bool keyed,bool solid,const dw::ui::Rect& clip) noexcept {
        if(!sw||!sh||(!solid&&(!source||uint64_t(sw)*sh>bytes)))return false;
        const auto target=dw::ui::intersect({0,0,w_,h_},clip);if(!target)return true;
        const auto region=dw::ui::intersect(*target,{x,y,x+sw,y+sh});if(!region)return true;
        for(auto row=region->top;row<region->bottom;++row)for(auto col=region->left;col<region->right;++col)
            if(solid||!keyed||source[size_t(row-y)*sw+size_t(col-x)]!=0)pixels_[size_t(row)*w_+size_t(col)]=1;
        return true;
    }
};
}
