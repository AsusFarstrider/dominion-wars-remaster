#pragma once
// Two explicit original UI families. Geometry is logical, not a display mode.
// Main-menu policy is research-only pending the full animation contract review.
#include <array>
#include <cstdint>
namespace MenuCanvas {
struct Button {uint32_t id;int16_t x,y;uint16_t w,h;uint32_t baseImage=0;uint8_t hoverRestart=0;};
struct Family {
    const char* name;
    uint32_t state,root,id,flags,draw,firstButton,input,buttonDraw,dynamicFlags;
    std::array<Button,6> buttons;
};
inline constexpr Family selection={"selection",10,0x4E5050,0x43F,4,0x472630,0x4E50E0,0x4729F0,0,0xE0000,
    {{{0x441,128,48,570,32},{0x440,128,94,570,32},{0x442,510,140,180,32},
      {0x443,310,140,180,32},{0x445,110,140,180,32},{0x446,40,558,140,32}}}};
inline constexpr Family mainMenu={"main_menu",7,0x4E47C0,0x405,0x84,0x46ADD0,0x4E4850,0x46AFA0,0x46B440,0x1F0000,
    {{{0x3F3,320,50,180,100,0x202,12},{0x3F4,320,115,180,100,0x26B,12},{0x3F7,320,185,180,100,0x225,11},
      {0x3F6,320,255,180,100,0x28E,12},{0x3F8,320,325,180,100,0x248,12},{0x3F5,320,395,180,100,0x2B1,12}}}};
inline const Family* forState(uint32_t state,bool mainEnabled) noexcept {
    return state==selection.state?&selection:mainEnabled&&state==mainMenu.state?&mainMenu:nullptr;
}
inline const Family* forRoot(uint32_t root,bool mainEnabled) noexcept {
    return root==selection.root?&selection:mainEnabled&&root==mainMenu.root?&mainMenu:nullptr;
}
inline const Family* forNode(uint32_t node,bool mainEnabled) noexcept {
    for(const auto* f:{&selection,&mainMenu}) {
        if(f==&mainMenu&&!mainEnabled)continue;
        if(node>=f->firstButton&&node<=f->firstButton+5*0x90&&(node-f->firstButton)%0x90==0)return f;
    }
    return nullptr;
}
inline uint32_t mainDeparture(uint32_t node,bool submenus) noexcept {
    if(node==0x4E4850)return 10;
    if(node==0x4E4B20)return 18;
    if(submenus&&node==0x4E4970)return 20;
    if(submenus&&node==0x4E4A90)return 15;
    return 0;
}
inline bool pendingAllowed(const Family& f,uint32_t pending,bool rendering,uint32_t provedPending=0) noexcept {
    if(pending==f.state)return true;
    // The main menu can finish a proved destination activation while drawing. This
    // permits a complete departing visual only, never interactive eligibility.
    return rendering && (&f==&mainMenu) && pending==provedPending && (pending==10||pending==18||pending==15||pending==20);
}
}
