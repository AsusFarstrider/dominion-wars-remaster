#pragma once
// Optional opening-playtest evidence. The exception observer never handles an
// exception or waits for the recorder queue. Records are first-chance evidence,
// not an assertion that an exception was fatal.
#include <Windows.h>
#include <cstdint>
#include <filesystem>
namespace Qfr::Durability {
inline HANDLE exceptionFile=INVALID_HANDLE_VALUE;
inline PVOID handler=nullptr;
inline volatile LONG observed=0,writeFailures=0,writing=0;
inline bool active=false;
#pragma pack(push,1)
struct ExceptionRecord {
    uint32_t magic=0x31455851,size=sizeof(ExceptionRecord),version=1;
    uint32_t code=0,flags=0,thread=0,address=0,eip=0,esp=0,eax=0,ebx=0,ecx=0,edx=0,esi=0,edi=0,ebp=0;
    uint32_t parameterCount=0,parameters[4]{};
    uint64_t uptimeMs=0;
};
#pragma pack(pop)
static_assert(sizeof(ExceptionRecord)==92);
inline LONG CALLBACK observe(EXCEPTION_POINTERS* p) noexcept {
    if(!p||!p->ExceptionRecord||!p->ContextRecord)return EXCEPTION_CONTINUE_SEARCH;
    const auto code=p->ExceptionRecord->ExceptionCode;
    if(code!=EXCEPTION_ACCESS_VIOLATION&&code!=EXCEPTION_IN_PAGE_ERROR&&code!=EXCEPTION_ILLEGAL_INSTRUCTION&&
       code!=EXCEPTION_INT_DIVIDE_BY_ZERO&&code!=EXCEPTION_STACK_OVERFLOW&&code!=0xC0000374u&&code!=0xC0000409u)
        return EXCEPTION_CONTINUE_SEARCH;
    const DWORD error=GetLastError();const auto index=InterlockedIncrement(&observed);
    if(index<=64&&InterlockedCompareExchange(&writing,1,0)==0) {
        ExceptionRecord r;r.code=code;r.flags=p->ExceptionRecord->ExceptionFlags;r.thread=GetCurrentThreadId();
        r.address=static_cast<uint32_t>(reinterpret_cast<uintptr_t>(p->ExceptionRecord->ExceptionAddress));
        r.eip=p->ContextRecord->Eip;r.esp=p->ContextRecord->Esp;r.eax=p->ContextRecord->Eax;
        r.ebx=p->ContextRecord->Ebx;r.ecx=p->ContextRecord->Ecx;r.edx=p->ContextRecord->Edx;
        r.esi=p->ContextRecord->Esi;r.edi=p->ContextRecord->Edi;r.ebp=p->ContextRecord->Ebp;r.uptimeMs=GetTickCount64();
        r.parameterCount=p->ExceptionRecord->NumberParameters;
        for(unsigned i=0;i<4&&i<r.parameterCount;++i)r.parameters[i]=static_cast<uint32_t>(p->ExceptionRecord->ExceptionInformation[i]);
        DWORD count=0;
        if(!WriteFile(exceptionFile,&r,sizeof(r),&count,nullptr)||count!=sizeof(r)||!FlushFileBuffers(exceptionFile))
            InterlockedIncrement(&writeFailures);
        InterlockedExchange(&writing,0);
    } else InterlockedIncrement(&writeFailures);
    SetLastError(error);return EXCEPTION_CONTINUE_SEARCH;
}
inline void enable(const std::filesystem::path& root) noexcept {
    wchar_t v[2]{};
    active=GetEnvironmentVariableW(L"QFR_OPENING_FLOW",v,2)==1&&v[0]==L'1';
    if(!active)return;
    exceptionFile=CreateFileW((root/L"exceptions.bin").c_str(),GENERIC_WRITE,FILE_SHARE_READ,nullptr,CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH,nullptr);
    if(exceptionFile!=INVALID_HANDLE_VALUE)handler=AddVectoredExceptionHandler(1,observe);
}
}
