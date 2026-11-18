#pragma once
#include "Recorder.h"
#include <sstream>
#include <type_traits>
#include <Common/Log.h>

namespace Qfr {
template<class... T> std::string parameters(const T&... values){
    std::ostringstream out;Compat::LogStream stream(out);unsigned index=0;
    ((stream << (index++?", ":"") << values),...);return out.str();
}
template<class T> int64_t number(T value){if constexpr(std::is_pointer_v<T>)return int64_t(reinterpret_cast<uintptr_t>(value));else return int64_t(value);}
}
