#pragma once
#include <cstdint>
#include <string>
namespace MenuProbe {
void install(const std::string& exeIdentity) noexcept;
void finish() noexcept;
std::string annotate(const char* kind,const std::string& fields) noexcept;
std::string ownerKey() noexcept;
bool apiBegin(uint64_t id,const char* name,uintptr_t object,const uintptr_t* args,size_t count) noexcept;
void apiEnd(uint64_t id) noexcept;
}
