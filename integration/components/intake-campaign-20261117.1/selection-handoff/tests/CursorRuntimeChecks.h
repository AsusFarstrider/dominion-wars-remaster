namespace {
uint32_t __cdecl cursorOriginalFixture(void*,int,int){return 77;}
void cursorRuntimeChecks(){
    using namespace MenuCanvasTest;
    Fixture f;CMainCursor::owner=GetCurrentThreadId();CMainCursor::available=true;CMainCursor::epoch=1;CMainCursor::frame=0;
    struct Reset {~Reset(){for(auto& lease:CMainCursor::leases)lease.reset();CMainCursor::available=false;CMainCursor::artwork.reset();CMainCursor::fresh={};CMainCursor::token={};}} reset;
    std::ifstream stream("E:/Codex/research/dominion-rendering/mission-select-assets-20261101-04/cursor.dwcur",std::ios::binary);
    CMainCursor::artwork=CMainCursor::Art::parse({std::istreambuf_iterator<char>(stream),{}});check(bool(CMainCursor::artwork),"actual two cursor source frames parse");
    constexpr uint32_t source=0xF1900000;reserve(source,0x48);put(source,uint32_t{0x52545854});put(source+0x28,uint32_t{22});put(source+0x2C,uint32_t{22});
    reserve(0x5F7E58,0x30);put(0x5F7E58,uint32_t{1});put(0x5F7E5C,int16_t{100});put(0x5F7E5E,int16_t{100});put(0x5F7E68,source);put(0x5F7E6C,source);
    put(0x5F7E80,uint32_t{0x4DC5C0});reserve(0x4DC5C0,8);put(0x4DC5C0,std::array<uint32_t,2>{0xAF87,0xFFFFFFFF});scalar(0x71386C,0);
    auto commit=[&]{CMainCursor::Target t{};check(CMainCursor::context(t),"mock commit target readable");CMainCursor::committed(t.outer,t.native,t.surface,t.w,t.h);};
    CMainCursor::frameBegin();commit();check(CMainCursor::fresh.valid,"fresh native menu commit arms cursor");
    const auto background=f.surface.pixels;bool written=false;
    check(CMainCursor::replace(0x43B22F,source,100,100,written)&&written,"exact CMain draw writes native patch");
    check(f.surface.pixels!=background,"cursor adds visible source pixels");
    check(f.surface.refs==2&&CMainCursor::leases[0]&&CMainCursor::leases[0]->written,"owned underlay retains native COM buffer");
    DWORD registers[9]{};registers[6]=registers[5]=22;CMainCursor::sizeObserve(1,registers);
    check(registers[6]==44&&registers[5]==44&&!CMainCursor::token.valid,"size receipt changes dimensions only once");
    registers[6]=registers[5]=22;CMainCursor::sizeObserve(1,registers);check(registers[6]==22&&registers[5]==22,"consumed token never affects later draw");
    CMainCursor::frameBegin();check(f.surface.pixels==background&&f.surface.refs==1&&!CMainCursor::leases[0],"next pre-producer frame restores whole cursor footprint and releases lease");
    commit();written=false;check(!CMainCursor::replace(0x43B230,source,100,100,written)&&!written,"unrelated caller keeps original");
    put(0x5F7E64,uint32_t{1});check(!CMainCursor::replace(0x43B22F,source,100,100,written)&&!written,"other cursor states remain original");put(0x5F7E64,uint32_t{0});
    put(source+0x28,uint32_t{88});check(!CMainCursor::replace(0x43B22F,source,100,100,written),"source image density cannot redefine logical TXTR extent");put(source+0x28,uint32_t{22});
    CMainCursor::boundary();check(!CMainCursor::replace(0x43B22F,source,100,100,written),"resource invalidation requires a new full paint");
    commit();f.surface.lockResult=DDERR_SURFACELOST;written=false;
    check(!CMainCursor::replace(0x43B22F,source,100,100,written)&&!written,"Lock failure allows original before writes");f.surface.lockResult=DD_OK;
    CMainCursor::frameBegin();commit();f.surface.unlockResult=DDERR_SURFACELOST;written=false;
    check(CMainCursor::replace(0x43B22F,source,100,100,written)&&written,"post-write Unlock failure remains handled without small original fallback");
    check(CMainCursor::leases[0]&&CMainCursor::leases[0]->written&&CMainCursor::leases[0]->retired,"post-write cleanup retained until verified repaint");f.surface.unlockResult=DD_OK;
    f.surface.pixels=background;commit();check(!CMainCursor::leases[0]&&f.surface.refs==1,"fresh full overwrite retires failed-write footprint");
    for(const auto campaign:{0u,1u}){
        put(0x71386C,campaign);CMainCursor::frameBegin();commit();written=false;
        check(CMainCursor::replace(0x43B22F,source,100,100,written)&&written,"both campaign variants share native CMain policy");
        CMainCursor::frameBegin();check(f.surface.pixels==background,"both variants restore transparent underlay");
    }
    put(0x71386C,uint32_t{0});commit();written=false;
    check(CMainCursor::replace(0x43B22F,source,100,100,written),"setup outgoing state cursor");
    put(0x4D0A24,uint32_t{3});const auto outgoing=f.surface.pixels;CMainCursor::frameBegin();check(f.surface.pixels==outgoing,"never restores menu pixels into gameplay state");
    put(0x4D0A24,uint32_t{10});CMainCursor::boundary();f.surface.pixels=background;commit();check(!CMainCursor::leases[0],"fresh native re-entry retires old state lease");
    // Neither foreign hook path may steal the owner thread's size receipt.
    CMainCursor::Target t{};check(CMainCursor::context(t),"cursor target for foreign/release fixtures");
    CMainCursor::token={true,t,source,CMainCursor::frame,CMainCursor::epoch};const auto ownerToken=CMainCursor::token;
    const auto original=CMainCursor::originals[0];CMainCursor::originals[0]=reinterpret_cast<void*>(&cursorOriginalFixture);
    uint32_t returned=0;DWORD foreignRegs[9]{};foreignRegs[5]=foreignRegs[6]=22;
    std::thread other([&]{returned=CMainCursor::drawHook(nullptr,0,0);CMainCursor::sizeObserve(1,foreignRegs);});other.join();CMainCursor::originals[0]=original;
    check(returned==77&&foreignRegs[5]==22&&foreignRegs[6]==22,"foreign draw forwards original and size observer changes no registers");
    check(CMainCursor::token.valid&&CMainCursor::token.source==ownerToken.source&&CMainCursor::token.frame==ownerToken.frame&&CMainCursor::foreign.load(),"foreign hooks cannot consume owner token");
    CMainCursor::foreign=false;CMainCursor::frameBegin();commit();written=false;
    check(CMainCursor::replace(0x43B22F,source,100,100,written),"setup committed lease for Release reentry");
    f.surface.pixels=background;f.surface.boundaryOnRelease=true;commit();
    check(!CMainCursor::fresh.valid,"COM Release invalidation prevents commit from blessing a new epoch");
    CMainCursor::frameBegin();commit();f.surface.boundaryOnAddRef=true;written=false;
    check(!CMainCursor::replace(0x43B22F,source,100,100,written)&&!written&&f.surface.pixels==background,"COM AddRef invalidation prevents native cursor write");
    CMainCursor::frameBegin();commit();check(CMainCursor::context(t),"target remains valid after AddRef reentry");
    auto old=std::make_unique<CMainCursor::Lease>(t);old->target.outer++;old->epoch=CMainCursor::epoch;old->retired=true;CMainCursor::leases[t.index]=std::move(old);
    f.surface.boundaryOnRelease=true;written=false;
    check(!CMainCursor::replace(0x43B22F,source,100,100,written)&&!written&&f.surface.pixels==background,"old different-target COM Release invalidation prevents native write");
}
}
