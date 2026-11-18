#pragma once
#include "ButtonImage.h"
#include <Research/MenuCanvas/Canvas565.h>
#include <algorithm>
#include <cmath>
#include <new>
namespace Remaster {
// Ordered owned foreground only. Palette conversion has already happened via
// QUANTUM. Immutable RGBA frames bypass the logical raster but keep its geometry.
class OrderedPaint {
    struct Op {dw::ui::Rect rect{},clip{};std::vector<uint16_t> colors;std::vector<uint8_t> mask;SpriteFrame sprite;};
    std::vector<Op> ops;size_t retained=0;
    static constexpr size_t maxBytes=64*1024*1024,maxOps=4096;
    static uint8_t channel(uint16_t v,unsigned which){return which==0?uint8_t(((v>>11)<<3)|(v>>13)):which==1?uint8_t((((v>>5)&63)<<2)|((v>>9)&3)):uint8_t(((v&31)<<3)|((v&31)>>2));}
    static void blend(const SpriteFrame& f,double x,double y,uint16_t& target) noexcept {
        x=std::clamp(x,0.0,double(f.width-1));y=std::clamp(y,0.0,double(f.height-1));
        const auto x0=unsigned(x),y0=unsigned(y),x1=std::min(x0+1,f.width-1),y1=std::min(y0+1,f.height-1);
        const double tx=x-x0,ty=y-y0;const double weights[]={(1-tx)*(1-ty),tx*(1-ty),(1-tx)*ty,tx*ty};
        const size_t indices[]={size_t(y0)*f.width+x0,size_t(y0)*f.width+x1,size_t(y1)*f.width+x0,size_t(y1)*f.width+x1};
        const auto* p=f.storage->data()+f.offset;double a=0,c[3]{};
        for(unsigned k=0;k<4;++k){const auto* pixel=p+indices[k]*4;const double alpha=pixel[3]/255.0;
            a+=weights[k]*alpha;for(unsigned n=0;n<3;++n)c[n]+=weights[k]*alpha*pixel[n];}
        if(a<=0)return;unsigned result[3]{};
        for(unsigned n=0;n<3;++n)result[n]=unsigned(std::clamp(std::floor(c[n]+(1-a)*channel(target,n)+0.5),0.0,255.0));
        target=uint16_t((result[0]>>3)<<11|(result[1]>>2)<<5|(result[2]>>3));
    }
public:
    size_t count()const noexcept{return ops.size();}
    size_t bytes()const noexcept{return retained;}
    bool raw(const uint16_t* logical,const uint8_t* indices,size_t indexBytes,unsigned sw,unsigned sh,int64_t x,int64_t y,bool keyed,bool solid,dw::ui::Rect clip) {
        if(!logical||!sw||!sh||uint64_t(sw)*sh>800*600||x<-32768||x>32767||y<-32768||y>32767||(!solid&&(!indices||uint64_t(sw)*sh>indexBytes)))return false;
        auto r=dw::ui::intersect({x,y,x+sw,y+sh},clip);if(r)r=dw::ui::intersect(*r,{0,0,800,600});if(!r)return true;
        const size_t width=size_t(r->right-r->left),height=size_t(r->bottom-r->top),size=width*height;
        if(ops.size()>=maxOps||size*3>maxBytes-retained)return false;
        Op op;op.rect=*r;op.clip=*r;op.colors.resize(size);op.mask.resize(size);
        for(size_t row=0;row<height;++row)for(size_t col=0;col<width;++col){const auto xx=r->left+col,yy=r->top+row;
            const auto at=row*width+col;op.colors[at]=logical[size_t(yy)*800+size_t(xx)];
            op.mask[at]=uint8_t(solid||!keyed||indices[size_t(yy-y)*sw+size_t(xx-x)]!=0);}
        retained+=size*3;ops.push_back(std::move(op));return true;
    }
    bool sprite(SpriteFrame frame,dw::ui::Rect rect,dw::ui::Rect clip) {
        if(!frame.valid()||dw::ui::empty(rect)||rect.left<0||rect.top<0||rect.right>800||rect.bottom>600||ops.size()>=maxOps)return false;
        Op op;op.rect=rect;op.clip=clip;op.sprite=std::move(frame);ops.push_back(std::move(op));return true;
    }
    bool compose(const dw::ui::Canvas& plan,MenuCanvas::WriteView dst,size_t* written=nullptr)const noexcept {
        if(!MenuCanvas::validView(dst.pixels,dst.bytes,dst.pitch,dst.width,dst.height))return false;
        const auto logical=plan.logical_bounds();if(logical.left||logical.top||logical.right!=800||logical.bottom!=600)return false;
        const auto out=plan.output_bounds();if(out.left||out.top||out.right!=dst.width||out.bottom!=dst.height)return false;
        const auto db=reinterpret_cast<uintptr_t>(dst.pixels),de=db+(dst.height-1)*dst.pitch+dst.width*2;
        for(const auto& op:ops)if(op.sprite.storage){if(!op.sprite.valid())return false;
            const auto sb=reinterpret_cast<uintptr_t>(op.sprite.pixels()),se=sb+size_t(op.sprite.width)*op.sprite.height*4;
            if(sb<de&&db<se)return false;}
        // Raw modal panels can cover millions of output pixels. The inverse
        // rounded-edge partition is separable: calculate each physical column
        // and row once, using the same authoritative inverse as before.
        std::unique_ptr<uint16_t[]> columns,rows;
        if(std::any_of(ops.begin(),ops.end(),[](const Op& op){return !op.sprite.storage;})) {
            columns.reset(new(std::nothrow) uint16_t[dst.width]);
            rows.reset(new(std::nothrow) uint16_t[dst.height]);
            if(!columns||!rows)return false; // No destination writes yet.
            const auto content=plan.content_bounds();
            for(auto x=content.left;x<content.right;++x){const auto q=plan.inverse_pixel({x,content.top});if(!q)return false;columns[size_t(x)]=uint16_t(q->x);}
            for(auto y=content.top;y<content.bottom;++y){const auto q=plan.inverse_pixel({content.left,y});if(!q)return false;rows[size_t(y)]=uint16_t(q->y);}
        }
        size_t count=0;
        for(const auto& op:ops){auto clipped=dw::ui::intersect(op.rect,op.clip);if(!clipped)continue;
            const auto dest=plan.map_rect(*clipped);if(!dest)continue;const auto full=plan.map_rect(op.rect);if(!full)continue;
            for(auto y=dest->top;y<dest->bottom;++y)for(auto x=dest->left;x<dest->right;++x){auto* p=static_cast<uint8_t*>(dst.pixels)+size_t(y)*dst.pitch+size_t(x)*2;
                if(op.sprite.storage){uint16_t pixel=0;memcpy(&pixel,p,2);
                    const double sx=(double(x-full->left)+0.5)*op.sprite.width/double(full->right-full->left)-0.5;
                    const double sy=(double(y-full->top)+0.5)*op.sprite.height/double(full->bottom-full->top)-0.5;
                    blend(op.sprite,sx,sy,pixel);memcpy(p,&pixel,2);++count;
                }else{
                    const auto at=size_t(rows[size_t(y)]-op.rect.top)*size_t(op.rect.right-op.rect.left)+size_t(columns[size_t(x)]-op.rect.left);
                    if(op.mask[at]){memcpy(p,&op.colors[at],2);++count;}}
            }
        }if(written)*written=count;return true;
    }
};
}
