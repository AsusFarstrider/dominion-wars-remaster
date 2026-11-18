#pragma once
#include <Research/MenuCanvas/Canvas565.h>
#include <array>
#include <algorithm>
#include <cstring>
namespace OpeningFlow {
inline constexpr unsigned logicalWidth=800,logicalHeight=600,logicalPitch=1600;
struct alignas(32) Canvas {
    std::array<uint16_t,800*600+64> storage;
    Canvas(){storage.fill(0xA55A);std::fill(storage.begin()+32,storage.end()-32,uint16_t{});}
    uint16_t* data(){return storage.data()+32;}
    bool intact()const {
        for(unsigned i=0;i<32;++i)if(storage[i]!=0xA55A||storage[storage.size()-1-i]!=0xA55A)return false;
        return true;
    }
    bool snapshot(MenuCanvas::ReadView source) {
        if(source.width<800||source.height<600||!MenuCanvas::validView(source.pixels,source.bytes,source.pitch,source.width,source.height))return false;
        for(unsigned y=0;y<600;++y)std::memcpy(data()+y*800,static_cast<const uint8_t*>(source.pixels)+y*source.pitch,1600);
        return intact();
    }
    bool legacy(MenuCanvas::WriteView dest) {
        if(!intact()||dest.width<800||dest.height<600||!MenuCanvas::validView(dest.pixels,dest.bytes,dest.pitch,dest.width,dest.height))return false;
        for(unsigned y=0;y<dest.height;++y)std::memset(static_cast<uint8_t*>(dest.pixels)+y*dest.pitch,0,dest.width*2);
        for(unsigned y=0;y<600;++y)std::memcpy(static_cast<uint8_t*>(dest.pixels)+y*dest.pitch,data()+y*800,1600);
        return true;
    }
    bool fit(const MenuCanvas::Plan& plan,MenuCanvas::WriteView dest) {
        return intact()&&plan.compose({data(),800*600*2,1600,800,600},dest)&&intact();
    }
};
struct MovieGeometry {
    uint32_t width=0,height=0,internalWidth=0,internalHeight=0,handleFlags=0;
    bool supports(uint32_t copyHeight,uint32_t x,uint32_t y,uint32_t flags)const noexcept {
        return width&&height&&width<=800&&height<=600&&width%4==0&&height%2==0&&
            width==internalWidth&&height==internalHeight&&copyHeight==height&&flags==10&&
            !(handleFlags&(0x70000000u|0x80000u))&&x<=800-width&&y<=600-height;
    }
};
}
