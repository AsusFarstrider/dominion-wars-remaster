#include <Research/NativeProbe/Probe.h>
#include "Hooks.h"
#include "Driver.h"
#include <array>
#include <atomic>
#include <map>
#include <mutex>
#include <set>
#include <D3dDdi/Device.h>
#include <D3dDdi/Adapter.h>
#include <D3dDdi/Resource.h>
#include <D3dDdi/FormatInfo.h>
#include <D3dDdi/Log/DeviceFuncsLog.h>
#include <D3dDdi/Visitors/DeviceFuncsVisitor.h>

namespace Qfr {
namespace {
uintptr_t ptr(HANDLE h){return reinterpret_cast<uintptr_t>(h);}
std::string field(const char* key,uint64_t n){return ",\""+std::string(key)+"\":"+std::to_string(n);}
std::string signedField(const char* key,LONG n){return ",\""+std::string(key)+"\":"+std::to_string(n);}
std::string rect(const char* key,const RECT& r){return ",\""+std::string(key)+"\":["+std::to_string(r.left)+","+std::to_string(r.top)+","+std::to_string(r.right)+","+std::to_string(r.bottom)+"]";}
// Driver function pointers can outlive CRT static destruction. Keep dispatch state
// process-lived, like Recorder; OS reclaims it after all driver callbacks stop.
std::mutex& mutex=*new std::mutex;
struct Lock {const void* data=nullptr;UINT pitch=0,flags=0;RECT area{};};
struct DeviceInfo {D3DDDI_DEVICEFUNCS raw{};Id lifetime=0;std::array<HANDLE,8> textures{};HANDLE target=nullptr;D3DDDIARG_VIEWPORTINFO viewport{};const void* vertices=nullptr;UINT stride=0;bool readbackFailed=false;};
std::atomic<Id> deviceSerial{0};
auto& devices=*new std::map<uintptr_t,DeviceInfo>;
auto& descriptors=*new std::map<std::pair<uintptr_t,uintptr_t>,D3DDDIARG_CREATERESOURCE2>;
auto& surfaces=*new std::map<std::pair<uintptr_t,uintptr_t>,std::vector<D3DDDI_SURFACEINFO>>;
auto& locks=*new std::map<std::tuple<uintptr_t,uintptr_t,UINT>,std::vector<Lock>>;
auto& readbacks=*new std::set<Id>;
// Failed staging work may still reference its caller-owned system-memory bytes.
// Retain at most one failed buffer per device, disable further attempts, and
// release the bytes only after successful driver device destruction.
auto& retainedReadbacks=*new std::map<uintptr_t,std::vector<unsigned char>>;

template<auto Member> const char* functionName=nullptr;
template<class... P> void payload(const char*,HANDLE,P...){}
void payload(const char* name,HANDLE dev,const D3DDDIARG_CREATEPIXELSHADER* d,const UINT* data){memory("shader_code",data,d->CodeSize,field("device",ptr(dev))+",\"function\":"+quote(name));}
void payload(const char* name,HANDLE dev,const D3DDDIARG_CREATEVERTEXSHADERFUNC* d,const UINT* data){memory("shader_code",data,d->Size,field("device",ptr(dev))+",\"function\":"+quote(name));}
void payload(const char* name,HANDLE dev,const D3DDDIARG_CREATEVERTEXSHADERDECL* d,const D3DDDIVERTEXELEMENT* data){memory("vertex_declaration",data,size_t(d->NumVertexElements)*sizeof(*data),field("device",ptr(dev))+",\"function\":"+quote(name));}
template<class P> void payload(const char* name,HANDLE dev,const D3DDDIARG_SETPIXELSHADERCONST* d,const P* data){const size_t stride=strstr(name,"ConstB")?4:16;memory("shader_constants",data,size_t(d->Count)*stride,field("device",ptr(dev))+field("register",d->Register)+",\"function\":"+quote(name));}
template<class P> void payload(const char* name,HANDLE dev,const D3DDDIARG_SETVERTEXSHADERCONST* d,const P* data){const size_t stride=strstr(name,"ConstB")?4:16;memory("shader_constants",data,size_t(d->Count)*stride,field("device",ptr(dev))+field("register",d->Register)+",\"function\":"+quote(name));}
template<auto Member,class R,class... P> R APIENTRY invoke(HANDLE dev,P... p){
    auto raw=[&]{std::lock_guard lock(mutex);return devices.at(ptr(dev)).raw.*Member;}();
    Call call("driver",functionName<Member>,ptr(dev),[&]{return parameters(p...);});payload(functionName<Member>,dev,p...);
    if(strstr(functionName<Member>,"pfnDraw")||strstr(functionName<Member>,"LockAsync")||strstr(functionName<Member>,"UnlockAsync")||
       strstr(functionName<Member>,"GenerateMip")||strstr(functionName<Member>,"UpdateSubresource")||
       strstr(functionName<Member>,"ResourceCopy")||strstr(functionName<Member>,"Discard")){
        gap("unclassified_driver_write",functionName<Member>);boundary("unclassified_driver_write");
    }
    if constexpr(std::is_void_v<R>){raw(dev,p...);}else{const auto r=raw(dev,p...);call.result(number(r),[&]{return parameters(p...);});return r;}
}
template<auto Member,class R,class... P> auto select(R(APIENTRY*)(HANDLE,P...)){return &invoke<Member,R,P...>;}
struct Visitor {
    D3DDDI_DEVICEFUNCS& out;
    template<auto Member> void visit(const char* name){if(out.*Member){functionName<Member> = name;out.*Member=select<Member>(out.*Member);}}
};
// Selected entries below add structured facts. All remaining visited functions still have paired raw driver calls.
template<auto Member,class... P> auto rawCall(const char* name,HANDLE dev,P... p){
    auto function=[&]{std::lock_guard lock(mutex);return devices.at(ptr(dev)).raw.*Member;}();
    Call call("driver",name,ptr(dev),[&]{return parameters(p...);});const auto result=function(dev,p...);call.result(result,[&]{return parameters(p...);});return result;
}
void created(HANDLE dev,const D3DDDIARG_CREATERESOURCE2& data,HRESULT hr){
    if(FAILED(hr)){event("resource_create_failed",signedField("hr",hr)+field("device",ptr(dev)));return;}
    Id deviceLife;{std::lock_guard lock(mutex);deviceLife=devices.at(ptr(dev)).lifetime;}
    const auto id=create(ptr(dev),ptr(data.hResource),field("device_lifetime",deviceLife)+field("format",data.Format)+field("pool",data.Pool)+field("flags",data.Flags.Value)+field("mips",data.MipLevels)+field("surfaces",data.SurfCount));
    {std::lock_guard lock(mutex);const auto key=std::make_pair(ptr(dev),ptr(data.hResource));descriptors[key]=data;surfaces[key]=std::vector<D3DDDI_SURFACEINFO>(data.pSurfList,data.pSurfList+data.SurfCount);descriptors[key].pSurfList=nullptr;}
    for(UINT sub=0;sub<data.SurfCount;++sub){const auto& s=data.pSurfList[sub];const auto fields=field("resource",id)+field("sub",sub)+field("width",s.Width)+field("height",s.Height)+field("pitch",s.SysMemPitch)+field("format",data.Format);
        event("subresource",fields);
        if(s.pSysMem){const auto bpp=D3dDdi::getFormatInfo(data.Format).bytesPerPixel;if(data.Flags.VertexBuffer)memory("buffer",s.pSysMem,s.Width,fields+",\"stage\":\"creation\"");
            else rows("pixels",s.pSysMem,s.Width,s.Height,s.SysMemPitch,bpp,field("resource",id)+field("sub",sub)+field("format",data.Format)+",\"stage\":\"creation\"");}}
}
HRESULT APIENTRY create2(HANDLE dev,D3DDDIARG_CREATERESOURCE2* data){const auto hr=rawCall<&D3DDDI_DEVICEFUNCS::pfnCreateResource2>("pfnCreateResource2",dev,data);created(dev,*data,hr);return hr;}
HRESULT APIENTRY create1(HANDLE dev,D3DDDIARG_CREATERESOURCE* data){const auto hr=rawCall<&D3DDDI_DEVICEFUNCS::pfnCreateResource>("pfnCreateResource",dev,data);D3DDDIARG_CREATERESOURCE2 d{};memcpy(&d,data,sizeof(*data));created(dev,d,hr);return hr;}
HRESULT APIENTRY destroyResource(HANDLE dev,HANDLE handle){const auto hr=rawCall<&D3DDDI_DEVICEFUNCS::pfnDestroyResource>("pfnDestroyResource",dev,handle);destroy(ptr(dev),ptr(handle),hr);if(SUCCEEDED(hr)){std::lock_guard lock(mutex);descriptors.erase({ptr(dev),ptr(handle)});surfaces.erase({ptr(dev),ptr(handle)});for(auto i=locks.begin();i!=locks.end();)if(std::get<0>(i->first)==ptr(dev)&&std::get<1>(i->first)==ptr(handle))i=locks.erase(i);else++i;}return hr;}
HRESULT APIENTRY destroyDevice(HANDLE dev){Id life;{std::lock_guard lock(mutex);life=devices.at(ptr(dev)).lifetime;}const auto hr=rawCall<&D3DDDI_DEVICEFUNCS::pfnDestroyDevice>("pfnDestroyDevice",dev);event("device_destroy",field("device",ptr(dev))+field("device_lifetime",life)+signedField("hr",hr));if(SUCCEEDED(hr)){retireDevice(ptr(dev));std::lock_guard lock(mutex);retainedReadbacks.erase(ptr(dev));}return hr;}
HRESULT APIENTRY openResource(HANDLE dev,D3DDDIARG_OPENRESOURCE* d){const auto hr=rawCall<&D3DDDI_DEVICEFUNCS::pfnOpenResource>("pfnOpenResource",dev,d);if(SUCCEEDED(hr)){create(ptr(dev),ptr(d->hResource),",\"descriptor_known\":false,\"role\":\"opened_shared_resource\"");gap("shared_resource","allocation dimensions and content not available from open descriptor");}return hr;}
void copied(HANDLE dev,const D3DDDIARG_BLT& data,HRESULT hr,const char* origin){const auto w=currentReplica();
    event("copy",field("device",ptr(dev))+field("src",resource(ptr(dev),ptr(data.hSrcResource)))+field("dst",resource(ptr(dev),ptr(data.hDstResource)))+field("src_handle",ptr(data.hSrcResource))+field("dst_handle",ptr(data.hDstResource))+field("src_sub",data.SrcSubResourceIndex)+field("dst_sub",data.DstSubResourceIndex)+field("flags",data.Flags.Value)+signedField("hr",hr)+rect("src_rect",data.SrcRect)+rect("dst_rect",data.DstRect)+",\"origin\":"+quote(origin)+field("owner",resource(w.device,w.owner))+field("shadow",resource(w.device,w.shadow))+",\"cpu_current\":"+(w.cpuCurrent?"true":"false")+",\"gpu_current\":"+(w.gpuCurrent?"true":"false"));}
HRESULT APIENTRY blt(HANDLE dev,const D3DDDIARG_BLT* data){const auto hr=rawCall<&D3DDDI_DEVICEFUNCS::pfnBlt>("pfnBlt",dev,data);copied(dev,*data,hr,"driver");return hr;}
HRESULT APIENTRY texBlt(HANDLE dev,const D3DDDIARG_TEXBLT* d){const auto hr=rawCall<&D3DDDI_DEVICEFUNCS::pfnTexBlt>("pfnTexBlt",dev,d);D3DDDIARG_BLT b{};b.hSrcResource=d->hSrcResource;b.hDstResource=d->hDstResource;b.SrcRect=d->SrcRect;b.DstRect={d->DstPoint.x,d->DstPoint.y,d->DstPoint.x+d->SrcRect.right-d->SrcRect.left,d->DstPoint.y+d->SrcRect.bottom-d->SrcRect.top};if(d->CubeMapFace)b.Flags.Value=~0u;copied(dev,b,hr,"tex_blt");return hr;}
HRESULT APIENTRY texBlt1(HANDLE dev,const D3DDDIARG_TEXBLT1* d){const auto hr=rawCall<&D3DDDI_DEVICEFUNCS::pfnTexBlt1>("pfnTexBlt1",dev,d);D3DDDIARG_BLT b{};b.hSrcResource=d->hSrcResource;b.hDstResource=d->hDstResource;b.SrcRect=d->SrcRect;b.DstRect={d->DstPoint.x,d->DstPoint.y,d->DstPoint.x+d->SrcRect.right-d->SrcRect.left,d->DstPoint.y+d->SrcRect.bottom-d->SrcRect.top};if(d->CubeMapFace || d->CopyFlags)b.Flags.Value=~0u;copied(dev,b,hr,"tex_blt1");return hr;}
HRESULT APIENTRY lockResource(HANDLE dev,D3DDDIARG_LOCK* d){const auto hr=rawCall<&D3DDDI_DEVICEFUNCS::pfnLock>("pfnLock",dev,d);event("lock",field("resource",resource(ptr(dev),ptr(d->hResource)))+field("sub",d->SubResourceIndex)+field("flags",d->Flags.Value)+signedField("hr",hr));if(SUCCEEDED(hr)){std::lock_guard lock(mutex);locks[{ptr(dev),ptr(d->hResource),d->SubResourceIndex}].push_back({d->pSurfData,d->Pitch,d->Flags.Value,d->Flags.AreaValid?d->Area:RECT{}});}return hr;}
HRESULT APIENTRY unlockResource(HANDLE dev,const D3DDDIARG_UNLOCK* d){
    Lock saved{};D3DDDI_SURFACEINFO surface{};D3DDDIFORMAT format=D3DDDIFMT_UNKNOWN;bool found=false;
    {std::lock_guard lock(mutex);const auto key=std::make_tuple(ptr(dev),ptr(d->hResource),d->SubResourceIndex);auto it=locks.find(key);if(it!=locks.end()&&!it->second.empty()){saved=it->second.back();it->second.pop_back();const auto k=std::make_pair(ptr(dev),ptr(d->hResource));const auto s=surfaces.find(k);if(s!=surfaces.end()&&d->SubResourceIndex<s->second.size()){surface=s->second[d->SubResourceIndex];format=descriptors[k].Format;found=true;}}}
    if(found && saved.data){const UINT w=saved.area.right>saved.area.left?saved.area.right-saved.area.left:surface.Width;const UINT h=saved.area.bottom>saved.area.top?saved.area.bottom-saved.area.top:surface.Height;
        const auto fields=field("resource",resource(ptr(dev),ptr(d->hResource)))+field("sub",d->SubResourceIndex)+field("format",format)+field("lock_flags",saved.flags)+rect("area",saved.area)+",\"stage\":\"before_driver_unlock\"";
        if(format==D3DDDIFMT_VERTEXDATA)memory("buffer",saved.data,surface.Width,fields);else rows("pixels",saved.data,w,h,saved.pitch,D3dDdi::getFormatInfo(format).bytesPerPixel,fields);}
    const auto hr=rawCall<&D3DDDI_DEVICEFUNCS::pfnUnlock>("pfnUnlock",dev,d);event("unlock",field("resource",resource(ptr(dev),ptr(d->hResource)))+field("sub",d->SubResourceIndex)+signedField("hr",hr));return hr;
}
HRESULT APIENTRY setTexture(HANDLE dev,UINT stage,HANDLE texture){const auto hr=rawCall<&D3DDDI_DEVICEFUNCS::pfnSetTexture>("pfnSetTexture",dev,stage,texture);if(SUCCEEDED(hr)){std::lock_guard lock(mutex);if(stage<8)devices[ptr(dev)].textures[stage]=texture;}event("bind",field("device",ptr(dev))+field("stage",stage)+field("resource",resource(ptr(dev),ptr(texture)))+signedField("hr",hr));return hr;}
HRESULT APIENTRY setTarget(HANDLE dev,const D3DDDIARG_SETRENDERTARGET* d){const auto hr=rawCall<&D3DDDI_DEVICEFUNCS::pfnSetRenderTarget>("pfnSetRenderTarget",dev,d);if(SUCCEEDED(hr)&&d->RenderTargetIndex==0){std::lock_guard lock(mutex);devices[ptr(dev)].target=d->hRenderTarget;}return hr;}
HRESULT APIENTRY setViewport(HANDLE dev,const D3DDDIARG_VIEWPORTINFO* d){const auto hr=rawCall<&D3DDDI_DEVICEFUNCS::pfnSetViewport>("pfnSetViewport",dev,d);if(SUCCEEDED(hr)){std::lock_guard lock(mutex);devices[ptr(dev)].viewport=*d;}return hr;}
void drawEvent(HANDLE dev,HRESULT hr,UINT primitives,UINT type,const char* origin){DeviceInfo d;{std::lock_guard lock(mutex);d=devices.at(ptr(dev));}std::string textures="[";for(unsigned s=0;s<8;++s){if(s)textures+=",";textures+=std::to_string(resource(ptr(dev),ptr(d.textures[s])));}textures+="]";
    event("draw",field("device",ptr(dev))+signedField("hr",hr)+field("primitives",primitives)+field("type",type)+field("target",resource(ptr(dev),ptr(d.target)))+",\"textures\":"+textures+",\"origin\":"+quote(origin)+",\"viewport\":["+std::to_string(d.viewport.X)+","+std::to_string(d.viewport.Y)+","+std::to_string(d.viewport.Width)+","+std::to_string(d.viewport.Height)+"]");}
UINT count(UINT type,UINT primitives){switch(type){case D3DPT_POINTLIST:return primitives;case D3DPT_LINELIST:return primitives*2;case D3DPT_LINESTRIP:return primitives+1;case D3DPT_TRIANGLELIST:return primitives*3;case D3DPT_TRIANGLESTRIP:case D3DPT_TRIANGLEFAN:return primitives+2;default:return 0;}}
void vertices(HANDLE dev,UINT first,UINT n){DeviceInfo d;{std::lock_guard lock(mutex);d=devices.at(ptr(dev));}if(d.vertices && d.stride && n)memory("vertices",(const char*)d.vertices+size_t(first)*d.stride,size_t(n)*d.stride,field("device",ptr(dev))+field("stride",d.stride)+field("first",first));}
HRESULT APIENTRY streamGpu(HANDLE dev,const D3DDDIARG_SETSTREAMSOURCE* d){const auto hr=rawCall<&D3DDDI_DEVICEFUNCS::pfnSetStreamSource>("pfnSetStreamSource",dev,d);if(SUCCEEDED(hr)&&d->Stream==0){std::lock_guard lock(mutex);devices[ptr(dev)].vertices=nullptr;devices[ptr(dev)].stride=d->Stride;}return hr;}
HRESULT APIENTRY streamUm(HANDLE dev,const D3DDDIARG_SETSTREAMSOURCEUM* d,const void* data){const auto hr=rawCall<&D3DDDI_DEVICEFUNCS::pfnSetStreamSourceUm>("pfnSetStreamSourceUm",dev,d,data);if(SUCCEEDED(hr)&&d->Stream==0){std::lock_guard lock(mutex);devices[ptr(dev)].vertices=data;devices[ptr(dev)].stride=d->Stride;}return hr;}
HRESULT APIENTRY draw(HANDLE dev,const D3DDDIARG_DRAWPRIMITIVE* d,const UINT* flags){vertices(dev,d->VStart,count(d->PrimitiveType,d->PrimitiveCount));const auto hr=rawCall<&D3DDDI_DEVICEFUNCS::pfnDrawPrimitive>("pfnDrawPrimitive",dev,d,flags);drawEvent(dev,hr,d->PrimitiveCount,d->PrimitiveType,"draw_primitive");return hr;}
HRESULT APIENTRY drawIndexed(HANDLE dev,const D3DDDIARG_DRAWINDEXEDPRIMITIVE2* d,UINT stride,const void* indices,const UINT* flags){if(indices&&stride)memory("indices",indices,size_t(count(d->PrimitiveType,d->PrimitiveCount))*stride,field("device",ptr(dev))+field("stride",stride)+field("primitive_type",d->PrimitiveType));vertices(dev,0,d->NumVertices);const auto hr=rawCall<&D3DDDI_DEVICEFUNCS::pfnDrawIndexedPrimitive2>("pfnDrawIndexedPrimitive2",dev,d,stride,indices,flags);drawEvent(dev,hr,d->PrimitiveCount,d->PrimitiveType,"draw_indexed_primitive2");return hr;}
HRESULT APIENTRY colorFill(HANDLE dev,const D3DDDIARG_COLORFILL* d){const auto hr=rawCall<&D3DDDI_DEVICEFUNCS::pfnColorFill>("pfnColorFill",dev,d);event("write",field("resource",resource(ptr(dev),ptr(d->hResource)))+signedField("hr",hr)+",\"reason\":\"color_fill\"");return hr;}
HRESULT APIENTRY clear(HANDLE dev,const D3DDDIARG_CLEAR* d,UINT count,const RECT* rects){
    if(count&&rects)memory("clear_rects",rects,size_t(count)*sizeof(RECT),field("device",ptr(dev)));
    const auto hr=rawCall<&D3DDDI_DEVICEFUNCS::pfnClear>("pfnClear",dev,d,count,rects);
    if(d->Flags&D3DCLEAR_TARGET){HANDLE target;{std::lock_guard lock(mutex);target=devices.at(ptr(dev)).target;}
        event("write",field("resource",resource(ptr(dev),ptr(target)))+signedField("hr",hr)+field("flags",d->Flags)+field("rectangle_count",count)+",\"reason\":\"render_target_clear\"");}
    return hr;
}
void readback(HANDLE dev,HANDLE handle,bool on12){
    wchar_t option[2]{};if(GetEnvironmentVariableW(L"QFR_READBACK",option,2)!=1 || option[0]!=L'1')return;
    const auto sourceId=resource(ptr(dev),ptr(handle));D3DDDIARG_CREATERESOURCE2 original{};D3DDDI_SURFACEINFO source{};
    {std::lock_guard lock(mutex);if(devices.at(ptr(dev)).readbackFailed || !sourceId || !readbacks.insert(sourceId).second)return;const auto key=std::make_pair(ptr(dev),ptr(handle));if(!descriptors.count(key)||surfaces[key].empty()){gap("gpu_readback","no_descriptor");return;}original=descriptors[key];source=surfaces[key][0];}
    if(on12){event("readback_unavailable",field("source",sourceId)+",\"reason\":\"D3D9On12_readback_failed_hardware_validation_no_driver_calls_issued\"");return;}
    Call capture("readback","staging_snapshot",ptr(handle),[]{return "intrusive first-prepare snapshot per resource lifetime";});
    const auto bpp=D3dDdi::getFormatInfo(original.Format).bytesPerPixel;
    const uint64_t pitch=(uint64_t(source.Width)*bpp+3)&~uint64_t(3),size=pitch*source.Height;
    if(!bpp || !size || size>32*1024*1024 || original.SurfCount!=1 || original.MipLevels!=1 || original.Flags.CubeMap){gap("gpu_readback","compressed_multisurface_or_over_32MiB");return;}
    std::vector<unsigned char> allocation(static_cast<size_t>(size)+31);auto* buffer=reinterpret_cast<unsigned char*>((reinterpret_cast<uintptr_t>(allocation.data())+31)&~uintptr_t(31));
    D3DDDI_SURFACEINFO surface{};surface.Width=source.Width;surface.Height=source.Height;surface.pSysMem=buffer;surface.SysMemPitch=UINT(pitch);
    D3DDDIARG_CREATERESOURCE2 staging{};staging.Format=original.Format;staging.Pool=D3DDDIPOOL_SYSTEMMEM;staging.SurfCount=1;staging.MipLevels=1;staging.pSurfList=&surface;staging.Flags.Texture=1;staging.Rotation=D3DDDI_ROTATION_IDENTITY;
    bool create2Available;{std::lock_guard lock(mutex);create2Available=devices[ptr(dev)].raw.pfnCreateResource2!=nullptr;}
    HRESULT hr=create2Available?rawCall<&D3DDDI_DEVICEFUNCS::pfnCreateResource2>("readback_create2",dev,&staging):rawCall<&D3DDDI_DEVICEFUNCS::pfnCreateResource>("readback_create",dev,reinterpret_cast<D3DDDIARG_CREATERESOURCE*>(&staging));
    if(FAILED(hr)){event("readback_failed",field("source",sourceId)+signedField("hr",hr)+",\"stage\":\"create\"");return;}
    std::unique_ptr<void,std::function<void(void*)>> cleanup(staging.hResource,[dev](void* h){try{destroyResource(dev,h);}catch(...){gap("gpu_readback","cleanup_exception");}});
    created(dev,staging,hr);
    D3DDDIARG_BLT blt{};blt.hSrcResource=handle;blt.hDstResource=staging.hResource;blt.SrcRect={0,0,LONG(source.Width),LONG(source.Height)};blt.DstRect=blt.SrcRect;blt.Flags.Point=1;
    hr=rawCall<&D3DDDI_DEVICEFUNCS::pfnBlt>("readback_blt",dev,&blt);copied(dev,blt,hr,"readback");
    if(SUCCEEDED(hr)){
        D3DDDIARG_CREATEQUERY query{};query.QueryType=D3DDDIQUERYTYPE_EVENT;
        hr=rawCall<&D3DDDI_DEVICEFUNCS::pfnCreateQuery>("readback_query_create",dev,&query);
        if(SUCCEEDED(hr)){
            D3DDDIARG_ISSUEQUERY issue{};issue.hQuery=query.hQuery;issue.Flags.End=1;
            hr=rawCall<&D3DDDI_DEVICEFUNCS::pfnIssueQuery>("readback_query_issue",dev,&issue);
            bool flush1;{std::lock_guard lock(mutex);flush1=devices[ptr(dev)].raw.pfnFlush1!=nullptr;}
            if(SUCCEEDED(hr))hr=flush1?rawCall<&D3DDDI_DEVICEFUNCS::pfnFlush1>("readback_flush1",dev,0u):rawCall<&D3DDDI_DEVICEFUNCS::pfnFlush>("readback_flush",dev);
            BOOL ready=FALSE;D3DDDIARG_GETQUERYDATA get{};get.hQuery=query.hQuery;get.pData=&ready;const auto deadline=GetTickCount64()+5000;
            if(SUCCEEDED(hr))do{hr=rawCall<&D3DDDI_DEVICEFUNCS::pfnGetQueryData>("readback_query_poll",dev,&get);if(hr==S_FALSE)SwitchToThread();}while(hr==S_FALSE&&GetTickCount64()<deadline);
            if(hr==S_FALSE || !ready)hr=E_FAIL;
            rawCall<&D3DDDI_DEVICEFUNCS::pfnDestroyQuery>("readback_query_destroy",dev,query.hQuery);
            event("readback_sync",field("source",sourceId)+signedField("hr",hr));
        }
    }
    if(SUCCEEDED(hr)){
        D3DDDIARG_LOCK lock{};lock.hResource=staging.hResource;lock.Flags.ReadOnly=1;lock.Flags.NotifyOnly=1;
        hr=rawCall<&D3DDDI_DEVICEFUNCS::pfnLock>("readback_lock",dev,&lock);
        if(SUCCEEDED(hr)){
            rows("gpu_snapshot",buffer,source.Width,source.Height,LONG(pitch),bpp,field("source",sourceId)+field("staging",resource(ptr(dev),ptr(staging.hResource)))+field("format",original.Format)+",\"intrusive\":true,\"scope\":\"first_prepare_per_lifetime\",\"synchronization\":\"event_fence_and_preallocated_sysmem_notify_lock\"");
            D3DDDIARG_UNLOCK unlock{};unlock.hResource=staging.hResource;unlock.Flags.NotifyOnly=1;hr=rawCall<&D3DDDI_DEVICEFUNCS::pfnUnlock>("readback_unlock",dev,&unlock);
        }
    }
    if(FAILED(hr)){
        cleanup.release();
        {std::lock_guard lock(mutex);devices.at(ptr(dev)).readbackFailed=true;retainedReadbacks[ptr(dev)]=std::move(allocation);}
        event("readback_failed",field("source",sourceId)+signedField("hr",hr)+",\"stage\":\"copy_sync_or_lock\",\"device_readback_disabled\":true,\"staging_retained_until_device_destroy\":true");
    }
}
}
D3DDDI_DEVICEFUNCS wrapDriver(HANDLE device,const D3DDDI_DEVICEFUNCS& raw){if(!requested())return raw;
    Id lifetime;{std::lock_guard lock(mutex);auto& d=devices[ptr(device)];d={};d.raw=raw;lifetime=d.lifetime=++deviceSerial;}
    D3DDDI_DEVICEFUNCS result=raw;Visitor visitor{result};forEach<D3DDDI_DEVICEFUNCS>(visitor,UINT_MAX);
#define QFR_REPLACE(field,fn) if(result.field)result.field=&fn
    QFR_REPLACE(pfnCreateResource2,create2);QFR_REPLACE(pfnCreateResource,create1);QFR_REPLACE(pfnDestroyResource,destroyResource);QFR_REPLACE(pfnBlt,blt);QFR_REPLACE(pfnTexBlt,texBlt);QFR_REPLACE(pfnTexBlt1,texBlt1);QFR_REPLACE(pfnLock,lockResource);QFR_REPLACE(pfnUnlock,unlockResource);QFR_REPLACE(pfnSetTexture,setTexture);QFR_REPLACE(pfnSetRenderTarget,setTarget);QFR_REPLACE(pfnSetViewport,setViewport);QFR_REPLACE(pfnSetStreamSourceUm,streamUm);QFR_REPLACE(pfnDrawPrimitive,draw);QFR_REPLACE(pfnDrawIndexedPrimitive2,drawIndexed);QFR_REPLACE(pfnColorFill,colorFill);
    QFR_REPLACE(pfnSetStreamSource,streamGpu);
    QFR_REPLACE(pfnClear,clear);
#undef QFR_REPLACE
    if(result.pfnDestroyDevice)result.pfnDestroyDevice=&destroyDevice;
    if(result.pfnOpenResource)result.pfnOpenResource=&openResource;
    event("device_create",field("device",ptr(device))+field("device_lifetime",lifetime));return result;
}
void logical(D3dDdi::Resource& r,const char* where,HANDLE shadow,bool cpu,bool gpu,UINT sub,const std::string& context) noexcept {if(!enabled())return;try{const auto dev=ptr(r.getDevice());const auto handle=ptr(r);const auto id=resource(dev,handle);const auto& o=r.getOrigDesc();const auto& f=r.getFixedDesc();
    const auto shape=[&](const char* name,const D3DDDIARG_CREATERESOURCE2& d){return ",\""+std::string(name)+"\":{\"format\":"+std::to_string(d.Format)+",\"pool\":"+std::to_string(d.Pool)+",\"flags\":"+std::to_string(d.Flags.Value)+",\"mips\":"+std::to_string(d.MipLevels)+",\"surfaces\":"+std::to_string(d.SurfCount)+",\"width\":"+std::to_string(sub<d.SurfCount?d.pSurfList[sub].Width:0)+",\"height\":"+std::to_string(sub<d.SurfCount?d.pSurfList[sub].Height:0)+"}";};
    event("ownership",field("resource",id)+field("object",reinterpret_cast<uintptr_t>(&r))+field("shadow",resource(dev,ptr(shadow)))+field("shadow_handle",ptr(shadow))+field("sub",sub)+field("original_format",o.Format)+field("fixed_format",f.Format)+shape("original",o)+shape("fixed",f)+",\"where\":"+quote(where)+",\"cpu_current\":"+(cpu?"true":"false")+",\"gpu_current\":"+(gpu?"true":"false")+context);}catch(...){gap("ownership","exception");}}
void snapshot(D3dDdi::Resource& r,UINT sub,const void* data,UINT pitch) noexcept {if(!enabled())return;try{const auto& f=r.getFixedDesc();if(sub>=f.SurfCount){gap("pixels","bad_subresource");return;}rows("pixels",data,f.pSurfList[sub].Width,f.pSurfList[sub].Height,pitch,D3dDdi::getFormatInfo(f.Format).bytesPerPixel,field("resource",resource(ptr(r.getDevice()),ptr(r)))+field("sub",sub)+field("format",f.Format)+",\"stage\":\"before_resource_unlock\"");}catch(...){gap("pixels","exception");}}
void cpuCopy(D3dDdi::Resource& dest,D3dDdi::Resource& src,const D3DDDIARG_BLT& d,HRESULT hr) noexcept {try{copied(dest.getDevice(),d,hr,"cpu");event("cpu_copy_end",field("src",resource(ptr(src.getDevice()),ptr(src)))+field("dst",resource(ptr(dest.getDevice()),ptr(dest)))+signedField("hr",hr));}catch(...){gap("cpu_copy","exception");}}
void prepared(D3dDdi::Resource& source,D3dDdi::Resource& effective,UINT stage) noexcept {try{event("prepared",field("source",resource(ptr(source.getDevice()),ptr(source)))+field("effective",resource(ptr(effective.getDevice()),ptr(effective)))+field("stage",stage));if(enabled()&&!NativeProbe::active())readback(effective.getDevice(),effective,effective.getDevice().getAdapter().getInfo().isD3D9On12);}catch(...){gap("prepared","exception");}}
void palette(D3dDdi::Device& dev,UINT handle,const void* bytes) noexcept {memory("palette",bytes,1024,field("device",ptr(dev))+field("palette",handle)+",\"layout\":\"RGBQUAD256\"");}
void boundary(const char* kind) noexcept {event("boundary",",\"kind\":"+quote(kind));}
}
