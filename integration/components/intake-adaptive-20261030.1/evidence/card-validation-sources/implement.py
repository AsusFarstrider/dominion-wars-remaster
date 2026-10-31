"""Add native-source opening cards to the isolated, tested HD parent."""
from pathlib import Path
p=Path('E:/Codex/research/dominion-rendering/opening-cards-work-01/DDrawCompat/Research/OpeningFlow/OpeningFlow.cpp')
s=p.read_text();assert 'CardImage.h' not in s
s=s.replace('#include "HdHistory.h"','#include "HdHistory.h"\n#include "CardImage.h"')
s=s.replace('uint64_t hdLifetime=0;};','uint64_t hdLifetime=0;uint32_t cardResource=0,cardW=0,cardH=0;};')
at=s.index('\nbool supportedTree(')
s=s[:at]+'''
struct Card {
    unsigned state,resource;const char* name;const wchar_t* file;const char* digest;
    std::shared_ptr<CardImage> image;bool pixelReceipt=false;
};
bool cardsEnabled=false;
Card cards[]={
    {5,0x101,"publisher",L"publisher.dwcard","4b89bd1608a09db212401246b91584fb398275599e91b1c7c575f2e9d31055c5",{},false},
    {6,0x103,"title",L"title.dwcard","3818d7e2a72b634585edfb4134fe64869fce6d7d26757f8e255e24ce79b816bf",{},false}};
Card* cardFor(const Spec& spec) {
    if(!cardsEnabled)return nullptr;
    for(auto& card:cards)if(card.state==spec.state&&card.image&&value(spec.root+0x50)==card.resource&&
        !value(spec.root+0x73))return &card;
    return nullptr;
}
void loadCards();
void cardPixels(Card& card,const DDSURFACEDESC2& d) {
    if(card.pixelReceipt||!card.image->identity(d.dwWidth,d.dwHeight))return;
    const auto bytes=card.image->bytes();std::vector<unsigned char> packed(bytes);
    for(unsigned y=0;y<d.dwHeight;++y)memcpy(packed.data()+size_t(y)*d.dwWidth*2,
        static_cast<unsigned char*>(d.lpSurface)+size_t(y)*d.lPitch,size_t(d.dwWidth)*2);
    unsigned char a[32]{},b[32]{};
    const bool hashed=BCryptHash(BCRYPT_SHA256_ALG_HANDLE,nullptr,0,static_cast<PUCHAR>(const_cast<void*>(card.image->data())),ULONG(bytes),a,32)>=0&&
        BCryptHash(BCRYPT_SHA256_ALG_HANDLE,nullptr,0,packed.data(),ULONG(bytes),b,32)>=0;
    const bool equal=hashed&&!memcmp(a,b,32);std::string sa,sb;const char* hex="0123456789abcdef";
    for(unsigned i=0;i<32;++i){sa+=hex[a[i]>>4];sa+=hex[a[i]&15];sb+=hex[b[i]>>4];sb+=hex[b[i]&15];}
    trace("native_card_pixels",n("resource",card.resource)+n("bytes",bytes)+",\\"source_sha256\\":"+Qfr::quote(sa)+
        ",\\"target_sha256\\":"+Qfr::quote(sb)+",\\"equal\\":"+(equal?"true":"false"));
    card.pixelReceipt=true;if(!equal)throw std::runtime_error("card identity mismatch");
}
''' +s[at:]
s=s.replace('    Busy busy(completing);\n    auto* surface=', '    auto* card=cardFor(*p.spec);\n    Busy busy(completing);\n    auto* surface=',1)
s=s.replace('if(format(d,now)&&p.epoch==epoch&&ownerValid()&&planFor(now)) {\n        if(!uiCanvas)',
'''if(format(d,now)&&p.epoch==epoch&&ownerValid()&&(card||planFor(now))) {
        if(card) {
            surfaces[now.surface].scaled=true;uiMembers=std::move(nodes);uiPublication={++serial,epoch,p.spec,now.surface,false};
            uiPublication.cardResource=card->resource;uiPublication.cardW=now.w;uiPublication.cardH=now.h;
            copied=card->image->compose(view(d));if(copied)cardPixels(*card,d);
        }else {
        if(!uiCanvas)''',1)
s=s.replace('    const auto result=unlock.close();\n    if(!copied', '    }\n    const auto result=unlock.close();\n    if(!copied',1)
s=s.replace('uiPublication={++serial,epoch,p.spec,now.surface,!mainMonitor&&!p.directLoading&&context()==p.spec};++frames;',
'''uiPublication={++serial,epoch,p.spec,now.surface,!mainMonitor&&!p.directLoading&&context()==p.spec};++frames;
    if(card) {
        uiPublication.cardResource=card->resource;uiPublication.cardW=now.w;uiPublication.cardH=now.h;
        const auto fit=dw::ui::Canvas::fit({0,0,card->image->width(),card->image->height()},{0,0,now.w,now.h});
        const auto bounds=fit->content_bounds();
        trace("native_card_commit",n("generation",serial)+n("root",p.spec->root)+n("resource",card->resource)+n("surface",now.surface)+
            n("source_width",card->image->width())+n("source_height",card->image->height())+n("target_width",now.w)+n("target_height",now.h)+n("target_pitch",d.lPitch)+
            n("left",bounds.left)+n("top",bounds.top)+n("right",bounds.right)+n("bottom",bounds.bottom)+
            ",\\"identity\\":"+(card->image->identity(now.w,now.h)?"true":"false")+",\\"interactive\\":"+(uiPublication.interactive?"true":"false"));
    }''')
# The existing flat card callbacks consume no coordinates, so only their exact
# committed root uses native-screen admission. Other UI keeps its existing inverse.
s=s.replace('        bool admitted=known&&publication.interactive', '''        const bool nativeCard=address==s->root&&publication.cardResource&&cardFor(*s)&&
            publication.cardResource==value(s->root+0x50);
        bool admitted=known&&publication.interactive''')
s=s.replace('(hd?(t.w==2560&&t.h==1440):(plan&&planW==t.w&&planH==t.h));',
 '(hd?(t.w==2560&&t.h==1440):nativeCard?(t.w==publication.cardW&&t.h==publication.cardH):(plan&&planW==t.w&&planH==t.h));')
s=s.replace('if(admitted&&hd)hit=', 'if(admitted&&(hd||nativeCard))hit=')
# Load only explicit scoped files, verified independently from source dimensions.
at=s.index('\n}\nbool installSafety() noexcept')
s=s[:at]+'''
void loadCards() {
    wchar_t env[2]{};if(GetEnvironmentVariableW(L"QFR_OPENING_CARDS",env,2)!=1||env[0]!=L'1')return;
    wchar_t exe[32768]{};if(!GetModuleFileNameW(nullptr,exe,32768))throw std::runtime_error("card module path");
    const auto base=std::filesystem::path(exe).parent_path()/L"rs/Remaster/OpeningCards";
    for(auto& card:cards) {
        const auto path=base/card.file;
        if(!fileIdentity(path,card.digest)){error("card_asset_missing_or_identity_mismatch_original_retained");continue;}
        std::ifstream stream(path,std::ios::binary);std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(stream)),{});
        card.image=CardImage::parse(bytes);if(!card.image){error("card_asset_invalid_original_retained");continue;}
        cardsEnabled=true;trace("native_card_asset",n("resource",card.resource)+n("source_width",card.image->width())+n("source_height",card.image->height())+
            ",\\"name\\":"+Qfr::quote(card.name)+",\\"asset_sha256\\":"+Qfr::quote(card.digest));
    }
}
''' +s[at:]
s=s.replace('ownerThread=GetCurrentThreadId();enabled=true;', 'ownerThread=GetCurrentThreadId();enabled=true;loadCards();')
p.write_text(s)
print('Native opening-card adapter applied')
