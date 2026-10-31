#include "Runtime.h"
#include "OutputMode.h"
#include <Research/NativeProbe/Probe.h>
#include <bcrypt.h>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <vector>
namespace Remaster {
namespace {
std::filesystem::path directory;
HANDLE ledger=INVALID_HANDLE_VALUE;
std::mutex ledgerMutex;unsigned ledgerEvents=0;
std::string quote(const std::string& s){std::string out="\"";for(unsigned char c:s){if(c=='"'||c=='\\')out+='\\';out+=c>=32&&c<127?char(c):'?';}return out+'"';}
std::vector<unsigned char> readFile(const std::filesystem::path& p,size_t maximum){std::ifstream f(p,std::ios::binary|std::ios::ate);
    if(!f)throw std::runtime_error("required package file unavailable");const auto size=f.tellg();
    if(size<=0||uint64_t(size)>maximum)throw std::runtime_error("package file size invalid");
    std::vector<unsigned char> bytes(static_cast<size_t>(size));f.seekg(0);if(!f.read(reinterpret_cast<char*>(bytes.data()),static_cast<std::streamsize>(bytes.size())))throw std::runtime_error("package file incomplete");return bytes;}
std::string hash(const std::vector<unsigned char>& bytes){unsigned char d[32]{};
    if(BCryptHash(BCRYPT_SHA256_ALG_HANDLE,nullptr,0,const_cast<PUCHAR>(bytes.data()),ULONG(bytes.size()),d,32)<0)throw std::runtime_error("package SHA failed");
    std::string out;const char* hex="0123456789abcdef";for(auto c:d){out+=hex[c>>4];out+=hex[c&15];}return out;}
std::string fileHash(const std::filesystem::path& p,size_t cap){return hash(readFile(p,cap));}
void openLedger(){wchar_t local[32768]{};const auto length=GetEnvironmentVariableW(L"LOCALAPPDATA",local,32768);
    if(!length||length>=32768||!std::filesystem::path(local).is_absolute())throw std::runtime_error("local application data unavailable for safety ledger");
    const auto folder=std::filesystem::path(local)/L"Dominion Wars Remastered/Rendering";std::filesystem::create_directories(folder);
    FILETIME t{};GetSystemTimeAsFileTime(&t);ULARGE_INTEGER stamp{};stamp.LowPart=t.dwLowDateTime;stamp.HighPart=t.dwHighDateTime;
    for(unsigned attempt=0;attempt<5;++attempt){const auto path=folder/(L"rendering-"+std::to_wstring(GetCurrentProcessId())+L"-"+std::to_wstring(stamp.QuadPart)+L"-"+std::to_wstring(attempt)+L".ndjson");
        ledger=CreateFileW(path.c_str(),GENERIC_WRITE,FILE_SHARE_READ,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
        if(ledger!=INVALID_HANDLE_VALUE||GetLastError()!=ERROR_FILE_EXISTS)break;}
    if(ledger==INVALID_HANDLE_VALUE)throw std::runtime_error("production safety ledger unavailable");}
bool movie(const wchar_t* name,bool permit,const char* original,const char* replacement){const auto actual=fileHash(directory/L"rs/Video"/name,128*1024*1024);
    return approvedMovie(actual,permit,original,replacement);}
}
void note(const char* kind,const std::string& detail) noexcept {if(!managed)return;const DWORD old=GetLastError();
    try{std::lock_guard lock(ledgerMutex);if(ledger!=INVALID_HANDLE_VALUE&&ledgerEvents<128){
        const auto line="{\"event\":"+quote(kind)+",\"detail\":"+quote(detail.substr(0,1024))+",\"pid\":"+std::to_string(GetCurrentProcessId())+"}\n";
        DWORD wrote=0;if(WriteFile(ledger,line.data(),DWORD(line.size()),&wrote,nullptr)&&wrote==line.size()){
            ++ledgerEvents;if(!FlushFileBuffers(ledger))OutputDebugStringA("Dominion Wars Rendering safety ledger flush failed");
        }else OutputDebugStringA("Dominion Wars Rendering safety ledger write failed");
    }else OutputDebugStringA((std::string("Dominion Wars Rendering: ")+kind+" "+detail).c_str());}catch(...){}
    SetLastError(old);
}
bool load() noexcept {const DWORD old=GetLastError();bool ok=true;
    try{wchar_t path[32768]{};if(!GetModuleFileNameW(nullptr,path,32768))throw std::runtime_error("module filename unavailable");
        directory=std::filesystem::path(path).parent_path();const auto configFile=directory/L"Remaster.Rendering.json";
        if(std::filesystem::exists(configFile)){
            managed=true;openLedger();const auto bytes=readFile(configFile,16384);config=options(std::string(bytes.begin(),bytes.end()));
            if(reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr))!=0x400000||fileHash(path,4*1024*1024)!=config.engine)throw std::runtime_error("managed native engine identity/base mismatch");
            const auto identityBytes=readFile(directory/L"version.json",16384);
            version=boundIdentity(config,std::string(identityBytes.begin(),identityBytes.end()),hash(identityBytes));
            if(fileHash(directory/L"binkw32.dll",16*1024*1024)!="65358b3cde7281df10cb68ac8e1c9f8705893a47df33f87bb882073fd0955c90")throw std::runtime_error("managed codec identity mismatch");
            paramountHD=movie(L"Paramount.bik",config.paramount,"963dba645c1489ea05aa23468fb81074791182c3bc92aa114d10ea9719e4f111","5a3506e3f48190073591c1349f4e35dbfe8dde9d2ded19364fa865817deecff3");
            gizmoHD=movie(L"GizmoLogo.bik",config.gizmo,"c80976eacec03c9dbe032b46524aaa99f2686d155bb846b34711e1a67d003e7e","b506781006eddd97fe6f86108d67259639f8044cbd5bcf71fcd0734a14013688");
            ssHD=movie(L"SSLogo1.bik",config.ss,"e444e02185d1fd7d7c7be5edf3bd2842cd43cfe9a89e715a2542ce59c0f7249e","910eb6d55755c72c67a557138634fc344fff25560cba7e267899ed9a9904bc50");
            valid=true;note("configuration_validated",version.label+"; build="+version.build+"; source="+version.revision+"; flavor="+version.flavor+"; recording="+(config.recording?"compact":"off"));
        }
    }catch(const std::exception& e){valid=false;ok=false;note("configuration_refused",e.what());}catch(...){valid=false;ok=false;note("configuration_refused","unknown exception");}
    SetLastError(old);return ok;
}
void initialize() noexcept {if(!enabled())return;const DWORD old=GetLastError();static std::once_flag once;
    try{std::call_once(once,[]{DEVMODEW mode{};mode.dmSize=sizeof(mode);
        const auto width=config.dynamicOutput?Output::session.width:2560u,height=config.dynamicOutput?Output::session.height:1440u;
        if((config.dynamicOutput&&!Output::bound)||!EnumDisplaySettingsW(nullptr,ENUM_CURRENT_SETTINGS,&mode)||mode.dmPelsWidth!=width||mode.dmPelsHeight!=height||mode.dmPosition.x||mode.dmPosition.y){
            note("output_refused","physical primary mode differs from immutable launch binding; no desktop mode change attempted");
            MessageBoxW(nullptr,L"The primary display changed or does not match this rendering profile. Restart with a supported launch configuration. No desktop mode was changed.",L"Dominion Wars Remastered",MB_OK|MB_ICONERROR);ExitProcess(0x4E10);}
        NativeProbe::install();if(!functionalReady()){note("startup_refused","functional hook set incomplete");ExitProcess(0x4E11);}
        note("functional_ready","native/UI/canvas/opening/HUD active independently of recording");
    });}catch(...){note("startup_refused","initialization exception");ExitProcess(0x4E12);}
    SetLastError(old);
}
}
