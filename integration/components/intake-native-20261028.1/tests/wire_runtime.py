"""One-time exact-anchor wiring of the disposable production derivative."""
from pathlib import Path
root=Path('E:/Codex/research/dominion-rendering/production-runtime-work-01/DDrawCompat')
def edit(rel,changes):
 p=root/rel;s=p.read_text()
 if rel.endswith('.cpp') or rel.endswith('MainBackdrop.h'):s='#include <Remaster/Runtime.h>\n'+s
 for old,new in changes:
  assert old in s,(rel,old[:100]);s=s.replace(old,new)
 p.write_text(s)
edit('Dll/DllMain.cpp',[
 ('\t\tQfr::initializeProcess();','\t\tRemaster::initialize();\n\t\tQfr::initializeProcess();'),
 ('if (!OpeningFlow::installSafety())return FALSE;','if (!Remaster::load())return FALSE;\n\t\tif (!OpeningFlow::installSafety())return FALSE;\n\t\tif (Remaster::enabled() && !Config::enableDDrawCompat.get()){Remaster::note("startup_refused","managed renderer requires DDrawCompat");return FALSE;}')])
edit('Research/FlightRecorder/Recorder.cpp',[
 ('bool requested() noexcept {wchar_t b[2]{};','bool requested() noexcept {if(!Remaster::recordingAllowed())return false;wchar_t b[2]{};'),
 ('void event(const char* kind,const std::string& fields) noexcept {if(!enabled()',
  '''void event(const char* kind,const std::string& fields) noexcept {
    if(Remaster::enabled()&&(!strcmp(kind,"native_opening_error")||!strcmp(kind,"native_main_background_reject")||!strcmp(kind,"native_canvas_coverage_lost")||!strcmp(kind,"native_hud_assembly_error")))Remaster::note(kind,fields);
    if(!enabled()''')])
edit('Research/FlightRecorder/Engine.cpp',[
 ('void engineHooks(){','void engineHooks(){\n    if(Remaster::enabled())return; // Functional installation already ran without recorder dependence.')])
edit('Research/NativeProbe/Probe.cpp',[
 ('return GetEnvironmentVariableW(L"QFR_NATIVE_PROBE",v,2)==1&&v[0]==L\'1\';','return Remaster::feature(Remaster::Feature::native,L"QFR_NATIVE_PROBE");'),
 ('        uint32_t r[9]{},stack[9]{};', '''        if(index==2&&selected){uint32_t context=0,mode=0,m[3]{};
            if(!read(0xA387A0,context)||!read(context+0x14,mode)||!read(mode,m)||m[0]!=selected->width||m[1]!=selected->height){
                Remaster::note("native_mode_refused","selected mode differs from package profile");ExitProcess(0x4E01);}}
        if(!Qfr::enabled())return;
        uint32_t r[9]{},stack[9]{};'''),
 ('        MenuProbe::install(identity);','        MenuProbe::install(identity);\n        Remaster::ready(Remaster::nativeReady);')])
edit('Research/MenuProbe/Probe.cpp',[
 ('#include "Probe.h"','#include "Probe.h"\n#include <Remaster/VersionLabel.h>'),
 ('        const uint32_t sp=r[3]+4;','''        const uint32_t sp=r[3]+4;
        if(index==12)Remaster::VersionLabel::substitute(s[0],s[1],s[2],sp+8,reader,
            [](uint32_t a,const void* p,size_t n){SIZE_T got=0;return WriteProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(a),p,n,&got)&&got==n;});'''),
 ('if(GetEnvironmentVariableW(L"QFR_UI_PROBE",env,2)!=1||env[0]!=L\'1\')return;','if(!Remaster::feature(Remaster::Feature::ui,L"QFR_UI_PROBE"))return;'),
 ('compact=GetEnvironmentVariableW(L"QFR_OPENING_FLOW",compactValue,2)==1&&compactValue[0]==L\'1\';','compact=Remaster::feature(Remaster::Feature::opening,L"QFR_OPENING_FLOW");'),
 ('        ready=true;Qfr::event(', '        ready=true;Remaster::ready(Remaster::uiReady);Qfr::event('),
 ('    try{emit("native_ui_movie",','    try{if(Qfr::enabled())emit("native_ui_movie",')])
edit('Research/MenuProbe/TransitionProbe.cpp',[
 ('if(GetEnvironmentVariableW(L"QFR_UI_TRANSITIONS",value,2)!=1||value[0]!=L\'1\')return;','if(!Remaster::feature(Remaster::Feature::transitions,L"QFR_UI_TRANSITIONS"))return;')])
edit('Research/MenuCanvas/CanvasRuntime.cpp',[
 ('enabled=GetEnvironmentVariableW(L"QFR_UI_CANVAS",value,2)==1&&value[0]==L\'1\';','enabled=Remaster::feature(Remaster::Feature::canvas,L"QFR_UI_CANVAS");'),
 ('mainEnabled=GetEnvironmentVariableW(L"QFR_UI_MAIN_CANVAS",value,2)==1&&value[0]==L\'1\';','mainEnabled=Remaster::feature(Remaster::Feature::mainCanvas,L"QFR_UI_MAIN_CANVAS");'),
 ('enabled=true;Qfr::event("native_canvas_mode"','enabled=true;Remaster::ready(Remaster::canvasReady);Qfr::event("native_canvas_mode"')])
edit('Research/HudCaptain/HudCaptain.cpp',[
 ('if(GetEnvironmentVariableW(L"QFR_HUD_CAPTAINS",v,2)!=1||v[0]!=L\'1\')return;','if(!Remaster::feature(Remaster::Feature::captainTrial,L"QFR_HUD_CAPTAINS"))return;')])
edit('Research/HudAssembly/HudAssembly.cpp',[
 ('if(GetEnvironmentVariableW(L"QFR_HUD_ASSEMBLY",setting,2)!=1||setting[0]!=L\'1\')return;','if(!Remaster::feature(Remaster::Feature::hud,L"QFR_HUD_ASSEMBLY"))return;'),
 ('enabled=true;trace("native_hud_assembly_mode"','enabled=true;Remaster::ready(Remaster::hudReady);trace("native_hud_assembly_mode"')])
edit('Research/MenuCanvas/MainBackdrop.h',[
 ('if(GetEnvironmentVariableW(L"QFR_MAIN_BACKGROUND",env,2)!=1||env[0]!=L\'1\')return;','if(!Remaster::feature(Remaster::Feature::mainBackground,L"QFR_MAIN_BACKGROUND"))return;'),
 ('const bool receipt=!pixelReceipt&&','const bool receipt=Qfr::enabled()&&!pixelReceipt&&')])
edit('Research/OpeningFlow/OpeningFlow.cpp',[
 ('const bool hd=hdRequested&&(!known||large(g));','const bool hd=(hdRequested||Remaster::enabled())&&(!known||large(g));'),
 ('if(hd&&(!known||!sameThread()||!safetyInstalled||!hdProfile(pointer,g)))','if(hd&&(!known||!sameThread()||!safetyInstalled||!hdRequested||!hdProfile(pointer,g)))'),
 ('if(GetEnvironmentVariableW(L"QFR_OPENING_CARDS",env,2)!=1||env[0]!=L\'1\')return;','if(!Remaster::feature(Remaster::Feature::cards,L"QFR_OPENING_CARDS"))return;'),
 ('    for(auto& card:cards) {','    for(auto& card:cards) {\n        if(!Remaster::cardAllowed(card.resource))continue;'),
 ('if(GetEnvironmentVariableW(L"QFR_HD_MOVIE",v,2)!=1||v[0]!=L\'1\') {','if(!Remaster::feature(Remaster::Feature::hd,L"QFR_HD_MOVIE")&&!Remaster::enabled()) {'),
 ('    hdRequested=true;','    hdRequested=Remaster::feature(Remaster::Feature::hd,L"QFR_HD_MOVIE");'),
 ('||\n           !fileIdentity(std::filesystem::path(path).parent_path()/L"rs/Video/Paramount.bik",','||\n           (!Remaster::enabled()&&(!fileIdentity(std::filesystem::path(path).parent_path()/L"rs/Video/Paramount.bik",'),
 ('"c65bf6cb39b6838ab76f059152790bd0bf437d513c93b4c7b2bdd81d700dc851"))return false;','"c65bf6cb39b6838ab76f059152790bd0bf437d513c93b4c7b2bdd81d700dc851"))))return false;'),
 ('if(GetEnvironmentVariableW(L"QFR_OPENING_FLOW",v,2)!=1||v[0]!=L\'1\')return;','if(!Remaster::feature(Remaster::Feature::opening,L"QFR_OPENING_FLOW"))return;'),
 ('if(GetEnvironmentVariableW(L"QFR_OPENING_SUBMENUS",submenuValue,2)==1&&submenuValue[0]==L\'1\') {','if(Remaster::feature(Remaster::Feature::submenus,L"QFR_OPENING_SUBMENUS")) {'),
 ('        trace("native_opening_mode",','        if(!Remaster::enabled()||submenus)Remaster::ready(Remaster::openingReady);\n        trace("native_opening_mode",'),
 ('if(!owned->hdReceipts||frame==owned->profile->frames/2||(frame>=owned->profile->frames-5&&owned->hdReceipts<3)) {','if(Qfr::enabled()&&(!owned->hdReceipts||frame==owned->profile->frames/2||(frame>=owned->profile->frames-5&&owned->hdReceipts<3))) {')])
p=root/'DDrawCompat.vcxproj';s=p.read_text();anchor='    <ClCompile Include="Research\\OpeningFlow\\OpeningFlow.cpp" />';assert anchor in s;s=s.replace(anchor,anchor+'\n    <ClCompile Include="Remaster\\Runtime.cpp" />');p.write_text(s)
print('Wired production startup/feature gates/readiness and exact font argument provider; not built/validated')
