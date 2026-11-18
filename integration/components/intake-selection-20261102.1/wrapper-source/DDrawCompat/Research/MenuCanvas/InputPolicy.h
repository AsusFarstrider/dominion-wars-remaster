#pragma once
#include "PublicationState.h"
#include <atomic>
#include <cstddef>
namespace MenuCanvas {
enum class InputRoute { Original, Miss, Hit };
inline void drainForeignInput(PublicationState& state,std::atomic<bool>& pending) noexcept {
    if(pending.exchange(false))state.invalidate();
}
inline void inputFrameEntered(PublicationState& state,std::size_t existingDepth) noexcept {
    if(existingDepth)state.invalidate();
}
inline InputRoute routeInput(PublicationState& state,bool admitted,bool hit) noexcept {
    if(state.originalPredicatesAllowed())return InputRoute::Original;
    // Admission failures differ from an eligible point outside a button.
    // Only a complete successful redraw can repair failed ownership.
    if(!admitted){state.invalidate();return InputRoute::Miss;}
    return state.scaledPredicatesAllowed()&&hit?InputRoute::Hit:InputRoute::Miss;
}
}
