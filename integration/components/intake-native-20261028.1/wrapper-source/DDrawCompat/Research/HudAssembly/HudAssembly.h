#pragma once
#include <cstdint>
#include <vector>
#include <d3d.h>
#include <Research/MenuCanvas/quantum_font_research.h>
namespace HudAssembly {
void install() noexcept;
void prepareFrame() noexcept;
void observe(unsigned,const uint32_t*,const uint32_t*,uint32_t) noexcept;
void inputScope(unsigned) noexcept;
void boundary(const char*) noexcept;
void finish() noexcept;
using Blit=uint32_t(__cdecl*)(QImageResearch*,QImageResearch*,int,int);
uint32_t software(Blit,QImageResearch*,QImageResearch*,int,int);
struct Draw {
    std::vector<unsigned char> vertices;bool adapted=false;
    Draw(uintptr_t caller,uint32_t format,const void*,uint32_t count) noexcept;
    void* data(void* original){return adapted?vertices.data():original;}
    void result(int64_t) noexcept;
};
struct Clear {
    D3DRECT rectangle{};bool adapted=false;
    Clear(uintptr_t caller,uintptr_t viewport,uint32_t count,const D3DRECT*,uint32_t flags,uint32_t color,float z,uint32_t stencil) noexcept;
    const D3DRECT* data(const D3DRECT* original)const{return adapted?&rectangle:original;}
    void result(int64_t) noexcept;
};
}
