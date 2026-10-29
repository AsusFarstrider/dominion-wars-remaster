#pragma once
#include <cstddef>
#include <cstdint>
namespace OpeningFlow {
using BinkCopy=int(__stdcall*)(void*,void*,int,unsigned,unsigned,unsigned,unsigned);
bool installSafety() noexcept; // Must precede all media Open calls for the opt-in HD profile.
void install(bool copyReady=true) noexcept;
void observe(unsigned,const uint32_t*,const uint32_t*,uint32_t,void* originalBlit=nullptr) noexcept;
void inputScope(unsigned) noexcept;
bool submenuMode() noexcept;
bool mainEvent(uint32_t caller,uint32_t node,uint32_t event) noexcept;
uint32_t predicate(const uint32_t*,uint32_t original) noexcept;
void apiBegin(const char*,uintptr_t,uintptr_t,const uintptr_t*,size_t) noexcept;
void apiResult(const char*,uintptr_t,uintptr_t,const uintptr_t*,size_t,int64_t) noexcept;
void resourceBoundary(const char*) noexcept;
int copy(BinkCopy,uintptr_t,void*,void*,int,unsigned,unsigned,unsigned,unsigned);
void finish() noexcept;
}
