#pragma once
#include <cstdint>
#include <vector>
#include <d3d.h>
#include <Research/MenuCanvas/quantum_font_research.h>
namespace HudCaptain {
void install() noexcept;
void observe(unsigned,const uint32_t*,const uint32_t*,uint32_t) noexcept;
void inputScope(unsigned) noexcept;
uint32_t predicate(const uint32_t*,uint32_t) noexcept;
void boundary(const char*) noexcept;
void finish() noexcept;
using Blit=uint32_t(__cdecl*)(QImageResearch*,QImageResearch*,int,int);
uint32_t software(Blit,QImageResearch*,QImageResearch*,int,int);
struct Draw {
    std::vector<unsigned char> vertices;bool adapted=false;
    Draw(uint32_t format,const void*,uint32_t count) noexcept;
    void* data(void* original){return adapted?vertices.data():original;}
    void result(int64_t) noexcept;
};
struct StripClear {
    D3DRECT rectangle{};bool adapted=false;
    StripClear(uintptr_t caller,uintptr_t viewport,uint32_t count,const D3DRECT*,uint32_t flags,uint32_t color,float z,uint32_t stencil) noexcept;
    const D3DRECT* data(const D3DRECT* original)const{return adapted?&rectangle:original;}
    void result(int64_t) noexcept;
};
}
