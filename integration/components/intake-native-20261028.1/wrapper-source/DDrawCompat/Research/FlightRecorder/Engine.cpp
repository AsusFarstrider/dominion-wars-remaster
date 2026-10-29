#include <Remaster/Runtime.h>
#include <Research/NativeProbe/Probe.h>
// Research entry observations. Registers/stack are raw evidence, not decoded arguments.
#include "Recorder.h"
#include <Windows.h>
#include <bcrypt.h>
#include <Common/Hook.h>
#include <fstream>
#include <vector>

namespace {
void* fontOriginal=reinterpret_cast<void*>(0x4A2650);
void* imageOriginal=reinterpret_cast<void*>(0x49F050);
void* dispatchOriginal=reinterpret_cast<void*>(0x40F900);
void* textureOriginal=reinterpret_cast<void*>(0x40F650);
const char* names[]={"font_entry","cpu_image_entry","image_dispatch_entry","textured_image_entry"};
void __stdcall engineEntry(unsigned which,const DWORD* frame) noexcept {
    const auto error=GetLastError();
    try {
    // pushad: EDI ESI EBP savedESP EBX EDX ECX EAX; next saved flags.
    DWORD registers[9]{};SIZE_T got=0;
    if(!ReadProcessMemory(GetCurrentProcess(),frame,registers,sizeof(registers),&got)||got!=sizeof(registers)){Qfr::gap("engine_entry","register_read_failed");SetLastError(error);return;}
    std::string values="[";
    for(unsigned i=0;i<9;++i){if(i)values+=",";values+=std::to_string(registers[i]);}values+="]";
    const auto fields=",\"name\":"+Qfr::quote(names[which])+",\"register_layout\":\"edi,esi,ebp,saved_esp,ebx,edx,ecx,eax,flags\",\"registers\":"+values;
    Qfr::event("engine_entry",fields);
    Qfr::memory("engine_stack",reinterpret_cast<void*>(registers[3]+4),128,fields+",\"interpretation\":\"32 raw entry stack DWORDs, beginning with return address\"");
    }catch(...){Qfr::gap("engine_entry","observation_failed");}
    SetLastError(error);
}
#define ENGINE_HOOK(name,index,target) \
__declspec(naked) void name(){ \
    __asm pushfd \
    __asm pushad \
    __asm mov eax,esp \
    __asm sub esp,528 \
    __asm and esp,0FFFFFFF0h \
    __asm fxsave [esp] \
    __asm mov [esp+512],eax \
    __asm fninit \
    __asm push eax \
    __asm push index \
    __asm call engineEntry \
    __asm fxrstor [esp] \
    __asm mov esp,[esp+512] \
    __asm popad \
    __asm popfd \
    __asm jmp dword ptr [target] \
}
ENGINE_HOOK(fontHook,0,fontOriginal)
ENGINE_HOOK(imageHook,1,imageOriginal)
ENGINE_HOOK(dispatchHook,2,dispatchOriginal)
ENGINE_HOOK(textureHook,3,textureOriginal)
#undef ENGINE_HOOK
using BinkCopy=INT (__stdcall*)(void*,void*,INT,UINT,UINT,UINT,UINT);
BinkCopy originalBink=nullptr;
INT __stdcall binkCopy(void* bink,void* dest,INT pitch,UINT height,UINT x,UINT y,UINT flags){
    Qfr::Call call("engine","BinkCopyToBuffer",reinterpret_cast<uintptr_t>(bink),[&]{return std::to_string(reinterpret_cast<uintptr_t>(dest))+","+std::to_string(pitch)+","+std::to_string(height)+","+std::to_string(x)+","+std::to_string(y)+","+std::to_string(flags);});
    const auto result=originalBink(bink,dest,pitch,height,x,y,flags);call.result(result);
    if(pitch>0 && uint64_t(pitch)*height<=32*1024*1024)Qfr::memory("movie_buffer",dest,size_t(pitch)*height,",\"pitch\":"+std::to_string(pitch)+",\"height\":"+std::to_string(height)+",\"x\":"+std::to_string(x)+",\"y\":"+std::to_string(y)+",\"flags\":"+std::to_string(flags)+",\"result\":"+std::to_string(result)+",\"pixel_layout\":\"Bink flags preserved; not inferred\"");
    else Qfr::gap("movie_buffer","unsupported_pitch_or_blob_limit");
    return result;
}
bool exactExe(){
    if(reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr))!=0x400000)return false;
    wchar_t path[32768]{};GetModuleFileNameW(nullptr,path,32768);
    std::ifstream f(path,std::ios::binary);std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(f)),{});
    if(bytes.empty()||bytes.size()>4*1024*1024)return false;unsigned char digest[32]{};
    if(BCryptHash(BCRYPT_SHA256_ALG_HANDLE,nullptr,0,bytes.data(),ULONG(bytes.size()),digest,32)<0)return false;
    const unsigned char expected[32]={0x77,0xa9,0x9e,0x3b,0xf0,0xa2,0xd8,0x70,0x4f,0x40,0x60,0xe5,0xce,0x58,0x45,0x14,0xda,0xa0,0x3b,0xb4,0x48,0x91,0xfd,0x33,0x36,0xa7,0xd6,0x64,0x60,0xef,0xe1,0x79};
    return memcmp(digest,expected,32)==0;
}
}
namespace Qfr {
void engineHooks(){
    if(Remaster::enabled())return; // Functional installation already ran without recorder dependence.
    if(NativeProbe::active()){NativeProbe::install();return;}
    if(!exactExe()){gap("engine_hooks","executable hash/base mismatch; raw renderer capture remains available");return;}
    Compat::hookFunction(fontOriginal,reinterpret_cast<void*>(&fontHook),"QUANTUM::font_entry");
    Compat::hookFunction(imageOriginal,reinterpret_cast<void*>(&imageHook),"QUANTUM::cpu_image_entry");
    Compat::hookFunction(dispatchOriginal,reinterpret_cast<void*>(&dispatchHook),"QUANTUM::image_dispatch_entry");
    Compat::hookFunction(textureOriginal,reinterpret_cast<void*>(&textureHook),"QUANTUM::textured_image_entry");
    originalBink=reinterpret_cast<BinkCopy>(Compat::hookIatFunction(GetModuleHandleW(nullptr),"_BinkCopyToBuffer@28",reinterpret_cast<void*>(&binkCopy)));
    event("engine_hook_coverage",",\"exe_hash_verified\":true,\"font_pc\":4859472,\"cpu_image_pc\":4845648,\"dispatch_pc\":4258048,\"textured_pc\":4257360,\"bink_iat\":"+std::string(originalBink?"true":"false"));
    gap("engine_semantics","font/image entry registers and stack captured; full argument, BOLT identity, CPU composition and hitbox semantics require further decoding");
    if(!originalBink)gap("movie_hook","Bink import was not found; movie pixel buffers unobserved");
}
}
