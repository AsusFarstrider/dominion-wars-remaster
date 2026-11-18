#pragma once
// Read-only state capture. Cached image fields are not fresh Lock/storage proof.
#include "Core.h"
#include <array>
#include <limits>

namespace MenuTransition {
inline constexpr std::array<uint32_t,13> globals = {
    0x4D0A24,0x4D0A28,0x4D0A20,0x713834,0x713838,0x71383C,
    0x713840,0xA39B64,0xA387B4,0xA38D74,0x5F7ED8+0x73,0xA396D8,0xA38774
};
struct Snapshot {
    // current,pending,previous,input,secondary,draw,scene,holder,alias,device,root child
    std::array<uint32_t,13> values{};
    uint32_t outer=0,native=0;
    std::array<uint32_t,6> nativeWords{};
    MenuProbe::Node root{},child{};
    bool outerRead=false,nativeRead=false,childRead=false;
    // Deliberately excludes mutable cached pixel address/pitch: they are recorded
    // separately and must not manufacture a new content/storage generation.
    std::array<uint32_t,25> key() const noexcept {
        return {values[0],values[1],values[2],values[3],values[4],values[5],values[6],
            values[7],values[8],values[9],values[10],outer,native,
            nativeRead?nativeWords[0]:0,nativeRead?nativeWords[1]:0,
            nativeRead?nativeWords[2]:0,nativeRead?nativeWords[3]:0,
            nativeRead?uint32_t(nativeWords[4]&0xFFFF):0,
            root.next,childRead?child.draw:0,root.flags,root.draw,childRead?child.next:0,
            values[11],values[12]};
    }
};
inline bool capture(const MenuProbe::Reader& read,Snapshot& s) {
    s={};
    for(size_t i=0;i<globals.size();++i)
        if(!read(globals[i],&s.values[i],4))return false;
    if(!MenuProbe::node(read,0x5F7ED8,s.root))return false;
    if(s.values[10])s.childRead=MenuProbe::node(read,s.values[10],s.child);
    s.outerRead=s.values[7]&&read(s.values[7],&s.outer,4);
    uint16_t flags=0;
    if(s.outerRead&&s.outer&&read(s.outer,&flags,2)&&(flags&0x8000)&&
        read(s.outer+0x20,&s.native,4)&&s.native)
        s.nativeRead=read(s.native,s.nativeWords.data(),sizeof(s.nativeWords));
    return true;
}
class Counter {
    std::array<uint32_t,25> previous_{};
    bool seen_=false;
public:
    uint64_t frame=0,epoch=0;
    bool observe(const Snapshot& s) noexcept {
        if(frame==std::numeric_limits<uint64_t>::max())return false;
        const auto k=s.key();
        if(!seen_||k!=previous_) {
            if(epoch==std::numeric_limits<uint64_t>::max())return false;
            ++epoch;previous_=k;seen_=true;
        }
        ++frame;return true;
    }
};
struct Frame {uint64_t id;uint32_t stack;uint8_t transitions;};
class Frames {
    std::array<Frame,32> frames_{};
    size_t depth_=0;
    uint64_t serial_=0;
public:
    size_t depth() const noexcept {return depth_;}
    const Frame* top() const noexcept {return depth_?&frames_[depth_-1]:nullptr;}
    void invalidate() noexcept {depth_=0;}
    bool enter(uint32_t stack,uint8_t transitions) noexcept {
        if(depth_==frames_.size()||serial_==std::numeric_limits<uint64_t>::max())return false;
        frames_[depth_++]={++serial_,stack,transitions};return true;
    }
    bool matches(uint32_t stack,uint32_t offset) const noexcept {
        return top()&&uint64_t(stack)+offset==top()->stack;
    }
    bool leave(uint32_t stack) noexcept {
        if(!matches(stack,0))return false;
        --depth_;return true;
    }
};
}
