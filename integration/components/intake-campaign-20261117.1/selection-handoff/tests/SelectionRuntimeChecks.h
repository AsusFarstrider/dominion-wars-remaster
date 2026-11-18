// Included after the inherited mock-memory controller fixtures.
namespace {
void selectionRuntimeChecks(){
    using namespace MenuCanvasTest;
    const std::filesystem::path folder="E:/Codex/research/dominion-rendering/selection-reference-free-assets-20261102-01";
    std::ifstream f(folder/"selection.dwselect",std::ios::binary);std::vector<uint8_t> bytes{std::istreambuf_iterator<char>(f),{}};
    SelectionRestoration::catalog=SelectionRestoration::Catalog::parse(std::move(bytes));
    check(bool(SelectionRestoration::catalog),"runtime fixture loads actual pinned catalog");
    SelectionRestoration::thread=GetCurrentThreadId();SelectionRestoration::flags=15;
    Fixture fixture;
    constexpr uint32_t face=0xF1500000,stateAddress=0xF1400000;
    reserve(face,12+96*12);QFontHeaderResearch header{};header.first_code=32;header.last_code=127;header.pixel_bits=8;header.tracking=1;header.unknown_04=1;header.max_y_extent=24;header.glyph_count=96;put(face,header);
    for(unsigned i=0;i<96;++i){const auto* e=SelectionRestoration::catalog->get(2,i+32);check(bool(e),"all glyph fixture bindings");
        const auto pixels=0xF1600000+i*0x1000;reserve(pixels,reference(*e).size());write(pixels,reference(*e).data(),reference(*e).size());
        QGlyphResearch g{e->ox,e->oy,e->w,e->h,pixels};put(face+12+i*12,g);}
    scalar(0x4EC3B8,stateAddress);reserve(stateAddress,sizeof(QFontStateResearch));
    QFontStateResearch state{};state.font=face;state.and_mask=255;put(stateAddress,state);
    constexpr uint32_t node=0x4E50E0;put(node+0x5C,uint16_t{0x1407});
    const auto* entry=SelectionRestoration::catalog->get(2,'A');QImageResearch src{};
    src.flags=3;src.and_mask=255;src.pixel_bits=8;src.origin_x=entry->ox;src.origin_y=entry->oy;src.width=entry->w;src.height=entry->h;src.pixels=0xF1600000+('A'-32)*0x1000;
    const auto record=face+12+('A'-32)*12;
    SelectionRestoration::Layer layer;
    check(layer.glyph(node,0x4A2B1B,0x4EC3C0,record,src)==entry,"exact saved EDI glyph join accepted");
    check(!layer.glyph(node,0x4A2B16,0x4EC3C0,record,src),"call instruction is not return address");
    check(!layer.glyph(node,0x4A2B1B,0x4EC3C4,record,src),"wrong scratch rejected");
    check(!layer.glyph(node,0x4A2B1B,0x4EC3C0,record+1,src),"misaligned glyph record rejected");
    check(!layer.glyph(node,0x4A2B1B,0x4EC3C0,face+12+96*12,src),"out of range glyph record rejected");
    put(0x4E53B0+0x5C,uint16_t{0x1407});
    check(layer.glyph(0x4E53B0,0x4A2B1B,0x4EC3C0,record,src)==entry,"CRT-initialized Previous uses verified007 glyph join");
    check(!layer.glyph(0x4E53B1,0x4A2B1B,0x4EC3C0,record,src),"sixth exact owner never admits adjacent address");
    auto wrong=src;wrong.pixels++;check(!layer.glyph(node,0x4A2B1B,0x4EC3C0,record,wrong),"scratch pixels must match exact glyph record");
    wrong=src;wrong.origin_x++;check(!layer.glyph(node,0x4A2B1B,0x4EC3C0,record,wrong),"scratch bearing mismatch rejected");
    put(node+0x5C,uint16_t{0x1406});check(!layer.glyph(node,0x4A2B1B,0x4EC3C0,record,src),"other font ID does not borrow007");put(node+0x5C,uint16_t{0x1407});
    state.effects=0x10;put(stateAddress,state);check(!layer.glyph(node,0x4A2B1B,0x4EC3C0,record,src),"unsupported monochrome effect retains original");state.effects=0;put(stateAddress,state);
    SelectionRestoration::Layer corrupted;auto changed=reference(*entry);changed[0]^=1;write(src.pixels,changed.data(),changed.size());
    check(!corrupted.glyph(node,0x4A2B1B,0x4EC3C0,record,src),"full face content identity rejects altered font");write(src.pixels,reference(*entry).data(),reference(*entry).size());
    const auto* body=SelectionRestoration::catalog->get(1,0x803);constexpr uint32_t source=0xF1700000,pixels=0xF1800000;
    reserve(pixels,reference(*body).size());write(pixels,reference(*body).data(),reference(*body).size());put(node+0x10,uint16_t{0x802});
    QImageResearch b{};b.flags=3;b.and_mask=255;b.pixel_bits=8;b.width=body->w;b.height=body->h;b.pixels=pixels;
    std::vector<uint16_t> logical(800*600);const dw::ui::Rect clip{0,0,800,600};
    auto before=SelectionRestoration::denseBodies;
    check(layer.record(node,0x40F984,0x41E328,source,source,node,b,128,48,clip,logical.data(),false),"known body producer admitted");
    check(SelectionRestoration::denseBodies==before+1,"actual hover pixels select803");
    check(layer.record(node,0x40F984,0x41E329,source,source,node,b,128,48,clip,logical.data(),false),"unknown enclosing caller retains raw draw");
    check(SelectionRestoration::denseBodies==before+1,"caller alone cannot identify replacement");
    changed=reference(*body);changed[0]^=1;write(pixels,changed.data(),changed.size());
    check(layer.record(node,0x40F984,0x41E328,source,source,node,b,128,48,clip,logical.data(),false),"modified body retains raw original");
    check(SelectionRestoration::denseBodies==before+1,"dimensions alone never identify replacement");
    for(unsigned variant=0;variant<3;++variant){const auto* back=SelectionRestoration::catalog->get(1,0x12+variant);
        reserve(pixels,reference(*back).size());write(pixels,reference(*back).data(),reference(*back).size());put(0x4E53B0+0x10,uint16_t{0x12});
        b.width=back->w;b.height=back->h;b.pixels=pixels;before=SelectionRestoration::denseBodies;
        check(layer.record(0x4E53B0,0x40F984,0x41E328,source,source,0x4E53B0,b,40,558,clip,logical.data(),false),"Previous body retains existing producer chain");
        check(SelectionRestoration::denseBodies==before+1,"Previous state admitted by exact original pixels");}
    for(const auto& faction:SelectionRestoration::factions){const auto* art=SelectionRestoration::catalog->get(4,faction.key);check(bool(art),"faction resource explicitly catalogued");
        reserve(pixels,reference(*art).size());write(pixels,reference(*art).data(),reference(*art).size());b.width=art->w;b.height=art->h;b.pixels=pixels;
        const auto initial=SelectionRestoration::denseFactions;
        check(layer.record(0x4E5050,faction.caller,0,source,0,0,b,faction.x,faction.y,clip,logical.data(),false),"faction original caller/anchor/content admitted");
        check(SelectionRestoration::denseFactions==initial+1,"faction source selects exact normal/highlight art");
        check(layer.record(0x4E5050,faction.caller+1,0,source,0,0,b,faction.x,faction.y,clip,logical.data(),false),"unexpected faction caller retains original");
        check(SelectionRestoration::denseFactions==initial+1,"faction dimensions alone insufficient");
        check(layer.record(0x4E5050,faction.caller,0,source,0,0,b,faction.x+1,faction.y,clip,logical.data(),false),"unexpected faction anchor retains original");
        check(SelectionRestoration::denseFactions==initial+1,"art cannot move original faction anchor");}
    failNextAllocation=true;SelectionRestoration::finish();check(!failNextAllocation,"optional finish diagnostic contains allocation exception");
    SelectionRestoration::flags=0;check(!SelectionRestoration::begin(),"zero flags keeps original controller");SelectionRestoration::catalog.reset();
}
}
