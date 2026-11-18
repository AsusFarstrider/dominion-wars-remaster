#include <Remaster/SelectionCatalog.h>
#include <Remaster/CursorRaster.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include "ReferenceCatalog.h"
static unsigned checks=0;
static void check(bool v,const char* message){++checks;if(!v)throw std::runtime_error(message);}
static std::vector<uint8_t> file(const std::filesystem::path& p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
#include "EquivalenceChecks.h"
int main(int argc,char** argv){try{
    check(argc==2,"asset path argument");const std::filesystem::path path=argv[1];
    const auto input=file(path/"selection.dwselect");auto c=SelectionRestoration::Catalog::parse(input);check(bool(c),"actual catalog parse");check(c->entries.size()==114,"all bound entries");
    check(!SelectionRestoration::Catalog::parse({}),"missing catalog rejected");
    for(const auto n:{size_t(8),size_t(12),input.size()-1})check(!SelectionRestoration::Catalog::parse({input.begin(),input.begin()+n}),"truncated catalog rejected");
    auto bad=input;bad.push_back(0);check(!SelectionRestoration::Catalog::parse(bad),"trailing payload rejected");
    bad=input;bad[8]=0;check(!SelectionRestoration::Catalog::parse(bad),"invalid count rejected");
    bad=input;bad[12]=99;check(!SelectionRestoration::Catalog::parse(bad),"unknown record kind rejected");
    bad=input;bad[6]='1';check(!SelectionRestoration::Catalog::parse(bad),"legacy format explicitly rejected");
    bad=input;std::fill(bad.begin()+36,bad.begin()+68,uint8_t{});check(!SelectionRestoration::Catalog::parse(bad),"zero fingerprint rejected");
    bad=input;bad[68]=1;check(!SelectionRestoration::Catalog::parse(bad),"transparent class cannot become glyph ink");
    bad=input;bad[69]=1;check(!SelectionRestoration::Catalog::parse(bad),"non-font record cannot hide class data");
    size_t recordPosition=12;
    for(const auto& entry:c->entries){if(entry.kind==2&&!std::all_of(entry.classes.begin(),entry.classes.end(),[](uint8_t v){return !v;})){
            bad=input;std::fill(bad.begin()+recordPosition+56,bad.begin()+recordPosition+88,uint8_t{});
            check(!SelectionRestoration::Catalog::parse(bad),"covered index outside class mask rejected");break;}
        recordPosition+=88+(entry.kind==3?0:size_t(entry.pw)*entry.ph*(entry.kind==2?2:4));}
    for(const auto& e:c->entries){
        check(reference(e).size()==size_t(e.w)*e.h,"private test authority reference available");
        check(SelectionRestoration::originalPixels(e,reference(e).data(),reference(e).size()),"full pixels match");
        auto changed=reference(e);changed[0]^=1;check(!SelectionRestoration::originalPixels(e,changed.data(),changed.size()),"one differing pixel rejects identity");
        QImageResearch q{};q.width=e.w;q.height=e.h;q.origin_x=e.ox;q.origin_y=e.oy;check(SelectionRestoration::shape(e,q),"original metric match");++q.width;check(!SelectionRestoration::shape(e,q),"modified metric rejected");
    }
    const auto* a=c->get(2,'A');check(a&&a->w==6&&a->h==18&&a->pw==24&&a->ph==72,"glyph density independent of logical dimensions");
    std::array<uint16_t,256> ramp{};ramp[8]=0xF800;ramp[7]=0x07E0;ramp[1]=0;
    const auto first=SelectionRestoration::colorize(*a,ramp);check(first.valid(),"colorized glyph valid");
    const auto cached=SelectionRestoration::colorize(*a,ramp);check(first.storage==cached.storage,"immutable same-ramp cache");
    for(size_t i=0;i<size_t(a->pw)*a->ph;++i){const auto* p=first.pixels()+i*4;check(p[3]==a->indexedCoverage[i*2+1],"fractional coverage retained independently of class");}
    ramp[8]=0x001F;const auto second=SelectionRestoration::colorize(*a,ramp);check(first.storage!=second.storage,"palette mutation owns a new command snapshot");
    SelectionRestoration::Entry solid;solid.kind=2;solid.pw=solid.ph=1;solid.indexedCoverage={1,255};
    std::array<uint16_t,256> blackRamp{};const auto black=SelectionRestoration::colorize(solid,blackRamp);check(black.pixels()[3]==255&&black.pixels()[0]==0,"opaque black is not transparency");
    const auto map=OpeningFlow::CardImage::parse(file(path/"selection.dwcard"));check(bool(map),"actual map parse");
    equivalenceChecks(c,*map);
    const unsigned dimensions[][2]={{1024,768},{1280,800},{1366,768},{1920,1080},{1920,1200},{2560,1440},{3840,2160},{3440,1440}};
    for(const auto& wh:dimensions){const auto w=wh[0],h=wh[1];const auto plan=MenuCanvas::Plan::fit(800,600,w,h);check(bool(plan),"UI fit for output matrix");
        std::vector<uint16_t> buffer(size_t(w)*h+64,0xA55A);MenuCanvas::WriteView out{buffer.data()+32,size_t(w)*h*2,size_t(w)*2,w,h};
        check(SelectionRestoration::composeMap(*map,plan->geometry(),out),"adaptive map composition");
        for(unsigned n=0;n<32;++n)check(buffer[n]==0xA55A&&buffer[buffer.size()-1-n]==0xA55A,"physical target canaries");
        const auto core=plan->geometry().content_bounds();const auto* data=static_cast<const uint16_t*>(map->data());
        const unsigned x=unsigned(core.left),y=unsigned(core.top);const auto sx=unsigned(std::floor(320.0+960.0/double(core.right-core.left)));
        const auto sy=unsigned(std::floor(720.0/double(core.bottom-core.top)));
        check(buffer[32+size_t(y)*w+x]==data[size_t(sy)*2560+sx],"core starts at authored anchor rather than double fitting whole map");
        if(w==2560&&h==1440)check(!memcmp(buffer.data()+32,map->data(),map->bytes()),"1440 identity map pixels");
        // Dense foreground is placed using the existing original UI transform.
        Remaster::OrderedPaint commands;auto rgba=std::make_shared<std::vector<uint8_t>>(size_t(16*16*4),uint8_t(255));
        for(size_t k=0;k<16*16;++k){(*rgba)[k*4]=uint8_t(k%2?255:0);(*rgba)[k*4+1]=0;(*rgba)[k*4+2]=0;}
        check(commands.sprite({rgba,0,16,16},{128,48,132,52},{128,48,130,52}),"bounded clipped dense operation accepted");
        check(commands.compose(plan->geometry(),out),"ordered foreground native output");
        // A later original pixel producer must cover earlier dense artwork.
        std::vector<uint16_t> logical(800*600,0x07E0);const uint8_t index=1;
        check(commands.raw(logical.data(),&index,1,1,1,129,49,true,false,{0,0,800,600}),"later raw command accepted");
        check(commands.compose(plan->geometry(),out),"ordered raw/dense overlap composition");
        auto cell=plan->geometry().map_rect({129,49,130,50});check(bool(cell),"overlap maps through UI");
        check(buffer[32+size_t(cell->top)*w+size_t(cell->left)]==0x07E0,"later producer occludes dense artwork");
    }
    const auto cursorBytes=file(path/"cursor.dwcur");auto cursor=CMainCursor::Art::parse(cursorBytes);check(bool(cursor),"two pinned cursor rasters parse");
    auto corruptedCursor=cursorBytes;corruptedCursor.push_back(0);check(!CMainCursor::Art::parse(corruptedCursor),"cursor extra payload rejected");
    corruptedCursor=cursorBytes;corruptedCursor[16]=0;check(!CMainCursor::Art::parse(corruptedCursor),"wrong cursor identity rejected");
    for(const auto& art:cursor->frames)for(int y:{-43,-1,0,50,89,90})for(int x:{-43,-1,0,75,119,120}){
        constexpr unsigned w=120,h=90;std::vector<uint16_t> target(w*h+64,0x07E0);const auto original=target;
        MenuCanvas::WriteView view{target.data()+32,w*h*2,w*2,w,h};CMainCursor::Patch patch;
        const bool expected=x<int(w)&&y<int(h);check(patch.prepare(view,art,x,y)==expected,"cursor signed clipping accepted only when visible");
        if(!expected){check(target==original,"rejected cursor preparation writes nothing");continue;}
        check(target==original,"cursor captures and prepares before first native write");
        check(patch.apply(view),"prepared cursor patch applies");
        for(unsigned i=0;i<32;++i)check(target[i]==0x07E0&&target[target.size()-1-i]==0x07E0,"clipped cursor preserves target guards");
        check(patch.apply(view,true)&&target==original,"cursor restores exact clipped underlay");
        view.width--;check(!patch.apply(view,true),"underlay never restores into another target shape");
    }
    std::cout<<"PASS "<<checks<<" catalog/color/ordering/adaptive/cursor offscreen checks; no hardware test claim\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<": "<<e.what()<<'\n';return 1;}}
