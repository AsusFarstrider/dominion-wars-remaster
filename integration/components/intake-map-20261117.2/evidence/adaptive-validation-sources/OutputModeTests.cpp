// Runs the actual startup installer/binder against owned synthetic executable
// pages at a mechanically relocated fixture base; never unmaps host-owned pages.
#include <Windows.h>
#include <Remaster/Runtime.h>
#include <Remaster/OutputMode.h>
#include "FixtureSites.h"
#include <iostream>
#include <stdexcept>
#include <vector>
#include <xmmintrin.h>
#include <float.h>
namespace {unsigned checks=0,protectCalls=0,flushCalls=0;int failProtect=0,failFlush=0;bool failAlloc=false;
void check(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
HMODULE WINAPI module(LPCWSTR){return reinterpret_cast<HMODULE>(0x20000000);}
LPVOID WINAPI allocate(LPVOID a,SIZE_T b,DWORD c,DWORD d){if(failAlloc)return nullptr;return ::VirtualAlloc(a,b,c,d);}
BOOL WINAPI protect(LPVOID a,SIZE_T b,DWORD c,PDWORD d){++protectCalls;if(int(protectCalls)==failProtect){SetLastError(ERROR_ACCESS_DENIED);return FALSE;}return ::VirtualProtect(a,b,c,d);}
BOOL WINAPI flush(HANDLE a,LPCVOID b,SIZE_T c){++flushCalls;if(int(flushCalls)==failFlush){SetLastError(ERROR_ACCESS_DENIED);return FALSE;}return ::FlushInstructionCache(a,b,c);}
}
#define GetModuleHandleW module
#define VirtualAlloc allocate
#define VirtualProtect protect
#define FlushInstructionCache flush
#include "OutputModeFixture.cpp"
#undef GetModuleHandleW
#undef VirtualAlloc
#undef VirtualProtect
#undef FlushInstructionCache
namespace Remaster {void note(const char*,const std::string&) noexcept{}}
namespace {
using namespace Remaster::Output;
unsigned char* image=nullptr;
constexpr size_t imageBytes=0xC0000;
void originalImage(){
 DWORD old=0;check(::VirtualProtect(image,imageBytes,PAGE_EXECUTE_READWRITE,&old)!=FALSE,"owned image writable");
 std::memset(image,0,imageBytes);
 auto* dos=reinterpret_cast<IMAGE_DOS_HEADER*>(image);dos->e_magic=IMAGE_DOS_SIGNATURE;dos->e_lfanew=0x100;
 auto* nt=reinterpret_cast<IMAGE_NT_HEADERS32*>(image+0x100);nt->Signature=IMAGE_NT_SIGNATURE;nt->FileHeader.Machine=IMAGE_FILE_MACHINE_I386;
 nt->OptionalHeader.Magic=IMAGE_NT_OPTIONAL_HDR32_MAGIC;nt->OptionalHeader.AddressOfEntryPoint=0xB085A;
 for(const auto& s:sites)std::memcpy(reinterpret_cast<void*>(s.address),s.expected.data(),s.length);
 std::memcpy(reinterpret_cast<void*>(entryAddress),entryBytes.data(),5);
 const unsigned char tail[]={0x83,0xC4,0x04,0x5D,0xB8,0x78,0x56,0x34,0x12,0xC3};
 std::memcpy(reinterpret_cast<void*>(entryAddress+5),tail,sizeof(tail));
 check(::VirtualProtect(image,imageBytes,PAGE_EXECUTE_READ,&old)!=FALSE,"owned image executable/read-only");
 check(::FlushInstructionCache(GetCurrentProcess(),image,imageBytes)!=FALSE,"owned image flush");
 protectCalls=flushCalls=0;failProtect=failFlush=0;failAlloc=false;installed=false;bound=false;session={};
}
void releaseTrampoline(){if(entryTrampoline){::VirtualFree(entryTrampoline,0,MEM_RELEASE);entryTrampoline=nullptr;}}
uint32_t operand(const Site& s){uint32_t v=0;std::memcpy(&v,reinterpret_cast<void*>(s.address+s.offset),4);return v;}
}
int main(){try{
 using namespace Remaster::Output;
 image=static_cast<unsigned char*>(::VirtualAlloc(reinterpret_cast<void*>(0x20000000),imageBytes,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE));
 check(image==reinterpret_cast<void*>(0x20000000),"synthetic relocated engine base reserved without changing any game");
 Remaster::managed=Remaster::valid=true;Remaster::config.dynamicOutput=true;
 for(const auto [w,h]:std::array<std::pair<const wchar_t*,const wchar_t*>,8>{{{L"1024",L"768"},{L"1280",L"800"},{L"1366",L"768"},{L"1920",L"1080"},{L"1920",L"1200"},{L"2560",L"1440"},{L"3840",L"2160"},{L"3440",L"1440"}}}){
  Size size{};check(parse(w,h,size),"matrix canonical launch size accepted");originalImage();
  check(bindOperands(size),"actual complete fifteen-operand transaction succeeds");
  for(const auto& s:sites)check(operand(s)==(s.axis==Axis::width?size.width:s.axis==Axis::height?size.height:projectionBits(size)),"every startup/reset/input/projection operand uses one size");
  check(!bound,"operand helper alone never publishes a session");
 }
 for(auto text:{L"",L"01920",L"+1920",L"1920 ",L"0",L"-1",L"999999"}){Size s{};check(!parse(text,L"1080",s),"malformed/partial width refused");}
 for(auto dims:std::array<Size,4>{{{799,600},{800,599},{8193,600},{800,8193}}})check(!valid(dims),"unsafe extent refused");
 originalImage();{
  DWORD old=0;::VirtualProtect(reinterpret_cast<void*>(sites[5].address),10,PAGE_EXECUTE_READWRITE,&old);
  *reinterpret_cast<unsigned char*>(sites[5].address)=0xCC;DWORD ignored=0;::VirtualProtect(reinterpret_cast<void*>(sites[5].address),10,old,&ignored);
  const uint32_t first=operand(sites[0]);check(!bindOperands({1920,1080})&&operand(sites[0])==first,"late recipe mismatch refuses before first operand write");}
 originalImage();failProtect=2;check(!bindOperands({1920,1080}),"protection setup failure refused");
 for(const auto& s:sites){uint32_t expected=0;std::memcpy(&expected,s.expected.data()+s.offset,4);check(operand(s)==expected,"protection setup failure writes no operands");}
 originalImage();failFlush=1;check(!bindOperands({1920,1080})&&!bound,"post-write cache failure never publishes a session");
 originalImage();check(!installStartup(false),"dynamic DLL loading cannot install an early binding");
 originalImage();failAlloc=true;check(!installStartup(true)&&!installed,"trampoline allocation failure refused");
 for(int failure:{1,2,3}){originalImage();failProtect=failure;const bool result=installStartup(true);check(!result&&!installed,"each startup protection failure refused");releaseTrampoline();}
 for(int failure:{1,2}){originalImage();failFlush=failure;check(!installStartup(true)&&!installed,"each startup cache failure refused");releaseTrampoline();}
 originalImage();{
  DEVMODEW mode{};mode.dmSize=sizeof(mode);check(EnumDisplaySettingsExW(nullptr,ENUM_CURRENT_SETTINGS,&mode,0)!=FALSE&&mode.dmPosition.x==0&&mode.dmPosition.y==0,"current primary mode available for real binder fixture");
  const auto width=std::to_wstring(mode.dmPelsWidth),height=std::to_wstring(mode.dmPelsHeight);
  wchar_t previousW[128]{},previousH[128]{};const auto nw=GetEnvironmentVariableW(L"DW_REMASTER_OUTPUT_WIDTH",previousW,128),nh=GetEnvironmentVariableW(L"DW_REMASTER_OUTPUT_HEIGHT",previousH,128);
  SetEnvironmentVariableW(L"DW_REMASTER_OUTPUT_WIDTH",width.c_str());SetEnvironmentVariableW(L"DW_REMASTER_OUTPUT_HEIGHT",height.c_str());
  check(installStartup(true)&&installed,"actual checked entry hook installs on synthetic CRT prologue");
  const auto csr=_mm_getcsr();const auto cw=_control87(0,0);SetLastError(0x1234ABCD);
  const auto result=reinterpret_cast<uint32_t(__cdecl*)()>(entryAddress)();const auto error=GetLastError();
  check(result==0x12345678&&bound&&session.width==mode.dmPelsWidth&&session.height==mode.dmPelsHeight,"real preserved-state bridge binds then jumps original trampoline/entry chain once");
  check(error==0x1234ABCD&&_mm_getcsr()==csr&&_control87(0,0)==cw,"startup bridge preserves LastError and FP/SSE controls");
  SetEnvironmentVariableW(L"DW_REMASTER_OUTPUT_WIDTH",nw?previousW:nullptr);SetEnvironmentVariableW(L"DW_REMASTER_OUTPUT_HEIGHT",nh?previousH:nullptr);
 }
 originalImage();releaseTrampoline();::VirtualFree(image,0,MEM_RELEASE);image=nullptr;
 std::cout<<"PASS "<<checks<<" actual early-entry/operand transaction/bridge assertions; synthetic executable pages, no game/display changes\n";
 }catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}}
