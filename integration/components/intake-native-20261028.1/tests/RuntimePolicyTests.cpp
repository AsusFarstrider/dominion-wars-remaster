#include <Remaster/Identity.h>
#include <Remaster/VersionLabel.h>
#include <cstring>
#include <iostream>
#include <map>
#include <vector>
namespace {unsigned checks=0,notes=0,writes=0;std::string lastNote,lastDetail;
void check(bool v,const char* why){++checks;if(!v)throw std::runtime_error(why);}
template<class F>void rejects(F f,const char* why){bool bad=false;try{f();}catch(...){bad=true;}check(bad,why);}
std::string replace(std::string s,const std::string& a,const std::string& b){const auto p=s.find(a);if(p==s.npos)throw std::runtime_error("fixture anchor");s.replace(p,a.size(),b);return s;}
const std::string goodIdentity=R"({"schema":1,"product":"Dominion Wars Remastered","original_game_version":"1.04","remaster_version":"1.10","release_stage":"alpha","integration_build_id":"int-20261027.1","source_revision":"0123456789012345678901234567890123456789","flavor":"native-1440","version_decision_id":"milestone-1.10","in_game_label":"Remastered 1.10"})";
const std::string goodOptions=R"({"schema":1,"profile":"native-1440-v1","engine_sha256":"de3e3ede7a9b4ce65a891201547f36413f3a880e4df3f28fb8a81e51f687abc6","version_sha256":"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa","recording":"off","paramount_hd":false,"gizmo_hd":false,"publisher_card":false,"title_card":false,"main_background":false,"ss_hd":false,"main_buttons":false})";
std::map<uint32_t,std::vector<uint8_t>> memory;
auto region(uint32_t a){auto i=memory.upper_bound(a);if(i==memory.begin())return memory.end();--i;return uint64_t(a)-i->first<i->second.size()?i:memory.end();}
bool rd(uint32_t a,void* p,size_t n){auto i=region(a);if(i==memory.end()||uint64_t(a)-i->first+n>i->second.size())return false;memcpy(p,i->second.data()+a-i->first,n);return true;}
bool wr(uint32_t a,const void* p,size_t n){auto i=region(a);if(i==memory.end()||uint64_t(a)-i->first+n>i->second.size())return false;memcpy(i->second.data()+a-i->first,p,n);++writes;return true;}
void reserve(uint32_t a,size_t n){memory[a]=std::vector<uint8_t>(n);}
template<class T>void put(uint32_t a,T value){if(!wr(a,&value,sizeof(value)))throw std::runtime_error("fixture write");}
void scalar(uint32_t a,uint32_t v){reserve(a,4);put(a,v);}
template<class T>T get(uint32_t a){T v{};if(!rd(a,&v,sizeof(v)))throw std::runtime_error("fixture read");return v;}
struct Fixture {
 static constexpr uint32_t dest=0x71000000,state=0x71001000,font=0x71002000,argument=0x71003000,oldText=0x4D0A38;
 Fixture(){memory.clear();Remaster::managed=Remaster::valid=true;Remaster::config=Remaster::options(goodOptions);Remaster::version=Remaster::identity(goodIdentity);
  Remaster::VersionLabel::logged=Remaster::VersionLabel::rejected=false;notes=0;lastNote.clear();
  scalar(0x4D0A24,7);reserve(0x4E47C0,0x90);put(0x4E47C0,uint32_t{0x405});put(0x4E47C4,uint32_t{0x84});put(0x4E47C0+0x77,uint32_t{0x5F7ED8});
  put(0x4E47C0+0x4C,uint32_t{0x46ADD0});put(0x4E47C0+0x50,uint32_t{0x201});scalar(0xA39B64,0x71004000);scalar(0x71004000,dest);
  scalar(0x4D09F4,oldText);reserve(oldText,13);wr(oldText,"Version 1.04",13);scalar(0x4EC3B8,state);
  reserve(state,sizeof(QFontStateResearch));QFontStateResearch s{};s.font=font;s.pen_x=700;s.pen_y=580;put(state,s);
  reserve(font,12+94*12);QFontHeaderResearch h{};h.first_code=32;h.last_code=125;h.pixel_bits=8;h.tracking=1;h.glyph_count=94;put(font,h);
  for(unsigned c=0;c<94;++c){QGlyphResearch g{};g.width=4;g.height=9;put(font+12+c*12,g);}
  scalar(argument,oldText);writes=0;
 }
 bool call(uint32_t caller=0x46AEB9,uint32_t destination=dest){return Remaster::VersionLabel::substitute(caller,destination,oldText,argument,rd,wr);}
};
void tests(){using namespace Remaster;
 const auto identityRecord=identity(goodIdentity);check(identityRecord.label=="Remastered 1.10"&&identityRecord.version=="1.10"&&identityRecord.stage=="alpha","exact packaged two-digit version");
 check(!approvedMovie("original",false,"original","HD")&&!approvedMovie("original",true,"original","HD"),"original movies remain admitted regardless of optional override permission");
 check(approvedMovie("HD",true,"original","HD"),"exact explicitly permitted HD profile admitted");
 rejects([]{approvedMovie("HD",false,"original","HD");},"installed HD without permission fails closed");
 rejects([]{approvedMovie("unknown",true,"original","HD");},"unknown media cannot use permissive HD fallback");
 const auto o=options(goodOptions);check(!o.recording&&!o.paramount&&!o.gizmo&&!o.main&&!o.publisher&&!o.title&&!o.ss&&!o.buttons,"original-art core profile defaults");
 check(boundIdentity(o,goodIdentity,std::string(64,'a')).label=="Remastered 1.10","exact configured identity digest permits validated package label");
 rejects([&]{boundIdentity(o,goodIdentity,std::string(64,'b'));},"well-formed but unbound version metadata is refused");
 check(options(replace(goodOptions,"\"off\"","\"compact\"")).recording,"explicit compact diagnostic package");
 for(const auto& pair:std::vector<std::pair<std::string,std::string>>{{"\"1.10\"","\"1.1\""},{"\"1.04\"","\"1.03\""},{"\"alpha\"","\"gamma\""},{"int-20261027.1","int-20261027.0"},{"Remastered 1.10","Remastered 1.11"},{"\"schema\":1","\"schema\":true"},{"\"native-1440\"","\" \""}})
  rejects([&]{identity(replace(goodIdentity,pair.first,pair.second));},"malformed/mismatched identity rejected");
 check(identity(replace(replace(goodIdentity,"\"remaster_version\":\"1.10\"","\"remaster_version\":null"),"Remastered 1.10","Version 1.04")).version.empty(),"unassigned original label retains official base");
 for(const auto& pair:std::vector<std::pair<std::string,std::string>>{{"native-1440-v1","native-1080-v1"},{"\"off\"","\"on\""},{"\"paramount_hd\":false","\"paramount_hd\":1"},{"de3e3ede","ae3e3ede"},{"\"schema\":1","\"schema\":1.0"}})
  rejects([&]{options(replace(goodOptions,pair.first,pair.second));},"unsupported runtime config rejected");
 for(const auto& s:std::vector<std::string>{goodOptions+"garbage",goodOptions.substr(0,goodOptions.size()-1)+",\"schema\":1}",
  replace(goodOptions,"\"recording\":\"off\"","\"recording\":{}"),replace(goodOptions,"\"recording\":\"off\"","\"unknown\":\"off\""),std::string(16385,' '),"{\"x\":\"\\u0000\"}"})
  rejects([&]{options(s);},"invalid flat JSON does not become a fallback profile");
 check(text(FlatJson("{\"label\":\"Remastered \\u0031.10\"}").parse(),"label")=="Remastered 1.10","bounded ASCII JSON unicode escapes supported");
 {Fixture f;check(f.call()&&writes==1,"exact Main label substitutes stack argument once");
  check(get<uint32_t>(Fixture::argument)==uint32_t(reinterpret_cast<uintptr_t>(version.label.c_str())),"stable owned identity string reaches original call");
  char prior[13]{};check(rd(Fixture::oldText,prior,13)&&!memcmp(prior,"Version 1.04",13),"original EXE text is untouched");
  check(notes==1&&lastNote=="version_label_substituted","production label evidence independent of recorder");check(f.call()&&notes==1,"repaint replaces once per original call with bounded ledger");}
 {Fixture f;
  // Exact selected014/004 glyph metrics independently pinned by QUANTUM;
  // no original glyph pixels are included in this fixture.
  const char* letters="Remast rd10.";const unsigned widths[]={3,3,5,3,3,3,4,3,3,2,3,1};
  for(unsigned i=0;i<12;++i){QGlyphResearch g{};g.width=uint16_t(widths[i]);g.height=letters[i]==' '||letters[i]=='.'?1:9;if(letters[i]=='.'){g.origin_x=1;g.origin_y=8;}put(Fixture::font+12+(letters[i]-32)*12,g);}
  writes=0;check(f.call()&&lastDetail.find("700,580,759,589; measured_width=59; final_pen=760")!=std::string::npos,"actual original font metric facts fit the new label without moving it");
 }
 {Fixture f;check(!f.call(0x46AEB8)&&!writes&&!notes,"unrelated callsite replays untouched");}
 {Fixture f;check(!f.call(0x46AEB9,Fixture::dest+4)&&!writes,"wrong target cannot replace label");}
 {Fixture f;put(0x4D0A24,uint32_t{12});writes=0;check(!f.call()&&!writes,"nonMain owner cannot replace label");}
 {Fixture f;put(0x4E47C0+0x50,uint32_t{0x202});writes=0;check(!f.call()&&!writes,"changed original Main resource cannot replace label");}
 {Fixture f;put(0x4EC3B8,uint32_t{0x4EC3B8});writes=0;check(!f.call()&&!writes,"font pointer slot cannot be mistaken for inline state");}
 {Fixture f;auto state=get<QFontStateResearch>(Fixture::state);state.alignment=1;put(Fixture::state,state);writes=0;check(!f.call()&&!writes,"unproved style is refused");}
 {Fixture f;version.label=std::string(79,'R');check(!f.call()&&!writes,"storage bound is not mistaken for visual width budget");}
 {Fixture f;auto h=get<QFontHeaderResearch>(Fixture::font);h.tracking=-1;put(Fixture::font,h);writes=0;check(!f.call()&&!writes,"font header contract mismatch is refused");}
 {Fixture f;managed=true;valid=false;SetEnvironmentVariableW(L"QFR_UI_CANVAS",L"1");check(!feature(Feature::canvas,L"QFR_UI_CANVAS")&&!recordingAllowed(),"invalid-present managed profile cannot use environment fallback");
  valid=true;config.recording=false;SetEnvironmentVariableW(L"QFR_DIR",L"inherited");check(feature(Feature::canvas,L"QFR_UI_CANVAS")&&!recordingAllowed(),"production rendering works with recorder disallowed");
  check(!feature(Feature::captainTrial,L"QFR_UI_CANVAS"),"temporary captain trial cannot leak into production");
  check(feature(Feature::transitions,L"QFR_UI_TRANSITIONS"),"functional transition hooks remain enabled with recording off");
  readiness=0;check(!functionalReady(),"empty functional hook set not ready");ready(nativeReady);ready(uiReady);ready(canvasReady);ready(openingReady);check(!functionalReady(),"partial hook set not ready");ready(hudReady);check(!functionalReady(),"missing functional transition hooks block readiness");ready(transitionReady);check(functionalReady(),"complete functional set ready independently of capture");
  SetEnvironmentVariableW(L"QFR_DIR",nullptr);SetEnvironmentVariableW(L"QFR_UI_CANVAS",nullptr);}
}
}
namespace Remaster {void note(const char* kind,const std::string& detail) noexcept{++notes;lastNote=kind;lastDetail=detail;}}
int main(){try{tests();std::cout<<"PASS "<<checks<<" runtime identity/feature/label assertions; no hooks/game/hardware executed\n";}catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}}
