#pragma once
#include <cstdint>
namespace MenuCanvas {
// Describes writes to a research target, not monitor scanout acknowledgment.
// The bounded controller has no automatic native repaint/retirement operation.
// Once scaled pixels may exist, native hitboxes are NEVER inferred from an error.
class PublicationState {
    bool mayBeScaled_=false,coherent_=false;
    uint64_t committed_=0,epoch_=0;
public:
    bool originalPredicatesAllowed() const noexcept {return !mayBeScaled_;}
    bool scaledPredicatesAllowed() const noexcept {return mayBeScaled_&&coherent_;}
    bool mayContainScaledPixels() const noexcept {return mayBeScaled_;}
    uint64_t generation() const noexcept {return committed_;}
    uint64_t epoch() const noexcept {return epoch_;}
    void invalidate() noexcept {coherent_=false;++epoch_;}
    void beforeScaledWrite() noexcept {mayBeScaled_=true;coherent_=false;}
    bool committed(uint64_t generation,uint64_t candidateEpoch,bool interactive=true) noexcept {
        if(candidateEpoch!=epoch_)return false;
        mayBeScaled_=true;coherent_=interactive;committed_=generation;return true;
    }
};
}
