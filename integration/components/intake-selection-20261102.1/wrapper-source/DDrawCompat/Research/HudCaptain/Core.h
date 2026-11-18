#pragma once
#include <Research/MenuCanvas/ui_canvas.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
namespace HudCaptain {
struct Transform {
    int ox=0,oy=0,dx=0,dy=0;int64_t n=1,d=1;unsigned width=0,height=0;
    static int64_t rounded(int64_t a,int64_t b){a+=b/2;const auto q=a/b;return q-(a%b<0);}
    int edge(int p,bool x)const{return int((x?dx:dy)+rounded(int64_t(p-(x?ox:oy))*n,d));}
    double vertex(double p,bool x)const{return (x?dx:dy)+(p-(x?ox:oy))*double(n)/double(d);}
    int cell(int p,bool x)const {
        int low=-32768,high=32768;
        while(low+1<high){const int mid=low+(high-low)/2;if(edge(mid,x)<=p)low=mid;else high=mid;}
        return low;
    }
    dw::ui::Rect rect(dw::ui::Rect r)const{return {edge(int(r.left),true),edge(int(r.top),false),edge(int(r.right),true),edge(int(r.bottom),false)};}
    std::optional<dw::ui::Point> inverse(int x,int y)const {
        if(x<0||y<0||x>=int(width)||y>=int(height))return {};
        return dw::ui::Point{cell(x,true),cell(y,false)};
    }
    static std::optional<Transform> fit(unsigned w,unsigned h,int x,int y,int occupiedRight,int sceneBottom) {
        if(w>16384||h>16384||w<800||h<600||x<0||y<sceneBottom||y+64>int(h)||occupiedRight<0)return {};
        const int margin=16,freeLeft=occupiedRight+margin,freeWidth=int(w)-freeLeft-margin;
        if(freeWidth<544)return {};
        Transform t{x,y,0,y,int64_t(h),600,w,h};
        auto limit=[&](int64_t numerator,int64_t denominator){if(numerator*t.d<t.n*denominator){t.n=numerator;t.d=denominator;}};
        limit(freeWidth,544);limit(int(h)-y,64);
        if(t.n<t.d)return {};
        t.dx=freeLeft+(freeWidth-int(rounded(544*t.n,t.d)))/2;
        return t;
    }
};
}
