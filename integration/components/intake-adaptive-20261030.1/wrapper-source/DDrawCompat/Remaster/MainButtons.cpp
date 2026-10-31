#include "MainButtons.h"
#include <Research/MenuCanvas/MainBackdrop.h>
#include <Research/MenuProbe/Bridge.h>
#include <Common/Hook.h>
#include <bcrypt.h>
#include <filesystem>
#include <fstream>
namespace Remaster::MainButtons {
namespace {
void* original[]={reinterpret_cast<void*>(0x46B540)};
void __stdcall observe(unsigned,const DWORD* saved)noexcept {
    const DWORD old=GetLastError();
    try{
        if(!available){SetLastError(old);return;}
        if(thread!=GetCurrentThreadId()){foreign=true;SetLastError(old);return;}
        pending={};uint32_t r[9]{},args[2]{};
        if(!MainBackdrop::read(uint32_t(reinterpret_cast<uintptr_t>(saved)),r,sizeof(r))||
           !MainBackdrop::read(r[3]+4,args,sizeof(args))){MainBackdrop::coverageGapPending=true;SetLastError(old);return;}
        const auto layer=MainBackdrop::current.lock();const auto i=index(r[1]);
        if(!layer||!layer->good||layer->composed||i<0||MainBackdrop::value<uint32_t>(0x4D0A24)!=7||
           args[0]!=MainBackdrop::value<uint32_t>(0x7138EC)){SetLastError(old);return;}
        const auto& b=table[i];
        if(MainBackdrop::value<uint32_t>(b.node)!=b.id||MainBackdrop::value<uint32_t>(b.node+0x77)!=0x4E47C0||
           MainBackdrop::value<uint32_t>(b.node+0x4C)!=0x46B440||MainBackdrop::value<uint32_t>(b.node+0x50)!=(i==2?0x000B1909u:0x000C1909u)){
            layer->fail("button_lookup_owner_changed");SetLastError(old);return;}
        witness(layer->scope,r[1],args[1],r[7]);
    }catch(...){MainBackdrop::coverageGapPending=true;}
    SetLastError(old);
}
MENU_BRIDGE(lookupHook,0,0,original,observe)
std::shared_ptr<const ButtonBank> load(const std::filesystem::path& path,const Button& b){
    std::ifstream f(path,std::ios::binary|std::ios::ate);if(!f||f.tellg()!=40320160)throw std::runtime_error("button bank missing/size");
    std::vector<uint8_t> bytes(40320160);f.seekg(0);if(!f.read(reinterpret_cast<char*>(bytes.data()),bytes.size()))throw std::runtime_error("button bank read");
    unsigned char digest[32]{};if(BCryptHash(BCRYPT_SHA256_ALG_HANDLE,nullptr,0,bytes.data(),ULONG(bytes.size()),digest,32)<0)throw std::runtime_error("button bank hash");
    std::string actual;const char* hex="0123456789abcdef";for(auto c:digest){actual+=hex[c>>4];actual+=hex[c&15];}
    if(actual!=b.digest)throw std::runtime_error("button bank identity");auto bank=ButtonBank::parse(std::move(bytes),b.idle);
    if(!bank)throw std::runtime_error("button bank resource layout");return std::make_shared<const ButtonBank>(std::move(*bank));
}
}
void install(){
    if(!feature(Feature::mainButtons,L"QFR_MAIN_BUTTONS"))return;
    try{wchar_t file[32768]{};if(!GetModuleFileNameW(nullptr,file,32768))throw std::runtime_error("button module path");
        const auto folder=std::filesystem::path(file).parent_path()/L"rs/Remaster/MainButtons";
        std::array<std::shared_ptr<const ButtonBank>,6> next;
        for(unsigned i=0;i<6;++i)next[i]=load(folder/table[i].file,table[i]);
        // Two whole original instructions, independently verified in both EXEs:
        // MOVSX ECX,word[ESI+0A]; MOV EDX,[ESP+20]. Observe before replay.
        constexpr unsigned char expected[]={0x0F,0xBF,0x4E,0x0A,0x8B,0x54,0x24,0x20};unsigned char actual[8]{};
        if(!MainBackdrop::read(0x46B540,actual,sizeof(actual))||memcmp(actual,expected,sizeof(actual)))throw std::runtime_error("button postlookup instruction mismatch");
        const auto before=original[0];Compat::hookFunction(original[0],reinterpret_cast<void*>(&lookupHook),"QUANTUM::Main actual button resource");
        if(original[0]==before)throw std::runtime_error("button lookup hook unavailable");
        banks=std::move(next);thread=GetCurrentThreadId();foreign=false;pending={};available=true;
        note("main_buttons_ready","v7;6banks/210RGBAframes;720x400 source/180x100 layout; original lookup/timers/callbacks");
    }catch(const std::exception& e){available=false;banks={};note("main_buttons_original_retained",e.what());}
}
}
