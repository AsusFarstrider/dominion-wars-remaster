#include <Research/NativeProbe/Probe.h>
#include "Driver.h"
#include <Common/Hook.h>
#include <Windows.h>
#include <mutex>

namespace Qfr {
void engineHooks();
namespace {
int WINAPI physicalMetric(int index){return CALL_ORIG_FUNC(GetSystemMetrics)(index);}
void WINAPI exitProcess(UINT code){NativeProbe::finish();event("process_exit",",\"code\":"+std::to_string(code));stop("ExitProcess");CALL_ORIG_FUNC(ExitProcess)(code);}
BOOL CALLBACK monitor(HMONITOR handle,HDC,LPRECT,LPARAM){MONITORINFOEXW m{};m.cbSize=sizeof(m);if(GetMonitorInfoW(handle,&m))event("monitor",",\"handle\":"+std::to_string(reinterpret_cast<uintptr_t>(handle))+",\"primary\":"+std::to_string(m.dwFlags)+",\"rect\":["+std::to_string(m.rcMonitor.left)+","+std::to_string(m.rcMonitor.top)+","+std::to_string(m.rcMonitor.right)+","+std::to_string(m.rcMonitor.bottom)+"]");return TRUE;}
}
void initializeProcess(){if(!requested())return;static std::once_flag once;std::call_once(once,[]{
    HMODULE self=nullptr;GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,reinterpret_cast<LPCWSTR>(&initializeProcess),&self);
    setMetrics(&physicalMetric);
    start();
    HOOK_FUNCTION(kernel32,ExitProcess,exitProcess);
    wchar_t exe[32768]{},dll[32768]{};GetModuleFileNameW(nullptr,exe,32768);GetModuleFileNameW(self,dll,32768);fileHash(exe,"executable");fileHash(dll,"recorder_wrapper");
    event("build",",\"baseline\":\"int-20261018.1\",\"source_revision\":\"921826392c5b6f5acdc523f4db2429cbad234782\",\"recorder\":\"qfr-1\"");
    using Version=LONG (WINAPI*)(OSVERSIONINFOW*);auto version=reinterpret_cast<Version>(GetProcAddress(GetModuleHandleW(L"ntdll.dll"),"RtlGetVersion"));
    OSVERSIONINFOW os{};os.dwOSVersionInfoSize=sizeof(os);if(version && version(&os)==0)event("os",",\"major\":"+std::to_string(os.dwMajorVersion)+",\"minor\":"+std::to_string(os.dwMinorVersion)+",\"build\":"+std::to_string(os.dwBuildNumber));
    using Dpi=UINT (WINAPI*)();auto dpi=reinterpret_cast<Dpi>(GetProcAddress(GetModuleHandleW(L"user32.dll"),"GetDpiForSystem"));if(dpi)event("dpi",",\"system_dpi\":"+std::to_string(dpi()));
    EnumDisplayMonitors(nullptr,nullptr,monitor,0);
    gap("capture_start","Recording starts at the first public DirectDraw export; earlier process initialization is unobserved");
    engineHooks();
    gap("gpu_pixels","QFR_READBACK=1 attempts native-D3D9 first-prepare staging snapshots; D3D9On12 readback is blocked after failed validation; continuous GPU contents and physical residency are unobserved");
    gap("payload_formats","CPU pixel packing supports positive-pitch uncompressed formats only; each rejected payload emits a coverage_gap");
});}
}
