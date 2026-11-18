#pragma once
// Original-address research layouts; no engine memory writes.
#include <array>
#include <cstdint>
#include <cstring>
#include <functional>
#include <string>
#include <vector>
namespace MenuProbe {
using Reader=std::function<bool(uint32_t,void*,size_t)>;
struct Node {uint32_t address=0,id=0,flags=0,parent=0,next=0,child=0,draw=0,input=0,palette=0;
    int16_t x=0,y=0;uint16_t w=0,h=0;};
inline bool node(const Reader& read,uint32_t address,Node& n){
    if(address<0x10000||uint64_t(address)+0x90>0x100000000ull)return false;
    uint8_t b[0x90]{};if(!read(address,b,sizeof(b)))return false;
    n={};n.address=address;
    auto u32=[&](size_t p){uint32_t v;memcpy(&v,b+p,4);return v;};
    n.id=u32(0);n.flags=u32(4);memcpy(&n.x,b+8,2);memcpy(&n.y,b+10,2);
    memcpy(&n.w,b+12,2);memcpy(&n.h,b+14,2);
    n.input=u32(0x48);n.draw=u32(0x4c);n.next=u32(0x6b);n.child=u32(0x73);n.parent=u32(0x77);n.palette=u32(0x8c);return true;
}
struct Ancestry {std::vector<Node> nodes;std::string role="unrelated",gap;};
inline Ancestry ancestry(const Reader& read,uint32_t address){
    Ancestry a;
    while(address){
        if(a.nodes.size()>=32){a.gap="parent_depth_limit";break;}
        for(const auto& n:a.nodes)if(n.address==address){a.gap="parent_cycle";return a;}
        Node n;if(!node(read,address,n)){a.gap="parent_unreadable";return a;}
        a.nodes.push_back(n);address=n.parent;
    }
    if(!a.gap.empty())return a;
    for(const auto& n:a.nodes){
        if(n.address==0x4DA7C8&&n.id==0x49d){a.role="native_popup";return a;}
        if(n.address==0x4E5050&&n.id==0x43f&&n.draw==0x472630){a.role=a.nodes.front().address==n.address?"selection_root":"selection_child";return a;}
    }
    return a;
}
struct Callback {uint64_t serial=0;uint32_t node=0;const char* role="unattributed";bool emitted=false;};
class CallbackStack {
    std::array<Callback,64> values{};size_t count=0;
public:
    bool push(Callback c){if(count==values.size())return false;values[count++]=c;return true;}
    const Callback* top() const {return count?&values[count-1]:nullptr;}
    bool pop(uint32_t node){if(!count||values[count-1].node!=node)return false;--count;return true;}
    size_t depth()const{return count;}
    void clear(){count=0;}
};
}
