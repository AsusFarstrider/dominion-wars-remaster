#pragma once
#include "Identity.h"
#include <Windows.h>
#include <atomic>
#include <string>
namespace Remaster {
inline bool managed=false,valid=false;
inline Options config;inline Identity version;
inline bool paramountHD=false,gizmoHD=false,ssHD=false;
enum class Feature {native,ui,canvas,mainCanvas,opening,submenus,hud,captainTrial,cards,mainBackground,mainButtons,hd,transitions};
inline bool env(const wchar_t* name){wchar_t v[2]{};return GetEnvironmentVariableW(name,v,2)==1&&v[0]==L'1';}
inline bool enabled() noexcept{return managed&&valid;}
inline bool recordingAllowed() noexcept{return !managed||(valid&&config.recording);}
inline bool feature(Feature f,const wchar_t* legacy) {
    if(!managed)return env(legacy);
    if(!valid)return false;
    switch(f){case Feature::captainTrial:return false;case Feature::cards:return config.publisher||config.title;
        case Feature::mainBackground:return config.main;case Feature::mainButtons:return config.buttons;case Feature::hd:return paramountHD||gizmoHD||ssHD;
        case Feature::transitions:return true;default:return true;}
}
inline bool cardAllowed(uint32_t resource){return !managed||(valid&&(resource==0x101?config.publisher:resource==0x103&&config.title));}
enum Component:unsigned {nativeReady=1,uiReady=2,canvasReady=4,openingReady=8,hudReady=16,transitionReady=32};
inline std::atomic<unsigned> readiness{0};
inline void ready(Component c) noexcept{readiness.fetch_or(c);}
inline bool functionalReady() noexcept{return enabled()&&readiness.load()==63;}
bool load() noexcept;
void initialize() noexcept;
void note(const char* kind,const std::string& detail={}) noexcept;
}
