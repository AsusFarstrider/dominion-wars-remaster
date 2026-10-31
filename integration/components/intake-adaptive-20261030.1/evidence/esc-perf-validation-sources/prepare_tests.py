"""Reuse the full actual canvas fixture, adding observed re-entry regression."""
from pathlib import Path
import shutil
here=Path(__file__).resolve().parent
parent=Path('E:/Codex/research/dominion-rendering/production-runtime-source-v2/production-validation-sources/canvas-tests')
tests=here/'tests';tests.mkdir(exist_ok=False)
shutil.copyfile(parent/'RuntimeTestMemory.h',tests/'RuntimeTestMemory.h')
s=(parent/'CanvasRuntimeTests.cpp').read_text()
extra=r'''
void reentryTests() {
    using namespace MenuCanvasTest;
    {
        MainFixture f;f.repair();const auto firstGeneration=publication.generation();
        resourceBoundary("simulated_mission_entry");
        put(0x4D0A24,uint32_t{3});put(0x4D0A28,uint32_t{3});
        put(0x71383C,uint32_t{0x42A7B0});put(0x713840,uint32_t{0x4022A0});
        put(0x5F7ED4,uint32_t{0x47DDC0});scalar(0x718690,0);
        f.begin();check(!active,"mission cannot acquire a CPU menu pass");
        check(publication.mayContainScaledPixels()&&!publication.scaledPredicatesAllowed(),"mission boundary retains scaled provenance but withdraws menu interaction");
        put(0x4D0A24,uint32_t{7});put(0x4D0A28,uint32_t{7});
        put(0x71383C,uint32_t{0x42A290});put(0x713840,uint32_t{0});
        check(family(false,&MenuCanvas::mainMenu),"observed re-entry default filter admits exact original Main family");
        f.point(0x4E4850,400,70);f.expectSuspended();
        f.repair();check(publication.generation()>firstGeneration&&commits==2,"fresh Main repaint establishes a new interactive generation");
        for(size_t i=0;i<6;++i){const auto& b=MenuCanvas::mainMenu.buttons[i];f.point(MenuCanvas::mainMenu.firstButton+uint32_t(i)*0x90,b.x+20,b.y+20);f.expectHit();}
        check(MainBackdrop::value<uint32_t>(0x5F7ED4)==0x47DDC0&&!MainBackdrop::value<uint32_t>(0x5F7ED0),"original input filter pointers remain unchanged");
        resourceBoundary("simulated_new_game_from_returned_main");
        put(0x4D0A24,uint32_t{10});put(0x4D0A28,uint32_t{10});put(0x5F7ED8+0x73,uint32_t{0x4E5050});f.input[1]=0x4E50E0;
        check(family(false,&MenuCanvas::selection),"same retained filter admits original mission selection after New Game");
        f.Fixture::repair();check(lastFamily==&MenuCanvas::selection,"selection publishes its own generation after returned Main");
        const int32_t x=1063,y=101;std::memcpy(f.inputMemory.data()+0x14,&x,4);std::memcpy(f.inputMemory.data()+0x10,&y,4);f.expectHit();
        resourceBoundary("simulated_previous_back_to_main");
        put(0x4D0A24,uint32_t{7});put(0x4D0A28,uint32_t{7});put(0x5F7ED8+0x73,MenuCanvas::mainMenu.root);
        f.repair();f.point(0x4E4850,400,70);f.expectHit();
        check(commits==4,"Main-selection-Main re-entry keeps complete independent repaint commits");
    }
    for(unsigned cause=0;cause<7;++cause){
        MainFixture f;f.repair();put(0x5F7ED4,uint32_t{0x47DDC0});scalar(0x718690,0);
        if(cause==0)put(0x5F7ED4,uint32_t{0x47DCE0});
        if(cause==1)put(0x5F7ED4,uint32_t{0xDEADBEEF});
        if(cause==2)put(0x5F7ED0,uint32_t{0x47DE90});
        if(cause==3)put(0x718690,uint32_t{1});
        if(cause==4)failedRead=0x718690;
        if(cause==5)put(0x4E4850,uint32_t{0x59});
        if(cause==6)put(0x713840,uint32_t{0x4022A0});
        check(!family(),"unknown filter, post filter, active mode, unreadable state, foreign node or scene remains rejected");
        f.begin();check(!active&&commits==1,"rejected re-entry cannot start/publish a replacement pass");
        f.input[1]=0x4E4850;f.expectSuspended();
        check(publication.mayContainScaledPixels(),"failed admission never resets scaled provenance to original input");
    }
}
'''
assert s.count('int main(){')==1
s=s.replace('int main(){',extra+'\nint main(){')
old='mainTests();nativeMainTests();std::cout'
assert s.count(old)==1
s=s.replace(old,'mainTests();nativeMainTests();check(assertions==391,"all inherited canvas regressions still pass");reentryTests();std::cout')
(tests/'EscMenuTests.cpp').write_text(s)
s=(parent/'CanvasRuntimeTests.vcxproj').read_text().replace('CanvasRuntimeTests.cpp','EscMenuTests.cpp')
s=s.replace('<TargetName>CanvasRuntimeTests</TargetName>','<TargetName>EscMenuTests</TargetName>')
(tests/'EscMenuTests.vcxproj').write_text(s)
print(tests)
