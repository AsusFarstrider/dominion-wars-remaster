#include <Remaster/Runtime.h>
#include <Research/MenuProbe/Probe.h>
#include <Research/NativeProbe/Probe.h>
#include "Recorder.h"
#include "Queue.h"
#include "Delta.h"
#include "Durability.h"
#include <Windows.h>
#include <bcrypt.h>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <map>
#include <mutex>
#include <sstream>
#include <thread>

namespace Qfr {
namespace {
thread_local bool busy=false;thread_local Id parentCall=0;thread_local Replica replica;
thread_local const char* currentName=nullptr;thread_local const char* currentLayer=nullptr;
std::atomic<Id> calls{0};std::atomic<uint64_t> lost{0};
std::atomic<unsigned> armed{0};
std::atomic<MetricFn> metrics{&GetSystemMetrics};
struct Item {uint64_t seq=0;int64_t qpc=0;DWORD thread=0;Id call=0;std::string kind,fields;std::vector<unsigned char> bytes;bool blob=false;};
std::wstring env(const wchar_t* key){DWORD n=GetEnvironmentVariableW(key,nullptr,0);if(!n)return {};std::wstring s(n,L'\0');GetEnvironmentVariableW(key,s.data(),n);s.resize(n-1);return s;}
std::string utf8(const std::wstring& s){if(s.empty())return {};const int n=WideCharToMultiByte(CP_UTF8,0,s.data(),int(s.size()),nullptr,0,nullptr,nullptr);std::string r(n,0);WideCharToMultiByte(CP_UTF8,0,s.data(),int(s.size()),r.data(),n,nullptr,nullptr);return r;}
int64_t tick(){LARGE_INTEGER n{};QueryPerformanceCounter(&n);return n.QuadPart;}
std::string hash(const void* data,size_t size){unsigned char digest[32]{};if(size>ULONG_MAX || BCryptHash(BCRYPT_SHA256_ALG_HANDLE,nullptr,0,(PUCHAR)data,ULONG(size),digest,32)<0)throw std::runtime_error("sha256");const char* hex="0123456789abcdef";std::string r;for(auto b:digest){r+=hex[b>>4];r+=hex[b&15];}return r;}
struct Guard {bool old=busy;Guard(){busy=true;}~Guard(){busy=old;}};
struct Recorder {
    Queue<Item> queue{64*1024*1024};std::mutex order,registry,pixelLock;FrameCache frames{64*1024*1024};
    std::thread writer,controller;std::atomic<bool> accepting{false},failed{false},ended{false};
    std::filesystem::path root;uint64_t sequence=0,nextResource=0;std::map<std::pair<uintptr_t,uintptr_t>,Id> live;
    HANDLE output=INVALID_HANDLE_VALUE;std::string finishReason="unclosed",pending;uint64_t writeCount=0,failAfter=0,writeCalls=0;
    HANDLE durable=INVALID_HANDLE_VALUE;
    bool checkpoint(uint64_t seq) {
        if(!flush()||!FlushFileBuffers(output))return false;
        const auto line="{\"through_sequence\":"+std::to_string(seq)+",\"lost_observed\":"+std::to_string(lost.load())+
            ",\"exceptions_observed\":"+std::to_string(Durability::observed)+",\"exception_write_failures\":"+std::to_string(Durability::writeFailures)+"}\n";
        DWORD count=0;
        return durable!=INVALID_HANDLE_VALUE&&WriteFile(durable,line.data(),DWORD(line.size()),&count,nullptr)&&count==line.size()&&FlushFileBuffers(durable);
    }
    bool flush(){
        if(pending.empty())return true;
        size_t at=0;while(at<pending.size()){DWORD n=0;++writeCalls;
            if(!WriteFile(output,pending.data()+at,DWORD(pending.size()-at),&n,nullptr)||!n)return false;at+=n;}
        pending.clear();return true;
    }
    bool write(const std::string& s){if(failAfter && ++writeCount>failAfter)return false;pending+=s;return pending.size()<256*1024 || flush();}
    Recorder(){
        const auto directory=env(L"QFR_DIR");if(directory.empty())return;
        try {
            root=directory;if(!root.is_absolute())return;
            std::filesystem::create_directories(root/"blobs");
            output=CreateFileW((root/L"events.ndjson").c_str(),GENERIC_WRITE,FILE_SHARE_READ,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
            if(output==INVALID_HANDLE_VALUE)return;
            Durability::enable(root);
            if(Durability::active) {
                durable=CreateFileW((root/L"durable.ndjson").c_str(),GENERIC_WRITE,FILE_SHARE_READ,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
                if(durable==INVALID_HANDLE_VALUE)throw std::runtime_error("durability_journal_open");
            }
            const auto fault=env(L"QFR_TEST_FAIL_AFTER_EVENTS");if(!fault.empty())failAfter=std::stoull(fault);
            accepting=true;writer=std::thread([this]{run();});
            // Separate controller permits graceful stop even when the game is idle.
            controller=std::thread([this]{
                try{
                    HWND priorWindow=nullptr;int priorWidth=0,priorHeight=0;
                    ULONGLONG nextCheckpoint=0;
                    while(accepting){
                        if(Durability::active&&GetTickCount64()>=nextCheckpoint) {
                            event("native_recording_checkpoint",",\"lost_observed\":"+std::to_string(lost.load()));
                            nextCheckpoint=GetTickCount64()+500;
                        }
                        if(std::filesystem::exists(root/"stop.request")){close("control_file");break;}
                        const auto window=GetForegroundWindow();const auto width=metrics.load()(SM_CXSCREEN),height=metrics.load()(SM_CYSCREEN);
                        if(window!=priorWindow || width!=priorWidth || height!=priorHeight){
                            DWORD pid=0;GetWindowThreadProcessId(window,&pid);
                            event("display_focus",",\"foreground\":"+std::to_string(reinterpret_cast<uintptr_t>(window))+",\"foreground_pid\":"+std::to_string(pid)+",\"game_foreground\":"+(pid==GetCurrentProcessId()?"true":"false")+",\"desktop\":["+std::to_string(width)+","+std::to_string(height)+"]");
                            priorWindow=window;priorWidth=width;priorHeight=height;
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                }catch(...){++lost;close("controller_failure");}
            });
            LARGE_INTEGER f{};QueryPerformanceFrequency(&f);wchar_t exe[32768]{};GetModuleFileNameW(nullptr,exe,32768);
            submit("session",",\"schema\":\"qfr-1\",\"pid\":"+std::to_string(GetCurrentProcessId())+",\"qpc_frequency\":"+std::to_string(f.QuadPart)+",\"exe\":"+quote(utf8(exe))+",\"pointer_bits\":32,\"queue_limit\":67108864,\"blob_limit\":33554432,\"desktop\":["+std::to_string(metrics.load()(SM_CXSCREEN))+","+std::to_string(metrics.load()(SM_CYSCREEN))+"]",{},false);
            if(Durability::active)submit("native_recording_durability",",\"checkpoint_interval_ms\":500,\"journal\":\"durable.ndjson\",\"exception_file\":\"exceptions.bin\",\"exception_record_bytes\":92,\"first_chance_observer_enabled\":"+std::string(Durability::handler?"true":"false")+",\"exception_limit\":64,\"limitations\":[\"queue tail after last durable checkpoint can be lost on hard termination\",\"first chance exceptions may be handled by the game\",\"fast-fail or external termination can bypass exception observation\",\"initialization before first DirectDraw export unobserved\"]",{},false);
            if(Durability::active)submit("native_recording_policy",",\"queue_policy\":\"nonblocking; 60MiB ordinary plus4MiB critical reserve\",\"overflow_policy\":\"drop excess records and report lost count; never wait on graphics thread\",\"durability\":\"periodic500ms and rare opening errors; no per-query disk flush\",\"opening_callback_sampling\":\"four per node/state key plus exact counters\"",{},false);
        }catch(...){failed=true;accepting=false;}
    }
    void submit(const char* kind,const std::string& fields,std::vector<unsigned char> data,bool blob){
        if(!accepting)return;
        std::lock_guard lock(order);if(!accepting)return;
        Item item;item.seq=++sequence;item.qpc=tick();item.thread=GetCurrentThreadId();item.call=parentCall;item.kind=kind;item.fields=fields;item.bytes=std::move(data);item.blob=blob;
        const auto size=item.fields.size()+item.bytes.size()+sizeof(Item)+item.kind.size();
        bool pushed=false;
        if(Durability::active) {
            const bool critical=!strcmp(kind,"native_opening_error")||!strcmp(kind,"native_api_failure")||
                !strcmp(kind,"native_recording_checkpoint")||!strcmp(kind,"coverage_gap")||strstr(kind,"_gap")||
                !strcmp(kind,"native_transition_install_entry")||!strcmp(kind,"native_transition_precleanup")||
                !strcmp(kind,"session")||!strcmp(kind,"build")||!strcmp(kind,"file")||!strcmp(kind,"process_exit");
            pushed=queue.tryPush(std::move(item),size,critical?64*1024*1024:60*1024*1024);
        }else pushed=queue.push(std::move(item),size);
        if(!pushed)++lost;
    }
    void run() noexcept {
        Guard guard;try {
            Item i;
            while(queue.pop(i)){
                std::string blob;
                if(i.blob){const auto sha=hash(i.bytes.data(),i.bytes.size());const auto path=root/"blobs"/(sha+".bin");
                    if(!std::filesystem::exists(path)){std::ofstream f(path,std::ios::binary);f.write((const char*)i.bytes.data(),i.bytes.size());if(!f)throw std::runtime_error("blob_write");}
                    blob=",\"sha256\":"+quote(sha)+",\"bytes\":"+std::to_string(i.bytes.size());}
                const auto line="{\"seq\":"+std::to_string(i.seq)+",\"qpc\":"+std::to_string(i.qpc)+",\"thread\":"+std::to_string(i.thread)+",\"call\":"+std::to_string(i.call)+",\"event\":"+quote(i.kind)+i.fields+blob+"}\n";
                if(!write(line))throw std::runtime_error("event_write");
                if(Durability::active&&(i.kind=="native_recording_checkpoint"||i.kind=="native_opening_error"))
                    if(!checkpoint(i.seq))throw std::runtime_error("durability_checkpoint_write");
            }
            const auto footer="{\"event\":\"capture_end\",\"last_sequence\":"+std::to_string(sequence)+",\"complete\":"+(lost.load()?"false":"true")+",\"lost\":"+std::to_string(lost.load())+",\"queue_high\":"+std::to_string(queue.high)+",\"stalls\":"+std::to_string(queue.stalls)+",\"event_file_writes_before_footer\":"+std::to_string(writeCalls)+",\"reason\":"+quote(finishReason)+"}\n";
            if(!write(footer)||!flush()||!FlushFileBuffers(output))throw std::runtime_error("footer_write");
        }catch(...){failed=true;++lost;queue.fail();accepting=false;
            // Best effort sidecar; absence of a footer is independently incomplete.
            HANDLE f=CreateFileW((root/L"INCOMPLETE").c_str(),GENERIC_WRITE,FILE_SHARE_READ,nullptr,CREATE_ALWAYS,0,nullptr);if(f!=INVALID_HANDLE_VALUE){const char* why="writer failure; evidence incomplete\n";DWORD n;WriteFile(f,why,DWORD(strlen(why)),&n,nullptr);CloseHandle(f);}}
        if(output!=INVALID_HANDLE_VALUE){CloseHandle(output);output=INVALID_HANDLE_VALUE;}ended=true;
        if(durable!=INVALID_HANDLE_VALUE){CloseHandle(durable);durable=INVALID_HANDLE_VALUE;}
    }
    void close(const char* why){std::lock_guard lock(order);if(!accepting.exchange(false))return;finishReason=why;queue.close();}
};
Recorder& recorder(){static Recorder* r=new Recorder;return *r;}
}
bool requested() noexcept {if(!Remaster::recordingAllowed())return false;wchar_t b[2]{};return GetEnvironmentVariableW(L"QFR_DIR",b,2)>0;}
void setMetrics(MetricFn fn) noexcept {metrics=fn;}
void start() noexcept {if(!requested())return;static std::once_flag once;try{std::call_once(once,[]{armed=1;{Guard guard;recorder();}armed=2;});}catch(...){++lost;armed=0;}}
bool enabled() noexcept {if(busy || armed!=2 || !requested())return false;try{Guard guard;return recorder().accepting;}catch(...){++lost;return false;}}
std::string quote(const std::string& s){std::string r="\"";const char* hex="0123456789abcdef";for(unsigned char c:s){if(c=='"' || c=='\\'){r+='\\';r+=char(c);}else if(c<32 || c>=127){r+="\\u00";r+=hex[c>>4];r+=hex[c&15];}else r+=char(c);}return r+'"';}
void event(const char* kind,const std::string& fields) noexcept {
    if(Remaster::enabled()&&(!strcmp(kind,"native_opening_error")||!strcmp(kind,"native_main_background_reject")||!strcmp(kind,"native_canvas_coverage_lost")||!strcmp(kind,"native_hud_assembly_error")))Remaster::note(kind,fields);
    if(!enabled()||!NativeProbe::allowEvent(kind,fields))return;try{Guard guard;recorder().submit(kind,MenuProbe::annotate(kind,fields),{},false);}catch(...){++lost;}}
void gap(const char* category,const char* reason) noexcept {event("coverage_gap",",\"category\":"+quote(category)+",\"reason\":"+quote(reason));}
void stop(const char* reason) noexcept {if(!requested() || armed!=2)return;try{Guard guard;auto& r=recorder();r.close(reason);if(r.writer.joinable() && r.writer.get_id()!=std::this_thread::get_id())r.writer.join();if(r.controller.joinable() && r.controller.get_id()!=std::this_thread::get_id())r.controller.join();}catch(...){++lost;}}
Id resource(uintptr_t device,uintptr_t handle) noexcept {if(!handle || !enabled())return 0;try{Guard guard;auto& r=recorder();std::lock_guard lock(r.registry);const auto it=r.live.find({device,handle});return it==r.live.end()?0:it->second;}catch(...){++lost;return 0;}}
Id create(uintptr_t device,uintptr_t handle,const std::string& desc) noexcept {
    if(!enabled() || !handle)return 0;try{Id id;bool reuse;{Guard guard;auto& r=recorder();std::lock_guard lock(r.registry);auto& entry=r.live[{device,handle}];reuse=entry!=0;id=entry=++r.nextResource;}
        event("resource_create",",\"resource\":"+std::to_string(id)+",\"device\":"+std::to_string(device)+",\"handle\":"+std::to_string(handle)+",\"unclosed_reuse\":"+(reuse?"true":"false")+desc);return id;
    }catch(...){++lost;return 0;}}
void destroy(uintptr_t device,uintptr_t handle,int32_t hr) noexcept {const auto id=resource(device,handle);event("resource_destroy",",\"resource\":"+std::to_string(id)+",\"device\":"+std::to_string(device)+",\"handle\":"+std::to_string(handle)+",\"hr\":"+std::to_string(hr));if(hr>=0 && enabled()){try{Guard guard;auto& r=recorder();std::lock_guard lock(r.registry);r.live.erase({device,handle});}catch(...){++lost;}}}
void memory(const char* kind,const void* data,size_t bytes,const std::string& fields) noexcept {
    if(NativeProbe::active()||!enabled())return;
    if(!data || !bytes || bytes>32*1024*1024){gap(kind,"null_empty_or_over_32MiB_blob");return;}
    try{std::vector<unsigned char> copy(bytes);SIZE_T got=0;if(!ReadProcessMemory(GetCurrentProcess(),data,copy.data(),bytes,&got) || got!=bytes){gap(kind,"unreadable_memory");return;}Guard guard;recorder().submit(kind,fields,std::move(copy),true);}catch(...){++lost;gap(kind,"allocation_failure");}
}
void retireDevice(uintptr_t device) noexcept {
    if(!enabled())return;
    try{Guard guard;auto& r=recorder();std::lock_guard lock(r.registry);
        for(auto it=r.live.begin();it!=r.live.end();)if(it->first.first==device)it=r.live.erase(it);else++it;
    }catch(...){++lost;}
}
void rows(const char* kind,const void* data,uint32_t width,uint32_t height,int64_t pitch,uint32_t bpp,const std::string& fields) noexcept {
    if(NativeProbe::active()||!enabled())return;
    const uint64_t row=uint64_t(width)*bpp;
    if(!data || !row || !height || pitch<int64_t(row) || row*height>32*1024*1024 || uint64_t(pitch)*height>128*1024*1024){gap(kind,"unsupported_layout_or_blob_limit");return;}
    try{
        std::vector<unsigned char> copy(static_cast<size_t>(row*height));SIZE_T got=0;
        if(uint64_t(pitch)==row){if(!ReadProcessMemory(GetCurrentProcess(),data,copy.data(),copy.size(),&got)||got!=copy.size()){gap(kind,"unreadable_rows");return;}}
        else{std::vector<unsigned char> padded(static_cast<size_t>(uint64_t(pitch)*(height-1)+row));if(!ReadProcessMemory(GetCurrentProcess(),data,padded.data(),padded.size(),&got)||got!=padded.size()){gap(kind,"unreadable_padded_rows");return;}for(unsigned y=0;y<height;++y)memcpy(copy.data()+size_t(row)*y,padded.data()+size_t(pitch)*y,size_t(row));}
        Guard guard;auto& r=recorder();std::lock_guard pixel(r.pixelLock);
        const auto layout=",\"width\":"+std::to_string(width)+",\"height\":"+std::to_string(height)+",\"pitch\":"+std::to_string(pitch)+",\"bytes_per_pixel\":"+std::to_string(bpp);
        auto delta=r.frames.encode(std::string(kind)+fields+layout,std::move(copy));
        const auto description=fields+layout+",\"pixel_stream\":"+std::to_string(delta.stream)+",\"snapshot\":"+std::to_string(delta.snapshot)+",\"base_snapshot\":"+std::to_string(delta.base)+",\"packed_bytes\":"+std::to_string(delta.packed)+",\"encoding\":"+quote(delta.encoding);
        const bool hasBytes=!delta.data.empty();r.submit(kind,description,std::move(delta.data),hasBytes);
    }catch(...){++lost;gap(kind,"allocation_failure");}
}
void fileHash(const wchar_t* path,const char* role) noexcept {if(!enabled())return;try{std::ifstream f(std::filesystem::path(path),std::ios::binary);std::vector<char> b((std::istreambuf_iterator<char>(f)),{});if(!f.eof() && !f){gap("file_hash","read_failed");return;}const auto sha=hash(b.data(),b.size());event("file",",\"role\":"+quote(role)+",\"path\":"+quote(utf8(path))+",\"sha256\":"+quote(sha)+",\"bytes\":"+std::to_string(b.size()));}catch(...){gap("file_hash","failed");}}
Call::Call(const char* layer,const char* name,uintptr_t object,const std::function<std::string()>& parameters) noexcept {
    if(!NativeProbe::allowCall(layer,name)||!enabled())return;
    if(currentName && currentLayer && !strcmp(layer,"wrapper") && (!strcmp(currentLayer,"api") || !strcmp(currentLayer,"driver")) && !strcmp(name,currentName))return;
    try{std::string p;{Guard guard;p=parameters?parameters():"";}parent=parentCall;id=++calls;event("call_begin",",\"id\":"+std::to_string(id)+",\"parent\":"+std::to_string(parent)+",\"layer\":"+quote(layer)+",\"name\":"+quote(name)+",\"object\":"+std::to_string(object)+",\"parameters\":"+quote(p));parentCall=id;previousName=currentName;previousLayer=currentLayer;currentName=name;currentLayer=layer;}catch(...){++lost;}
}
Call::~Call(){if(id){event("call_end",",\"id\":"+std::to_string(id));parentCall=parent;currentName=previousName;currentLayer=previousLayer;}}
void Call::result(int64_t value,const std::function<std::string()>& output) noexcept {if(!id)return;try{std::string p;{Guard guard;if(output)p=output();}event("call_result",",\"id\":"+std::to_string(id)+",\"result\":"+std::to_string(value)+",\"output\":"+quote(p));}catch(...){++lost;}}
void Call::resultText(const std::function<std::string()>& output) noexcept {if(!id)return;try{std::string p;{Guard guard;p=output();}event("call_output",",\"id\":"+std::to_string(id)+",\"output\":"+quote(p));}catch(...){++lost;}}
Replica currentReplica() noexcept{return replica;}
ReplicaScope::ReplicaScope(Replica r) noexcept:previous(replica){replica=r;}
ReplicaScope::~ReplicaScope(){replica=previous;}
}
