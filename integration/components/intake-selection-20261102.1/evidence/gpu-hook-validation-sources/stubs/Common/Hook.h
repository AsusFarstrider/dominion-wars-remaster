#pragma once
#include <string>
#include <typeinfo>
namespace Compat {
inline std::string funcPtrToStr(const void*) {return "fixture";}
template<class T> std::string getTypeName(){return typeid(T).name();}
}
