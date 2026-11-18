#pragma once
#include <cstdint>
namespace NativeProbe {
struct ApiScope {
 ApiScope(const char*,uintptr_t,uintptr_t){}
 template<class... P> void arguments(const P&...){}
 void result(int64_t){}
};
}
