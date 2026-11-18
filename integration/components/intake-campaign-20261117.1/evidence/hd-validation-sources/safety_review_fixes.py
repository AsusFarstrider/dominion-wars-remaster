from pathlib import Path
p=Path('E:/Codex/research/dominion-rendering/hd-video-work-01/DDrawCompat/Research/OpeningFlow/OpeningFlow.cpp')
s=p.read_text()
s=s.replace('if(GetEnvironmentVariableW(L"QFR_HD_MOVIE",v,2)!=1||v[0]!=L\'1\')return true;', '''if(GetEnvironmentVariableW(L"QFR_HD_MOVIE",v,2)!=1||v[0]!=L'1') {
        // An experimental HD installation must not become unsafe if launched
        // without its research runner/opt-ins. Refuse DLL load in that case.
        wchar_t path[32768]{};if(!GetModuleFileNameW(nullptr,path,32768))return false;
        std::ifstream file(std::filesystem::path(path).parent_path()/L"rs/Video/Paramount.bik",std::ios::binary);
        uint32_t fields[7]{};file.read(reinterpret_cast<char*>(fields),sizeof(fields));
        if(file.gcount()==sizeof(fields)&&(fields[5]>800||fields[6]>600))return false;
        return true;
    }''')
s=s.replace('    std::shared_ptr<Movie> owned;\n    try {','''    std::shared_ptr<Movie> owned;
    if(!sameThread()) {
        Qfr::event("native_hd_refused_thread",",\\"native_writes\\":false,\\"codec_called\\":false");
        SetLastError(before);return -1;
    }
    try {''')
s=s.replace('if(!sameThread()||!safetyInstalled||copyingVideo||i==movies.end()', 'if(!safetyInstalled||copyingVideo||i==movies.end()')
# All larger copies are refused safely, even if this research mode was not
# enabled. Existing smaller legacy copies keep their ordinary provider.
s=s.replace('    if(hdRequested) {\n        // Header probing', '    {\n        // Header probing')
# Close on a foreign thread may retire a map entry but cannot update UI state.
s=s.replace('        trace(hd?"native_hd_close":"native_opening_video_close",n("handle",pointer)+n("lifetime",life));',
'''        if(sameThread())trace(hd?"native_hd_close":"native_opening_video_close",n("handle",pointer)+n("lifetime",life));''')
s=s.replace('    if(!sameThread())return original;LastError preserve;\n    try {','    if(!sameThread())return original;LastError preserve;\n    std::lock_guard<std::recursive_mutex> lock(movieMutex);\n    try {')
s=s.replace('    if(!sameThread())return;LastError preserve;\n    try {\n        if(hr<0', '    if(!sameThread())return;LastError preserve;\n    std::lock_guard<std::recursive_mutex> lock(movieMutex);\n    try {\n        if(hr<0')
p.write_text(s)
print('Early unconfigured-HD refusal and foreign-thread review fixes applied')
