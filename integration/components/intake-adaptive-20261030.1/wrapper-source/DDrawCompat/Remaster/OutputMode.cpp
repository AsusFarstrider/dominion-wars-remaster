#include "Runtime.h"
#include "OutputMode.h"
#include "OutputSites.h"
#include <Windows.h>
#include <array>
#include <cstring>
#include <limits>

namespace Remaster::Output {
namespace {
constexpr uintptr_t entryAddress=0x4B085A;
constexpr std::array<unsigned char,5> entryBytes{0x55,0x8B,0xEC,0x6A,0xFF};
void* entryTrampoline=nullptr;
bool installed=false;
struct LastError {DWORD value=GetLastError();~LastError(){SetLastError(value);}};
bool read(uintptr_t address,void* dest,size_t bytes) noexcept {
    SIZE_T got=0;return ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<void*>(address),dest,bytes,&got)&&got==bytes;
}
bool relative(uintptr_t from,uintptr_t to,int32_t& result) noexcept {
    const auto delta=int64_t(to)-int64_t(from);
    if(delta<std::numeric_limits<int32_t>::min()||delta>std::numeric_limits<int32_t>::max())return false;
    result=int32_t(delta);return true;
}
bool jump(unsigned char* dest,uintptr_t address,uintptr_t target) noexcept {
    int32_t delta=0;if(!relative(address+5,target,delta))return false;
    dest[0]=0xE9;std::memcpy(dest+1,&delta,4);return true;
}
[[noreturn]] void fail(const char* reason) noexcept {
    note("native_output_refused",reason);
    MessageBoxW(nullptr,L"The selected display could not be bound safely before game startup. See the Rendering diagnostic ledger. No desktop mode was changed.",L"Dominion Wars Remastered",MB_OK|MB_ICONERROR);
    ExitProcess(0x4E20);
}
struct Page {uintptr_t address=0;DWORD old=0;bool changed=false;};
bool bindOperands(Size size) noexcept {
    // Prepare and validate the complete recipe before changing any instruction.
    std::array<uint32_t,sites.size()> values{};
    std::array<Page,32> pages{};size_t count=0;
    SYSTEM_INFO system{};GetSystemInfo(&system);const auto granule=uintptr_t(system.dwPageSize);
    if(!granule||(granule&(granule-1)))return false;
    for(size_t i=0;i<sites.size();++i){const auto& s=sites[i];std::array<unsigned char,10> current{};
        if(s.length>current.size()||s.offset+4>s.length||!read(s.address,current.data(),s.length)||
           std::memcmp(current.data(),s.expected.data(),s.length))return false;
        values[i]=s.axis==Axis::width?size.width:s.axis==Axis::height?size.height:projectionBits(size);
        const auto first=uintptr_t(s.address)&~(granule-1),last=(uintptr_t(s.address)+s.length-1)&~(granule-1);
        for(auto address=first;address<=last;address+=granule){
            bool present=false;for(size_t n=0;n<count;++n)present|=pages[n].address==address;
            if(!present){if(count==pages.size())return false;pages[count++].address=address;}
        }
    }
    const auto restore=[&]() noexcept {bool ok=true;for(size_t i=count;i>0;--i)if(pages[i-1].changed){DWORD ignored=0;
        if(!VirtualProtect(reinterpret_cast<void*>(pages[i-1].address),granule,pages[i-1].old,&ignored))ok=false;
    }return ok;};
    for(size_t i=0;i<count;++i){if(!VirtualProtect(reinterpret_cast<void*>(pages[i].address),granule,PAGE_EXECUTE_READWRITE,&pages[i].old)){restore();return false;}pages[i].changed=true;}
    // Once writes begin, any subsequent failure is fatal before the original CRT
    // entry is resumed. Never run the game with a partially admitted contract.
    for(size_t i=0;i<sites.size();++i)std::memcpy(reinterpret_cast<void*>(sites[i].address+sites[i].offset),&values[i],4);
    const bool flushed=FlushInstructionCache(GetCurrentProcess(),nullptr,0)!=FALSE;
    const bool protectedAgain=restore();
    if(!flushed||!protectedAgain)return false;
    for(size_t i=0;i<sites.size();++i){uint32_t actual=0;if(!read(sites[i].address+sites[i].offset,&actual,4)||actual!=values[i])return false;}
    return true;
}
void __stdcall bindStartup() noexcept {
    LastError preserve;
    try {
        if(!installed||bound||!enabled()||!config.dynamicOutput)fail("startup binding lifetime/profile");
        wchar_t width[16]{},height[16]{};
        const auto w=GetEnvironmentVariableW(L"DW_REMASTER_OUTPUT_WIDTH",width,16),h=GetEnvironmentVariableW(L"DW_REMASTER_OUTPUT_HEIGHT",height,16);
        Size selected{};
        if(!w||!h||w>=16||h>=16||!parse({width,w},{height,h},selected))fail("canonical launch width/height missing or unsupported");
        DEVMODEW mode{};mode.dmSize=sizeof(mode);
        if(!EnumDisplaySettingsExW(nullptr,ENUM_CURRENT_SETTINGS,&mode,0)||mode.dmPosition.x||mode.dmPosition.y||
           mode.dmPelsWidth!=selected.width||mode.dmPelsHeight!=selected.height)fail("launch dimensions no longer match physical primary display");
        if(!bindOperands(selected))fail("audited startup operand transaction failed");
        session=selected;bound=true;
        note("native_output_bound","width="+std::to_string(session.width)+"; height="+std::to_string(session.height)+"; physical-primary=0,0; sites=15; before EXE CRT; disk engine unchanged");
    }catch(...){fail("startup binding exception");}
}
__declspec(naked) void startupBridge() {
    __asm pushfd
    __asm pushad
    __asm mov eax,esp
    __asm sub esp,528
    __asm and esp,0FFFFFFF0h
    __asm fxsave [esp]
    __asm mov [esp+512],eax
    __asm fninit
    __asm call bindStartup
    __asm fxrstor [esp]
    __asm mov esp,[esp+512]
    __asm popad
    __asm popfd
    __asm jmp dword ptr [entryTrampoline]
}
}
bool installStartup(bool staticLoad) noexcept {
    if(!enabled()||!config.dynamicOutput)return true;
    LastError preserve;
    if(installed)return false;
    if(!staticLoad||reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr))!=0x400000)return false;
    IMAGE_DOS_HEADER dos{};IMAGE_NT_HEADERS32 nt{};
    if(!read(0x400000,&dos,sizeof(dos))||dos.e_magic!=IMAGE_DOS_SIGNATURE||dos.e_lfanew<0||dos.e_lfanew>0x1000||
       !read(0x400000+uintptr_t(dos.e_lfanew),&nt,sizeof(nt))||nt.Signature!=IMAGE_NT_SIGNATURE||
       nt.FileHeader.Machine!=IMAGE_FILE_MACHINE_I386||nt.OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC||
       0x400000+nt.OptionalHeader.AddressOfEntryPoint!=entryAddress||nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size)return false;
    std::array<unsigned char,5> current{},patch{};
    if(!read(entryAddress,current.data(),current.size())||current!=entryBytes||
       !jump(patch.data(),entryAddress,reinterpret_cast<uintptr_t>(&startupBridge)))return false;
    auto* block=static_cast<unsigned char*>(VirtualAlloc(nullptr,32,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE));
    if(!block)return false;
    std::memcpy(block,entryBytes.data(),5);
    if(!jump(block+5,reinterpret_cast<uintptr_t>(block)+5,entryAddress+5)){VirtualFree(block,0,MEM_RELEASE);return false;}
    DWORD previous=0;
    if(!VirtualProtect(block,32,PAGE_EXECUTE_READ,&previous)||!FlushInstructionCache(GetCurrentProcess(),block,32)){VirtualFree(block,0,MEM_RELEASE);return false;}
    std::array<unsigned char,10> expected{},verify{};std::memcpy(expected.data(),block,10);
    DWORD old=0;
    if(!VirtualProtect(reinterpret_cast<void*>(entryAddress),5,PAGE_EXECUTE_READWRITE,&old)){VirtualFree(block,0,MEM_RELEASE);return false;}
    entryTrampoline=block;std::memcpy(reinterpret_cast<void*>(entryAddress),patch.data(),5);
    const bool flushed=FlushInstructionCache(GetCurrentProcess(),reinterpret_cast<void*>(entryAddress),5)!=FALSE;
    DWORD ignored=0;const bool protectedAgain=VirtualProtect(reinterpret_cast<void*>(entryAddress),5,old,&ignored)!=FALSE;
    if(!flushed||!protectedAgain||!read(entryAddress,current.data(),5)||current!=patch||!read(reinterpret_cast<uintptr_t>(block),verify.data(),10)||verify!=expected){
        // DllMain returns FALSE: process initialization aborts. Keep the valid
        // trampoline allocated if the entry may reference it; never free a live
        // branch target or claim a successful partial installation.
        return false;
    }
    installed=true;note("native_output_startup_ready","checked CRT-entry bridge; one immutable launch binding");return true;
}
}
