#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace Qfr {
using Id=uint64_t;
bool requested() noexcept;
void start() noexcept; // Called outside DllMain; arms only after worker startup completes.
bool enabled() noexcept;
using MetricFn=int(__stdcall*)(int);
void setMetrics(MetricFn) noexcept;
std::string quote(const std::string&);
void event(const char* kind,const std::string& fields="") noexcept;
void gap(const char* category,const char* reason) noexcept;
void stop(const char* reason) noexcept; // Explicit control/ExitProcess only; never wait in DllMain.
Id resource(uintptr_t device,uintptr_t handle) noexcept;
Id create(uintptr_t device,uintptr_t handle,const std::string& descriptor) noexcept;
void destroy(uintptr_t device,uintptr_t handle,int32_t hr) noexcept;
void retireDevice(uintptr_t device) noexcept;
void memory(const char* kind,const void* data,size_t bytes,const std::string& fields) noexcept;
void rows(const char* kind,const void* data,uint32_t width,uint32_t height,int64_t pitch,uint32_t bytesPerPixel,const std::string& fields) noexcept;
void fileHash(const wchar_t* path,const char* role) noexcept;

class Call {
    Id id=0,parent=0;
    const char* previousName=nullptr;const char* previousLayer=nullptr;
public:
    Call(const char* layer,const char* name,uintptr_t object,const std::function<std::string()>& parameters) noexcept;
    ~Call();
    void result(int64_t value,const std::function<std::string()>& output={}) noexcept;
    void resultText(const std::function<std::string()>& output) noexcept;
    Call(const Call&)=delete;Call& operator=(const Call&)=delete;
};
// Owner witness applies only during this internal driver operation.
struct Replica {uintptr_t device=0,owner=0,shadow=0;bool cpuCurrent=false,gpuCurrent=false;};
Replica currentReplica() noexcept;
class ReplicaScope {
    Replica previous;
public:
    explicit ReplicaScope(Replica value) noexcept;
    ~ReplicaScope();
};
}
