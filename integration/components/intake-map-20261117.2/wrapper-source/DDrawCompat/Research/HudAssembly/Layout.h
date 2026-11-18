#pragma once
#include <Research/HudCaptain/Core.h>
#include <array>
#include <cstdint>
#include <optional>

namespace HudAssembly {
using Rect=dw::ui::Rect;
using Point=dw::ui::Point;
struct Layout {
    unsigned width=0,height=0;
    int panelWidth=0,panelHeight=0,previewInset=0;
    HudCaptain::Transform transform;
    Rect scene,reserve,radar,preview;
    static std::optional<Layout> fit(unsigned w,unsigned h,unsigned panel,unsigned panelH,unsigned inset) {
        if(w<800||h<600||w>16384||h>16384||panel<2||panel>300||panelH<1||panelH>400||inset>154)return {};
        Layout l{};l.width=w;l.height=h;l.panelWidth=int(panel)-1;l.panelHeight=int(panelH);l.previewInset=int(inset);
        const int64_t n=uint64_t(w)*600<uint64_t(h)*800?w:h;
        const int64_t d=uint64_t(w)*600<uint64_t(h)*800?800:600;
        const int assemblyWidth=int(HudCaptain::Transform::rounded(800*n,d));
        l.transform={0,int(h),(int(w)-assemblyWidth)/2,int(h),n,d,w,h};
        const auto mapped=[&](Rect r){r.top+=int(h)-600;r.bottom+=int(h)-600;return l.transform.rect(r);};
        l.reserve={0,mapped({0,446,800,600}).top,w,h};
        l.scene={0,0,w,l.reserve.top};
        l.radar=mapped({0,600-int(panelH),int(panel)-1,600});
        l.preview=mapped({int(panel)-1,446+int(inset),int(panel)-1+160,446+int(inset)+103});
        if(l.scene.bottom<=0||l.preview.bottom>h||l.radar.left<0||l.radar.right>w)return {};
        return l;
    }
    Rect mapSource(Rect r)const{return transform.rect(r);}
    double projectionFactor()const{return double(scene.bottom)/double(height-154);}
    HudCaptain::Transform group(int sourceX,int sourceY,int canonicalX,int canonicalY)const {
        auto t=transform;t.ox=sourceX;t.oy=sourceY;
        t.dx=transform.edge(canonicalX,true);t.dy=transform.edge(int(height)-600+canonicalY,false);return t;
    }
};
}
