#pragma once
#include <array>
#include <bit>
#include <cstdint>
#include <string_view>
namespace Remaster::Output {
inline constexpr const char* neutralEngine="e81aacb185e755c6478c06516bf0d442b7b95edc1f3a3603bfc7ecda1c777023";
struct Size {uint32_t width=0,height=0;};
inline Size session{};
inline bool bound=false;
inline bool valid(Size s) noexcept {
    return s.width>=800&&s.height>=600&&s.width<=8192&&s.height<=8192&&uint64_t(s.width)*s.height<=64ull*1024*1024;
}
inline bool decimal(std::wstring_view text,uint32_t& value) noexcept {
    if(text.empty()||text.size()>4||text.front()==L'0')return false;
    uint32_t n=0;for(wchar_t c:text){if(c<L'0'||c>L'9')return false;n=n*10+uint32_t(c-L'0');}value=n;return true;
}
inline bool parse(std::wstring_view width,std::wstring_view height,Size& result) noexcept {
    Size s{};if(!decimal(width,s.width)||!decimal(height,s.height)||!valid(s))return false;result=s;return true;
}
inline uint32_t projectionBits(Size s) noexcept {
    return std::bit_cast<uint32_t>(float(2.0*(800.0/s.width)*((double(s.height)-154.0)/446.0)));
}
bool installStartup(bool staticLoad) noexcept;
}
