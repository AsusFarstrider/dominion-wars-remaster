from pathlib import Path
p=Path('E:/Codex/research/dominion-rendering/hd-video-work-01/DDrawCompat/Research/OpeningFlow/OpeningFlow.cpp')
s=p.read_text()
s=s.replace('struct Movie {std::unique_ptr<HdHistory> hd;','struct Movie {bool hdBound=false;std::unique_ptr<HdHistory> hd;')
s=s.replace('if(reject){originalClose(handle);handle=nullptr;error("HD_open_refused_closed_once");}',
'''if(reject){
        {std::lock_guard<std::recursive_mutex> lock(movieMutex);movies.erase(pointer);}
        originalClose(handle);handle=nullptr;error("HD_open_refused_closed_once");
    }''')
a=s.index('void __stdcall binkClose(');b=s.index('bool movieOwner(',a)
s=s[:a]+'''void __stdcall binkClose(void* handle) {
    const DWORD before=GetLastError();
    std::lock_guard<std::recursive_mutex> lock(movieMutex);
    try {
        const auto pointer=static_cast<uint32_t>(reinterpret_cast<uintptr_t>(handle));const auto i=movies.find(pointer);
        const auto life=i==movies.end()?0:i->second->lifetime;
        const bool hd=i!=movies.end()&&bool(i->second->hd);
        movies.erase(pointer);if(sameThread())moviePublication={};
        trace(hd?"native_hd_close":"native_opening_video_close",n("handle",pointer)+n("lifetime",life));
    }catch(...){error("video_close_observation_failed");}
    SetLastError(before);originalClose(handle);
}
void __stdcall binkGoto(void* handle,unsigned frame,unsigned flags) {
    const DWORD before=GetLastError();
    std::lock_guard<std::recursive_mutex> lock(movieMutex);
    try {
        auto i=movies.find(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(handle)));
        if(i!=movies.end()&&i->second->hd){i->second->hd->reset();moviePublication={};
            trace("native_hd_seek",n("frame",frame)+n("flags",flags)+n("lifetime",i->second->lifetime));}
    }catch(...){error("HD_seek_observation_failed");}
    SetLastError(before);originalGoto(handle,frame,flags);
}
''' +s[b:]
s=s.replace('if(!owner){history.quarantined=true;error("HD_copy_quarantined_resource_identity");SetLastError(before);return -1;}',
'''if(owner)owned->hdBound=true;
        if(!owned->hdBound){history.quarantined=true;error("HD_copy_quarantined_resource_identity");SetLastError(before);return -1;}''')
s=s.replace('const auto copyFlags=history.copyFlags();Busy busy(copyingVideo);','const auto copyFlags=history.copyFlags();const auto sourceRevision=history.revision;Busy busy(copyingVideo);')
s=s.replace('const bool source=history.converted(result)&&live!=movies.end()&&live->second==owned;',
'''const bool source=history.revision==sourceRevision&&history.converted(result)&&live!=movies.end()&&live->second==owned;''')
p.write_text(s)
print('Lifecycle and seek revision guards applied')
