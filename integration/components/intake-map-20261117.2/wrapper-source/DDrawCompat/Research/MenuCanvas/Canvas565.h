#pragma once
// Research composition control. This deliberately scales a logical RGB565 image;
// it is not the high-density artwork/font rendering path.
#include "ui_canvas.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <optional>
#include <vector>

namespace MenuCanvas {
struct ReadView {
    const void* pixels = nullptr;
    size_t bytes = 0, pitch = 0;
    uint32_t width = 0, height = 0;
};
struct WriteView {
    void* pixels = nullptr;
    size_t bytes = 0, pitch = 0;
    uint32_t width = 0, height = 0;
};
// Explicit research allocation/coordinate limits, not a list of display modes.
inline constexpr uint32_t extentLimit = 16384;
inline constexpr uint64_t pixelLimit = 64ull * 1024 * 1024;
inline bool validSize(uint32_t w, uint32_t h) noexcept {
    return w && h && w <= extentLimit && h <= extentLimit && uint64_t(w)*h <= pixelLimit;
}
inline bool validView(const void* p, size_t bytes, size_t pitch, uint32_t w, uint32_t h) noexcept {
    if (!p || !validSize(w,h) || pitch < size_t(w)*2 || pitch % 2) return false;
    const size_t row = size_t(w)*2;
    if (bytes < row || (h > 1 && pitch > (bytes-row)/(h-1))) return false;
    const size_t needed = size_t(h-1)*pitch + row;
    const auto address = reinterpret_cast<uintptr_t>(p);
    return needed <= bytes && needed <= std::numeric_limits<uintptr_t>::max()-address;
}

class Plan {
    dw::ui::Canvas canvas_;
    std::vector<uint32_t> columns_, rows_;
    uint32_t sw_, sh_, dw_, dh_;
    Plan(dw::ui::Canvas c, uint32_t sw, uint32_t sh, uint32_t dw, uint32_t dh)
        : canvas_(c), sw_(sw), sh_(sh), dw_(dw), dh_(dh) {}
public:
    static std::optional<Plan> fit(uint32_t sw, uint32_t sh, uint32_t dw, uint32_t dh) {
        if (!validSize(sw,sh) || !validSize(dw,dh)) return std::nullopt;
        auto c = dw::ui::Canvas::fit({0,0,sw,sh},{0,0,dw,dh});
        if (!c || dw::ui::empty(c->content_bounds())) return std::nullopt;
        Plan p(*c,sw,sh,dw,dh);
        const auto r = c->content_bounds();
        // Use the exact rounded-edge partition owned by Resolution for both
        // sampling and hit cells. A generic center-sampling stretcher can differ
        // at half ties or due to fixed-point drift.
        for (auto x=r.left; x<r.right; ++x) {
            const auto q=c->inverse_pixel({x,r.top});
            if (!q || q->x<0 || q->x>=sw) return std::nullopt;
            p.columns_.push_back(static_cast<uint32_t>(q->x));
        }
        for (auto y=r.top; y<r.bottom; ++y) {
            const auto q=c->inverse_pixel({r.left,y});
            if (!q || q->y<0 || q->y>=sh) return std::nullopt;
            p.rows_.push_back(static_cast<uint32_t>(q->y));
        }
        return p;
    }
    const dw::ui::Canvas& geometry() const noexcept { return canvas_; }
    // Layer only explicitly covered logical source pixels. Black RGB565 is a
    // valid foreground color; transparency is producer coverage, not color.
    bool overlay(ReadView src,const uint8_t* mask,size_t maskBytes,WriteView dst,size_t* written=nullptr) const noexcept {
        if(src.width!=sw_||src.height!=sh_||dst.width!=dw_||dst.height!=dh_||!mask||maskBytes<uint64_t(sw_)*sh_||
            !validView(src.pixels,src.bytes,src.pitch,src.width,src.height)||!validView(dst.pixels,dst.bytes,dst.pitch,dst.width,dst.height))return false;
        const auto sb=reinterpret_cast<uintptr_t>(src.pixels),db=reinterpret_cast<uintptr_t>(dst.pixels);
        if(sb<db+(dst.height-1)*dst.pitch+dst.width*2&&db<sb+(src.height-1)*src.pitch+src.width*2)return false;
        size_t count=0;const auto r=canvas_.content_bounds();
        for(size_t y=0;y<rows_.size();++y) {
            auto* out=static_cast<uint8_t*>(dst.pixels)+(size_t(r.top)+y)*dst.pitch+size_t(r.left)*2;
            const auto* in=static_cast<const uint8_t*>(src.pixels)+rows_[y]*src.pitch;
            for(size_t x=0;x<columns_.size();++x)if(mask[size_t(rows_[y])*sw_+columns_[x]]){std::memcpy(out+x*2,in+columns_[x]*2,2);++count;}
        }
        if(written)*written=count;return true;
    }
    bool compose(ReadView src, WriteView dst, uint16_t margin = 0) const noexcept {
        if (src.width!=sw_ || src.height!=sh_ || dst.width!=dw_ || dst.height!=dh_ ||
            !validView(src.pixels,src.bytes,src.pitch,src.width,src.height) ||
            !validView(dst.pixels,dst.bytes,dst.pitch,dst.width,dst.height)) return false;
        const auto sb=reinterpret_cast<uintptr_t>(src.pixels);
        const auto db=reinterpret_cast<uintptr_t>(dst.pixels);
        const auto se=sb+(src.height-1)*src.pitch+src.width*2;
        const auto de=db+(dst.height-1)*dst.pitch+dst.width*2;
        // Reject aliasing before touching the destination. Owned logical storage
        // and a current surface Lock must be distinct allocations.
        if (sb<de && db<se) return false;
        auto* out=static_cast<uint8_t*>(dst.pixels);
        const auto* in=static_cast<const uint8_t*>(src.pixels);
        for (uint32_t y=0;y<dh_;++y)
            for (uint32_t x=0;x<dw_;++x) std::memcpy(out+y*dst.pitch+x*2,&margin,2);
        const auto r=canvas_.content_bounds();
        for (size_t y=0;y<rows_.size();++y) {
            auto* target=out+(size_t(r.top)+y)*dst.pitch+size_t(r.left)*2;
            const auto* source=in+rows_[y]*src.pitch;
            for (size_t x=0;x<columns_.size();++x)
                std::memcpy(target+x*2,source+columns_[x]*2,2);
        }
        return true;
    }
};
}
