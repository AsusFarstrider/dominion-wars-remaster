#pragma once
#include <cstdint>
namespace MenuTransition {
void install() noexcept;
uint64_t epoch() noexcept;
uint64_t frame() noexcept;
void finish() noexcept;
}
