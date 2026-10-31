#pragma once
#include <cstdint>
namespace MenuCanvasRuntime {
void resourceBoundary(const char* reason) noexcept;
// Called by the existing reviewed observer bridges, before displaced code.
void observe(unsigned index,const uint32_t* registers,const uint32_t* stack,uint32_t stackPointer,void* originalBlit) noexcept;
void enable() noexcept;
void finish() noexcept;
void coverageLost(const char* reason) noexcept;
}
