#pragma once
#include <cstdint>
#include <string>
#include <type_traits>
namespace Qfr {
inline bool recording=false;
inline bool requested() noexcept{return recording;}
template<class T> int64_t number(T v){if constexpr(std::is_pointer_v<T>)return int64_t(reinterpret_cast<uintptr_t>(v));else return int64_t(v);}
template<class... P> std::string parameters(const P&...){return {};}
struct Call {
 template<class F> Call(const char*,const char*,uintptr_t,F&&){}
 template<class F> void result(int64_t,F&&){}
};
}
