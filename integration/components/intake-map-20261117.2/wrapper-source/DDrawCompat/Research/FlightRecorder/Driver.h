#pragma once
#include "Recorder.h"
#include <Windows.h>
#include <d3d.h>
#include <d3dumddi.h>
namespace D3dDdi {class Resource;class Device;}
namespace Qfr {
D3DDDI_DEVICEFUNCS wrapDriver(HANDLE device,const D3DDDI_DEVICEFUNCS& raw);
void logical(D3dDdi::Resource&,const char* where,HANDLE shadow,bool cpu,bool gpu,UINT sub=0,const std::string& context={}) noexcept;
void snapshot(D3dDdi::Resource&,UINT sub,const void* data,UINT pitch) noexcept;
void cpuCopy(D3dDdi::Resource& dest,D3dDdi::Resource& src,const D3DDDIARG_BLT&,HRESULT) noexcept;
void prepared(D3dDdi::Resource&,D3dDdi::Resource&,UINT) noexcept;
void palette(D3dDdi::Device&,UINT,const void*) noexcept;
void boundary(const char* kind) noexcept;
void initializeProcess();
}
