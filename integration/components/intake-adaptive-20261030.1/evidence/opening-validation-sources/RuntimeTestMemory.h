#pragma once
#include <cstdint>
#include <cstddef>
namespace MenuCanvasTest {
bool managed(uint32_t,size_t);
bool read(uint32_t,void*,size_t);
bool write(uint32_t,const void*,size_t);
}
