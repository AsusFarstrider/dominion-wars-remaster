#pragma once
#include <Research/OpeningFlow/OpeningFlow.h>
#include <array>
#include <cstdint>
#include <string>
#include <type_traits>

namespace D3dDdi {class Resource;}
namespace NativeProbe {
bool active() noexcept;
bool allowEvent(const char* kind,const std::string& fields) noexcept;
bool allowCall(const char* layer,const char* name) noexcept;
bool apiName(const char*) noexcept;
void install() noexcept;
void finish() noexcept;
void presentation(uintptr_t device,const D3dDdi::Resource& dst,unsigned dstSub,const void* dstRect,
    const D3dDdi::Resource& src,unsigned srcSub,const void* srcRect,unsigned filter) noexcept;
class ApiScope {
    const char* name;uintptr_t object,caller;uint64_t id=0;bool menuScope=false;std::array<uintptr_t,8> args{};
    size_t argumentCount=0;
    void begin() noexcept;
    template<class T> static uintptr_t value(const T& v){
        if constexpr(std::is_pointer_v<T>)return reinterpret_cast<uintptr_t>(v);
        else if constexpr(std::is_integral_v<T>||std::is_enum_v<T>)return static_cast<uintptr_t>(v);
        else return 0; // Only known pointer/integer positions are decoded.
    }
public:
    ApiScope(const char* n,uintptr_t o,uintptr_t c=0):name(n),object(o),caller(c){}
    template<class... P> void arguments(const P&... p) noexcept {
        const uintptr_t values[]={0,value(p)...};
        for(size_t i=0;i<sizeof...(P)&&i<args.size();++i)args[i]=values[i+1];
        argumentCount=sizeof...(P);
        OpeningFlow::apiBegin(name,object,caller,args.data(),argumentCount);
        if(!active()||!apiName(name))return;
        begin();
    }
    void result(int64_t) noexcept;
    ~ApiScope();
};
}
