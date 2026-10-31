#pragma once
#include "Runtime.h"
#include "ButtonImage.h"
#include <Research/FlightRecorder/Recorder.h>
#include <array>
#include <bitset>
namespace Remaster::MainButtons {
struct Button {uint32_t node,id,idle;int y;const wchar_t* file;const char* name;const char* digest;};
inline constexpr Button table[]={
 {0x4E4850,0x3F3,0x202,50,L"new_game.dwbank","new_game","aa75785a8e503cf1d1bbe2dcb757f071149507840ac0a6801dc1ca86c4d50a83"},
 {0x4E48E0,0x3F4,0x26B,115,L"load_game.dwbank","load_game","7c23f348098db88db9b5307b12b14ee9a82cf4e04a595c268b2b2a8ab07d9001"},
 {0x4E4970,0x3F7,0x225,185,L"multiplayer.dwbank","multiplayer","2814a429864fcbd77788e5ce4a6332bb6a549b16d83f0e056245942227ef8b04"},
 {0x4E4A00,0x3F6,0x28E,255,L"preferences.dwbank","preferences","d0f3c2747b916c9c1f6ca955a391fd2ddb51f95029a9b8d40a0d8faa1eb9834b"},
 {0x4E4A90,0x3F8,0x248,325,L"credits.dwbank","credits","f62e8d6c060b6ab645b1af130ea0db8ff1d2a56c3c7b4e92f24568c98a970d5f"},
 {0x4E4B20,0x3F5,0x2B1,395,L"quit.dwbank","quit","ca5575fa6534a6772b053bedcee15881b8daf50de43d6ba010f1316bff002bf5"}};
inline std::array<std::shared_ptr<const ButtonBank>,6> banks;
inline bool available=false;inline DWORD thread=0;
struct Receipt {uint64_t scope=0;uint32_t node=0,resource=0,source=0;};
inline Receipt pending;inline std::atomic<bool> foreign{false};inline std::bitset<18> reported;
inline int index(uint32_t node){for(int i=0;i<6;++i)if(table[i].node==node)return i;return -1;}
inline void invalidate()noexcept{if(thread==GetCurrentThreadId())pending={};else foreign=true;}
inline void witness(uint64_t scope,uint32_t node,uint32_t resource,uint32_t source)noexcept {
    if(!available)return;if(thread!=GetCurrentThreadId()){foreign=true;return;}
    pending={};const auto i=index(node);if(i<0||!scope||resource<table[i].idle||resource>=table[i].idle+35||!source)return;
    pending={scope,node,resource,source};
}
inline std::optional<SpriteFrame> consume(uint64_t scope,uint32_t node,uint32_t source,uint32_t caller) {
    if(!available||thread!=GetCurrentThreadId())return std::nullopt;const auto receipt=pending;pending={};
    const auto i=index(node);if(foreign.exchange(false)||i<0||caller!=0x46B57E||receipt.scope!=scope||receipt.node!=node||receipt.source!=source||
        !banks[i]||receipt.resource<table[i].idle||receipt.resource>=table[i].idle+35)return std::nullopt;
    const auto ordinal=receipt.resource-table[i].idle;const auto frame=banks[i]->frames[ordinal];if(!frame.valid())return std::nullopt;
    const auto family=ordinal==0?0:ordinal<=25?1:2;const auto bit=size_t(i*3+family);
    if(!reported[bit]){reported.set(bit);note("native_button_state",std::string(table[i].name)+"; resource="+std::to_string(receipt.resource)+"; family="+std::to_string(family)+"; logical180x100/source720x400");}
    Qfr::event("native_main_button_draw",",\"scope\":"+std::to_string(scope)+",\"node\":"+std::to_string(node)+",\"resource\":"+std::to_string(receipt.resource)+",\"ordinal\":"+std::to_string(ordinal));
    return frame;
}
#if defined(OPENING_FLOW_TEST)||defined(MENU_CANVAS_TEST)
inline void install(){} // These controllers exercise a supplied synthetic catalog.
#else
void install();
#endif
}
