#pragma once
#include <chrono>
inline void equivalenceChecks(const std::shared_ptr<SelectionRestoration::Catalog>& c,const OpeningFlow::CardImage& map) {
    auto old=referenceCatalog();check(c->entries.size()==old->entries.size(),"equivalent entry inventory");
    Remaster::OrderedPaint before,after;unsigned slot=0;
    for(const auto& e:c->entries){const auto* legacy=old->get(e.kind,e.key);
        check(legacy&&e.ox==legacy->ox&&e.oy==legacy->oy&&e.w==legacy->w&&e.h==legacy->h&&e.pw==legacy->pw&&e.ph==legacy->ph,"all logical and replacement metrics exactly preserved");
        check(SelectionRestoration::originalPixels(e,legacy->original.data(),legacy->original.size()),"owner bytes match converted SHA256");
        auto changed=legacy->original;changed.back()^=0x80;
        check(!SelectionRestoration::originalPixels(e,changed.data(),changed.size()),"last-byte mutation rejected as well as first");
        if(e.kind==3)continue;
        Remaster::SpriteFrame a,b;
        if(e.kind==2){check(e.indexedCoverage==legacy->indexedCoverage,"glyph coverage and classes copied byte-exact");
            for(unsigned color=0;color<4;++color){std::array<uint16_t,256> ramp{};for(unsigned i=0;i<256;++i)ramp[i]=uint16_t((i*257u+color*7919u)&65535);
                a=LegacySelectionRestoration::colorize(*legacy,ramp);b=SelectionRestoration::colorize(e,ramp);
                check(a.valid()&&b.valid()&&*a.storage==*b.storage,"palette-colored fractional-alpha glyph exact equivalence");}
        }else{a=legacy->body;b=e.body;check(*a.storage==*b.storage,"RGBA artwork copied byte-exact");}
        const int x=int((slot*73)%(801-e.w)),y=int((slot*37)%(601-e.h));++slot;
        const dw::ui::Rect bounds{x,y,x+e.w,y+e.h},clip{x+e.w/4,y,800,600};
        check(before.sprite(a,bounds,clip)&&after.sprite(b,bounds,clip),"identical ordered scene records accepted");
    }
    const unsigned matrix[][2]={{1024,768},{1280,800},{1366,768},{1920,1080},{1920,1200},{2560,1440},{3840,2160},{3440,1440}};
    for(const auto& wh:matrix){const auto w=wh[0],h=wh[1];auto plan=MenuCanvas::Plan::fit(800,600,w,h);
        const size_t pitch=size_t(w)*2+8;std::vector<uint8_t> a(pitch*h,0xB1),b=a;
        MenuCanvas::WriteView av{a.data(),a.size(),pitch,w,h},bv{b.data(),b.size(),pitch,w,h};
        check(SelectionRestoration::composeMap(map,plan->geometry(),av)&&LegacySelectionRestoration::composeMap(map,plan->geometry(),bv),"both backgrounds compose");
        check(before.compose(plan->geometry(),av)&&after.compose(plan->geometry(),bv),"both full ordered offscreen scenes compose");
        check(a==b,"all final RGB565 pixels plus padded row bytes exactly equal");
        unsigned char digest[32]{};check(BCryptHash(BCRYPT_SHA256_ALG_HANDLE,nullptr,0,a.data(),ULONG(a.size()),digest,32)>=0,"equivalent frame SHA256 recorded");
        const char* hex="0123456789abcdef";std::string hash;for(auto v:digest){hash+=hex[v>>4];hash+=hex[v&15];}
        std::cout<<"EQUIVALENT "<<w<<'x'<<h<<" pitch="<<pitch<<" bytes="<<a.size()<<" sha256="<<hash<<'\n';
    }
    const auto* mapEntry=c->get(3,0x801);const auto& pixels=reference(*mapEntry);
    const auto start=std::chrono::steady_clock::now();unsigned matched=0;
    for(unsigned i=0;i<256;++i)matched+=SelectionRestoration::originalPixels(*mapEntry,pixels.data(),pixels.size());
    const auto us=std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-start).count();
    check(matched==256,"bounded identity benchmark did real checks");
    std::cout<<"BENCH map_sha256_repeats=256 bytes_each="<<pixels.size()<<" total_us="<<us<<" mean_us="<<double(us)/256<<"\n";
    const auto fullStart=std::chrono::steady_clock::now();matched=0;
    for(unsigned repeat=0;repeat<128;++repeat)for(const auto& e:c->entries){const auto& raw=reference(e);matched+=SelectionRestoration::originalPixels(e,raw.data(),raw.size());}
    const auto fullUs=std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-fullStart).count();
    check(matched==128*114,"full catalog benchmark checks every identity");
    std::cout<<"BENCH full_catalog_sha256_repeats=128 identities_each=114 bytes_each=593038 total_us="<<fullUs<<" mean_us="<<double(fullUs)/128<<"\n";
}
