#include <Research/OpeningFlow/OpeningCore.h>
#include <Research/FlightRecorder/Recorder.h>
#include <Research/NativeProbe/Probe.h>
#include <Research/MenuProbe/Probe.h>
#include <Windows.h>
#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
namespace NativeProbe {
bool active()noexcept{return false;}
bool allowEvent(const char*,const std::string&)noexcept{return true;}
bool allowCall(const char*,const char*)noexcept{return true;}
}
namespace MenuProbe {std::string annotate(const char*,const std::string& s)noexcept{return s;}}
static unsigned checks=0;
void check(bool ok){++checks;if(!ok)throw std::runtime_error("check"+std::to_string(checks));}
void handledException(){__try{RaiseException(EXCEPTION_ACCESS_VIOLATION,0,0,nullptr);}__except(EXCEPTION_EXECUTE_HANDLER){}}
int main(int argc,char**) {
    try {
        SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
        auto c=std::make_unique<OpeningFlow::Canvas>();check(c->intact());check(reinterpret_cast<uintptr_t>(c->data())%32==0);
        OpeningFlow::MovieGeometry g{640,480,640,480,0};check(g.supports(480,80,60,10));
        check(!g.supports(600,80,60,10));check(!g.supports(480,161,60,10));check(!g.supports(480,80,121,10));
        check(!g.supports(480,80,60,11));g.handleFlags=0x80000;check(!g.supports(480,80,60,10));
        g.handleFlags=0x10000000;check(!g.supports(480,80,60,10));g.handleFlags=0;g.internalWidth=320;check(!g.supports(480,80,60,10));
        std::vector<uint16_t> source(1024*768,0x1234);source[1024]=0x4321;
        check(c->snapshot({source.data(),source.size()*2,2048,1024,768}));check(c->data()[800]==0x4321);
        check(!c->snapshot({source.data(),10,2048,1024,768}));
        auto plan=MenuCanvas::Plan::fit(800,600,2560,1440);check(bool(plan));
        std::vector<uint16_t> native(2560*1440,0xEEEE);
        MenuCanvas::WriteView output{native.data(),native.size()*2,5120,2560,1440};
        check(c->fit(*plan,output));check(native[0]==0);check(native[320]==0x1234);
        // Partial update touches only one pixel; composition never overwrites
        // the retained original history that the next converter depends on.
        c->data()[801]=0x9999;check(c->fit(*plan,output));
        check(c->data()[0]==0x1234&&c->data()[800]==0x4321&&c->data()[801]==0x9999);
        const auto history=std::make_unique<decltype(c->storage)>(c->storage);
        check(c->fit(*plan,output));check(c->storage==*history); // skipped-copy repeat
        check(c->legacy(output));check(native[0]==0x1234&&native[2560]==0x4321&&native[2561]==0x9999&&native[800]==0);
        c->storage[0]=0;check(!c->intact());check(!c->fit(*plan,output));
        Qfr::start();check(Qfr::enabled());
        for(unsigned i=0;i<1000;++i)Qfr::event("native_fixture_context",",\"index\":"+std::to_string(i)+",\"state\":12");
        if(argc>1) {
            wchar_t path[32768]{};GetEnvironmentVariableW(L"QFR_DIR",path,32768);
            const auto journal=std::filesystem::path(path)/"durable.ndjson";const auto deadline=GetTickCount64()+5000;
            while(GetTickCount64()<deadline) {
                if(std::filesystem::exists(journal)&&std::filesystem::file_size(journal)>0)break;
                Sleep(10);
            }
            check(std::filesystem::exists(journal)&&std::filesystem::file_size(journal)>0);
            handledException();Qfr::event("native_fixture_context",",\"index\":1001,\"state\":12");
            TerminateProcess(GetCurrentProcess(),77);return 2;
        }
        Sleep(650);Qfr::stop("opening_fixture_complete");check(!Qfr::enabled());
        std::cout<<"PASS "<<checks<<" opening-buffer/durability fixture assertions\n";return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';Qfr::stop("fixture_failure");return 1;}
}
