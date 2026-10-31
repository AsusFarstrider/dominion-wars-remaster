from pathlib import Path
root=Path('E:/Codex/research/dominion-rendering/gizmo-main-work-01/DDrawCompat/Research')
p=root/'MenuCanvas/MainBackdrop.h';s=p.read_text()
s=s.replace('#include <array>','#include <array>\n#include <atomic>')
s=s.replace('inline std::weak_ptr<Layer> current;', 'inline std::atomic<bool> coverageGapPending{false};\ninline std::weak_ptr<Layer> current;')
line='inline void boundary() {if(auto layer=current.lock()){layer->good=false;layer->mainOwns=false;}current.reset();}'
assert line in s
s=s.replace(line,line+'\n// Foreign hooks only set an atomic signal; all shared layer mutation stays on the owner.\ninline bool drainCoverageGap() {if(!coverageGapPending.exchange(false))return false;boundary();return true;}')
s=s.replace('    if(!layer.good||layer.composed||!layer.background', '    if(drainCoverageGap()){layer.good=false;layer.mainOwns=false;return false;}\n    if(!layer.good||layer.composed||!layer.background')
p.write_text(s)
p=root/'MenuCanvas/CanvasRuntime.cpp';s=p.read_text()
s=s.replace('        MenuCanvas::drainForeignInput(publication,foreignThreadSeen);','        if(MainBackdrop::drainCoverageGap())coverageLost("foreign_coverage_gap");\n        MenuCanvas::drainForeignInput(publication,foreignThreadSeen);')
s=s.replace('        if(foreignThreadSeen.exchange(false))reject("foreign_UI_thread_observed");','        if(MainBackdrop::drainCoverageGap())coverageLost("foreign_coverage_gap");\n        if(foreignThreadSeen.exchange(false))reject("foreign_UI_thread_observed");')
s=s.replace('            if(foreignThreadSeen.exchange(false))reject("foreign_UI_thread_before_publication");','            if(MainBackdrop::drainCoverageGap())coverageLost("foreign_coverage_gap_before_publication");\n            if(foreignThreadSeen.exchange(false))reject("foreign_UI_thread_before_publication");')
old='''    const auto owner=ownerThread.load();if(!owner)return;if(owner!=GetCurrentThreadId()){foreignThreadSeen=true;return;}
    MainBackdrop::boundary();publication.invalidate();'''
assert old in s
s=s.replace(old,'''    const auto owner=ownerThread.load();if(!owner)return;if(owner!=GetCurrentThreadId()){MainBackdrop::coverageGapPending=true;foreignThreadSeen=true;return;}
#ifndef MENU_CANVAS_TEST
    OpeningFlow::resourceBoundary(reason);
#endif
    MainBackdrop::boundary();publication.invalidate();''')
p.write_text(s)
p=root/'OpeningFlow/OpeningFlow.cpp';s=p.read_text()
old='''void observe(unsigned index,const uint32_t* r,const uint32_t* s,uint32_t sp,void* originalBlit) noexcept {
    if(!sameThread())return;LastError preserve;
    try {'''
assert old in s
s=s.replace(old,old+'\n        if(MainBackdrop::drainCoverageGap()){resourceBoundary("foreign_coverage_gap");return;}')
s=s.replace('''void inputScope(unsigned index) noexcept {
    if(!sameThread())return;''','''void inputScope(unsigned index) noexcept {
    if(!sameThread())return;
    if(MainBackdrop::drainCoverageGap())resourceBoundary("foreign_coverage_gap_before_input");''')
s=s.replace('''    const auto result=unlock.close();
    if(!copied||FAILED(result)||p.epoch!=epoch)''','''    const auto result=unlock.close();
    if(MainBackdrop::drainCoverageGap())resourceBoundary("foreign_coverage_gap_before_publication");
    if(!copied||FAILED(result)||p.epoch!=epoch)''')
p.write_text(s)
p=Path(__file__).parent/'canvas-tests/CanvasRuntimeTests.cpp';s=p.read_text().replace('#include <atomic>','#include <atomic>\n#include <thread>')
old='    MainBackdrop::enabled=false;MainBackdrop::artwork.reset();MainBackdrop::current.reset();'
assert old in s
s=s.replace(old,'''    {
        MainFixture f;f.begin();auto held=active->backdrop;
        std::thread gap([]{coverageLost("fixture_foreign_gap");});gap.join();
        check(held->good&&held->mainOwns&&MainBackdrop::coverageGapPending.load(),"foreign gap defers shared mutation to owner");
        observe(2,f.draws,f.arguments,0,reinterpret_cast<void*>(&MainFixture::paletteBlit));
        check(!held->good&&!held->mainOwns&&!active->good&&!MainBackdrop::coverageGapPending.load(),"owner consumes foreign gap and retires incomplete foreground");
        f.end();check(!held->composed&&!publication.scaledPredicatesAllowed(),"foreign gap cannot publish incomplete frame");
    }
'''+old)
p.write_text(s)
print('Applied shared owner/foreign coverage invalidation and regression fixture')
