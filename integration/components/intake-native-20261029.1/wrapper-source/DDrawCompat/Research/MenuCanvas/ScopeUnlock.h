#pragma once
#include <type_traits>
#include <utility>
namespace MenuCanvas {
template<class Unlock> class ScopeUnlock {
    Unlock unlock_;
    bool held_=true;
public:
    explicit ScopeUnlock(Unlock fn) noexcept(std::is_nothrow_move_constructible_v<Unlock>) : unlock_(std::move(fn)) {}
    ScopeUnlock(const ScopeUnlock&)=delete;
    ScopeUnlock& operator=(const ScopeUnlock&)=delete;
    auto close(){held_=false;return unlock_();}
    ~ScopeUnlock() noexcept {if(held_)try{unlock_();}catch(...){}}
};
}
