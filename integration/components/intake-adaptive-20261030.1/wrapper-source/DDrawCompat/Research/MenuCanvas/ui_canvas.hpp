#pragma once
// Source-only geometry. No engine addresses, rendering, input hooks or OS APIs.
#include <algorithm>
#include <cstdint>
#include <initializer_list>
#include <optional>

namespace dw::ui {
using i64 = std::int64_t;
struct Point { i64 x, y; };
// Integer edges, half-open [left,right) x [top,bottom).
struct Rect { i64 left, top, right, bottom; };
struct Rational { i64 numerator, denominator; };

inline bool empty(Rect r) noexcept { return r.left >= r.right || r.top >= r.bottom; }
inline bool contains(Rect r, Point p) noexcept {
    return !empty(r) && p.x >= r.left && p.x < r.right && p.y >= r.top && p.y < r.bottom;
}
inline std::optional<Rect> intersect(Rect a, Rect b) noexcept {
    Rect r{std::max(a.left,b.left),std::max(a.top,b.top),
           std::min(a.right,b.right),std::min(a.bottom,b.bottom)};
    return empty(r) ? std::nullopt : std::optional<Rect>{r};
}

class Canvas {
public:
    // Explicit conservative limits keep all intermediate products in int64.
    static constexpr i64 max_extent = 1000000;
    static constexpr i64 max_coordinate = 1000000000;
    static std::optional<Canvas> fit(Rect logical, Rect output) noexcept {
        if (!valid(logical) || !valid(output)) return std::nullopt;
        const i64 lw=logical.right-logical.left, lh=logical.bottom-logical.top;
        const i64 ow=output.right-output.left, oh=output.bottom-output.top;
        const bool width_limited=ow*lh <= oh*lw;
        const i64 n=width_limited ? ow : oh;
        const i64 d=width_limited ? lw : lh;
        // Offsets are images of logical left/top, not transforms of absolute zero.
        return Canvas(logical,output,n,d,
            2*d*output.left + ow*d - lw*n,
            2*d*output.top  + oh*d - lh*n);
    }
    Rect logical_bounds() const noexcept { return logical_; }
    Rect output_bounds() const noexcept { return output_; }
    Rational uniform_scale() const noexcept { return {n_,d_}; }
    Rational offset_x() const noexcept { return {ox_,2*d_}; }
    Rational offset_y() const noexcept { return {oy_,2*d_}; }
    Rect content_bounds() const noexcept {
        return {edge_x(logical_.left),edge_y(logical_.top),
                edge_x(logical_.right),edge_y(logical_.bottom)};
    }
    // Clip in logical space first. Then transform EDGES, not origin+rounded size.
    // Ties round toward +infinity, including negative coordinates.
    std::optional<Rect> map_rect(Rect r) const noexcept {
        const auto local=intersect(r,logical_);
        if (!local) return std::nullopt;
        Rect mapped{edge_x(local->left),edge_y(local->top),
                    edge_x(local->right),edge_y(local->bottom)};
        return intersect(mapped,output_);
    }
    std::optional<Rect> map_rect(Rect r, Rect physical_clip) const noexcept {
        const auto mapped=map_rect(r);
        return mapped ? intersect(*mapped,physical_clip) : std::nullopt;
    }
    // Inverse for INTEGER physical pixel indices and INTEGER logical hit cells.
    // Matches the exact rounded-edge partition used by map_rect. No clamping of
    // margins. Collapsed logical cells during minification have no hit pixels.
    // This is not an unquantized cursor delta or texture-sampling coordinate API.
    std::optional<Point> inverse_pixel(Point p) const noexcept {
        if (!contains(content_bounds(),p)) return std::nullopt;
        return Point{cell(p.x,true),cell(p.y,false)};
    }
    std::optional<Point> inverse_pixel(Point p, Rect physical_clip) const noexcept {
        if (!contains(physical_clip,p)) return std::nullopt;
        return inverse_pixel(p);
    }
private:
    Rect logical_,output_;
    i64 n_,d_,ox_,oy_;
    Canvas(Rect l, Rect o, i64 n, i64 d, i64 x, i64 y) noexcept
        : logical_(l),output_(o),n_(n),d_(d),ox_(x),oy_(y) {}
    static bool valid(Rect r) noexcept {
        // Check bounds BEFORE subtracting: also handles extreme int64 inputs.
        for (i64 v : {r.left,r.top,r.right,r.bottom})
            if (v < -max_coordinate || v > max_coordinate) return false;
        return !empty(r) && r.right-r.left <= max_extent && r.bottom-r.top <= max_extent;
    }
    static i64 round_edge(i64 numerator, i64 denominator) noexcept {
        // floor(n/d + 1/2), avoiding implementation-defined negative shifts.
        const i64 adjusted=numerator+denominator/2;
        i64 q=adjusted/denominator;
        if (adjusted%denominator < 0) --q;
        return q;
    }
    i64 edge_x(i64 x) const noexcept {
        return round_edge(ox_+2*n_*(x-logical_.left),2*d_);
    }
    i64 edge_y(i64 y) const noexcept {
        return round_edge(oy_+2*n_*(y-logical_.top),2*d_);
    }
    i64 cell(i64 p, bool x_axis) const noexcept {
        i64 lo=x_axis ? logical_.left : logical_.top;
        i64 hi=x_axis ? logical_.right : logical_.bottom;
        // Last logical left edge <= pixel. Search is bounded O(log extent).
        // Equality uses the rounded edge, avoiding fractional tie disagreement.
        while (lo+1 < hi) {
            const i64 mid=lo+(hi-lo)/2;
            if ((x_axis ? edge_x(mid) : edge_y(mid)) <= p) lo=mid;
            else hi=mid;
        }
        return lo;
    }
};
} // namespace dw::ui
