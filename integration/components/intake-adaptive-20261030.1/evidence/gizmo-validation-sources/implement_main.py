"""Wire a shared producer layer into the two existing Main publishers."""
from pathlib import Path
base=Path('E:/Codex/research/dominion-rendering/gizmo-main-work-01/DDrawCompat/Research')
p=base/'MenuCanvas/CanvasRuntime.cpp';s=p.read_text();assert 'MainBackdrop.h' not in s
s=s.replace('#include "CanvasHookSites.h"','#include "MainBackdrop.h"\n#include "CanvasHookSites.h"')
s=s.replace('std::vector<uint16_t> pixels;QImageResearch image{};','std::shared_ptr<MainBackdrop::Layer> backdrop;std::vector<uint16_t> pixels;QImageResearch image{};')
s=s.replace('if(active&&active->good){active->good=false;', 'if(active&&active->good){if(active->backdrop)active->backdrop->mainOwns=false;active->good=false;')
s=s.replace('p->pixels.assign(800*600+64,0xA55A);', '''if(f==&MenuCanvas::mainMenu){p->backdrop=MainBackdrop::borrow({t.outer,t.surface,t.w,t.h},p->id);if(p->backdrop)p->backdrop->mainOwns=true;}
    p->pixels.assign(800*600+64,0xA55A);''')
at=s.index('    // The original main-menu painter clears',s.index('void shadow('))
s=s[:at]+'''    if(active->backdrop) {
        const auto result=MainBackdrop::mirror(*active->backdrop,args,trampoline,active->currentNode);
        if(result==MainBackdrop::Mirror::failed){reject("native_background_producer_rejected");return;}
        if(result==MainBackdrop::Mirror::clear)active->nativeClear=true;
        else {++shadowDraws;++active->draws;if(result==MainBackdrop::Mirror::background)active->background=true;}
        return;
    }
''' +s[at:]
s=s.replace('copied=plan->compose({active->pixels.data()+32,800*600*2,1600,800,600},',
'''copied=active->backdrop?MainBackdrop::compose(*active->backdrop,*plan,
                        {d.lpSurface,size_t(d.lPitch)*d.dwHeight,size_t(d.lPitch),d.dwWidth,d.dwHeight}):plan->compose({active->pixels.data()+32,800*600*2,1600,800,600},''')
p.write_text(s)
p=base/'OpeningFlow/OpeningFlow.cpp';s=p.read_text();assert 'MainBackdrop.h' not in s
s=s.replace('\nnamespace OpeningFlow {','\n#include <Research/MenuCanvas/MainBackdrop.h>\nnamespace OpeningFlow {',1)
s=s.replace('bool modalPainted=false,directLoading=false;};','bool modalPainted=false,directLoading=false;std::shared_ptr<MainBackdrop::Layer> backdrop;};')
s=s.replace('if(mainMonitor&&!p.expectedModal)return;', 'if(mainMonitor&&!p.expectedModal)return;\n    if(mainMonitor&&p.backdrop&&p.backdrop->composed)return;')
s=s.replace('ownerThread=GetCurrentThreadId();enabled=true;loadCards();', 'ownerThread=GetCurrentThreadId();enabled=true;loadCards();MainBackdrop::install();')
s=s.replace('void observe(unsigned index,const uint32_t* r,const uint32_t* s,uint32_t sp) noexcept', 'void observe(unsigned index,const uint32_t* r,const uint32_t* s,uint32_t sp,void* originalBlit) noexcept')
s=s.replace('pass={c,sp-12,epoch,false,false,t};', '''pass={c,sp-12,epoch,false,false,t};
                    if(c->root==0x4E47C0||(c->modal&&c->underlying==0x4E47C0))
                        pass.backdrop=MainBackdrop::begin({value(value(0xA39B64)),t.surface,t.w,t.h},sp);''')
s=s.replace('else if(pass.spec&&index==11)pass.gpu=true;', '''else if(pass.spec&&index==11){pass.gpu=true;if(pass.backdrop)pass.backdrop->fail("hardware_foreground_not_supported");}''')
s=s.replace('else if(pass.spec&&index==10) {\n            QImageResearch source{};', '''else if(pass.spec&&index==10) {
            if(pass.backdrop&&!pass.backdrop->mainOwns)MainBackdrop::mirror(*pass.backdrop,s,originalBlit,pass.node);
            QImageResearch source{};''')
s=s.replace('        if(card) {\n            surfaces[now.surface].scaled=true;', '''        if(p.backdrop&&p.backdrop->good) {
            surfaces[now.surface].scaled=true;uiMembers=std::move(nodes);uiPublication={++serial,epoch,p.spec,now.surface,false};
            copied=MainBackdrop::compose(*p.backdrop,*plan,view(d));
        }else if(p.backdrop&&p.backdrop->nativeWritten) {
            error("native_background_failed_after_write_no_snapshot_fallback");
        }else if(card) {
            surfaces[now.surface].scaled=true;''')
s=s.replace('++epoch;pass={};lease={};loading={};', '++epoch;MainBackdrop::boundary();pass={};lease={};loading={};')
p.write_text(s)
p=base/'OpeningFlow/OpeningFlow.h';s=p.read_text().replace('void observe(unsigned,const uint32_t*,const uint32_t*,uint32_t) noexcept;', 'void observe(unsigned,const uint32_t*,const uint32_t*,uint32_t,void* originalBlit=nullptr) noexcept;');p.write_text(s)
p=base/'MenuProbe/Probe.cpp';s=p.read_text().replace('OpeningFlow::observe(index,r,s,sp);','OpeningFlow::observe(index,r,s,sp,originals[10]);');p.write_text(s)
print('Shared Main producer/compositor wired into existing publication paths')
