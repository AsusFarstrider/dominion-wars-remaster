#pragma once
// Console controller test only; no production build includes this header.
#include <cstdint>
#include <cstddef>
namespace MenuCanvasTest {
bool managed(uint32_t address,size_t bytes);
bool readable(uint32_t address,size_t bytes);
bool read(uint32_t address,void* output,size_t bytes);
void write(uint32_t address,const void* input,size_t bytes);
}
