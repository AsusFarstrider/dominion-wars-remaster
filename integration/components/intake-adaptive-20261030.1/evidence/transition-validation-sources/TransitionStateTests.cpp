#include "TransitionState.h"
#include <cstdio>
#include <cstdlib>
#include <map>

namespace {
unsigned assertions=0;
void check(bool value){++assertions;if(!value){std::fprintf(stderr,"FAIL %u\n",assertions);std::exit(1);}}
std::map<uint32_t,std::vector<uint8_t>> memory;
template<class T> void put(uint32_t address,const T& value) {
    auto& b=memory[address];b.resize(sizeof(value));memcpy(b.data(),&value,sizeof(value));
}
bool read(uint32_t address,void* output,size_t bytes) {
    const auto i=memory.find(address);
    if(i==memory.end()||i->second.size()!=bytes)return false;
    memcpy(output,i->second.data(),bytes);return true;
}
}
int main() {
    using namespace MenuTransition;
    Snapshot a{};Counter c;
    a.values[0]=10;a.values[1]=10;a.values[5]=0x42A290;
    check(c.observe(a)&&c.frame==1&&c.epoch==1);
    for(unsigned i=0;i<8;++i)check(c.observe(a)&&c.epoch==1);
    auto b=a;b.values[1]=7;
    check(c.observe(b)&&c.epoch==2);
    check(c.observe(a)&&c.epoch==3); // Returning to an old context is a NEW epoch.
    a.nativeRead=true;a.nativeWords[3]=0x123405A0;a.nativeWords[4]=0xABCD4321;
    check(c.observe(a)&&c.epoch==4);
    a.nativeWords[4]=0x55554321; // Cached pixel pointer lower half only.
    a.nativeWords[5]=0x0A00FFFF; // Cached pixels upper half/pitch only.
    check(c.observe(a)&&c.epoch==4);
    a.nativeWords[4]=0x55554322; // COM address upper half changes.
    check(c.observe(a)&&c.epoch==5);
    a.childRead=true;a.child.next=123;
    check(c.observe(a)&&c.epoch==6);
    c.frame=std::numeric_limits<uint64_t>::max();check(!c.observe(a));
    Counter overflow;overflow.epoch=std::numeric_limits<uint64_t>::max();check(!overflow.observe(a));

    Snapshot captured;
    check(!capture(read,captured));
    Frames stack;
    check(!stack.top()&&!stack.matches(100,0)&&!stack.leave(100));
    check(stack.enter(1000,1)&&stack.depth()==1&&stack.top()->transitions==1);
    check(stack.matches(992,8)&&!stack.matches(991,8));
    check(stack.enter(900,0)&&stack.depth()==2&&stack.top()->id==2);
    check(!stack.leave(1000)&&stack.depth()==2);
    check(stack.leave(900)&&stack.top()->id==1);
    check(stack.leave(1000)&&!stack.top());
    for(unsigned i=0;i<32;++i)check(stack.enter(2000+i,1));
    check(!stack.enter(4000,1)&&stack.depth()==32);
    stack.invalidate();check(!stack.top()&&stack.depth()==0);
    for(const auto address:globals)put(address,uint32_t{0});
    std::array<uint8_t,0x90> root{};put(0x5F7ED8,root);
    check(capture(read,captured)&&!captured.outerRead&&!captured.nativeRead&&!captured.childRead);
    put(0xA39B64,uint32_t{0x110000});put(0x110000,uint32_t{0x120000});
    put(0x120000,uint16_t{0x8000});put(0x120020,uint32_t{0x130000});
    std::array<uint32_t,6> image={1,2,3,4,5,6};put(0x130000,image);
    check(capture(read,captured)&&captured.outerRead&&captured.nativeRead&&captured.nativeWords==image);
    memory.erase(0x130000);
    check(capture(read,captured)&&captured.outerRead&&!captured.nativeRead);
    memory.erase(0x4D0A28);
    check(!capture(read,captured));
    std::printf("PASS %u transition state/epoch assertions; no game/device calls\n",assertions);
}
