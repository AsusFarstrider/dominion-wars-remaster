// Execute the actual transition observer with recording disabled; hook/graphics
// callees are stubs. This is not a game or hardware test.
#include <Windows.h>
#include <iostream>
#include <stdexcept>
#include <thread>
#include "TransitionProbe.cpp"
namespace {unsigned prepared=0,boundaries=0,events=0,checks=0;
void check(bool v,const char* why){++checks;if(!v)throw std::runtime_error(why);}}
namespace Qfr {
bool enabled() noexcept{return false;}
std::string quote(const std::string& s){return '"'+s+'"';}
void event(const char*,const std::string&) noexcept{++events;}
}
namespace HudAssembly {void prepareFrame() noexcept{++prepared;}}
namespace OpeningFlow {void resourceBoundary(const char* reason) noexcept{if(std::string(reason)=="state_install")++boundaries;}}
namespace Compat {void hookFunction(void*&,void*,const char*){throw std::runtime_error("fixture cannot install hooks");}}
int main(){try{
 Remaster::managed=Remaster::valid=true;Remaster::config.recording=false;
 check(Remaster::feature(Remaster::Feature::transitions,L"QFR_UI_TRANSITIONS"),"production transition installation must not depend on recording");
 MenuTransition::enabled=true;MenuTransition::owner=0;DWORD saved[9]{};
 MenuTransition::observe(0,saved);check(prepared==1&&boundaries==0,"frame preparation runs once with recorder disabled");
 MenuTransition::observe(4,saved);check(prepared==1&&boundaries==1,"state install invalidates opening ownership with recorder disabled");
 for(unsigned i:{1u,2u,3u,5u})MenuTransition::observe(i,saved);
 check(prepared==1&&boundaries==1&&!MenuTransition::snapshots.load()&&!MenuTransition::ownerFrames.depth(),"other phases neither replay functional calls nor create capture history");
 check(events==0,"routine record-off transitions emit no diagnostic events");
 std::thread foreign([&]{MenuTransition::observe(0,saved);MenuTransition::observe(4,saved);});foreign.join();
 check(prepared==1&&boundaries==1,"foreign observer cannot prepare or mutate owned render state");
 MenuTransition::observe(0,saved);MenuTransition::observe(4,saved);
 check(prepared==2&&boundaries==2,"functional actions remain active across repeated transitions");
 std::cout<<"PASS "<<checks<<" actual transition observer assertions with recording disabled; no game/hooks/hardware\n";
 }catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}}
