"""Add the separately identified5C6 profile without changing codec scheduling."""
from pathlib import Path
p=Path('E:/Codex/research/dominion-rendering/gizmo-main-work-01/DDrawCompat/Research/OpeningFlow/OpeningFlow.cpp')
s=p.read_text();assert 'struct HdProfile' not in s
s=s.replace('struct Movie {unsigned hdReceipts=0;', '''struct HdProfile {unsigned resource,frames,rate,divisor,x,y;};
constexpr HdProfile hdProfiles[]={{0x5C5,225,15,1,80,60},{0x5C6,601,2997,100,0,0}};
struct Movie {const HdProfile* profile=nullptr;unsigned hdReceipts=0;''')
a=s.index('bool hdProfile(');b=s.index('\nbool large(',a)
s=s[:a]+'''const HdProfile* hdProfile(uint32_t p,const MovieGeometry& g) {
    if(g.width!=2560||g.height!=1440||g.internalWidth!=g.width||g.internalHeight!=g.height||
        (g.handleFlags&(0x70000000|0x80000)))return nullptr;
    for(const auto& profile:hdProfiles)if(value(p+8)==profile.frames&&value(p+0x14)==profile.rate&&value(p+0x18)==profile.divisor)return &profile;
    return nullptr;
}
''' +s[b:]
s=s.replace('if(hd)m->hd=std::make_unique<HdHistory>(g.width,g.height);', 'if(hd){m->profile=hdProfile(pointer,g);m->hd=std::make_unique<HdHistory>(g.width,g.height);}')
a=s.index('        std::ifstream file(std::filesystem::path(path).parent_path()/L"rs/Video/Paramount.bik"');b=s.index('\n        return true;',a)
s=s[:a]+'''        for(const auto name:{L"Paramount.bik",L"GizmoLogo.bik"}) {
            std::ifstream file(std::filesystem::path(path).parent_path()/L"rs/Video"/name,std::ios::binary);
            uint32_t fields[7]{};file.read(reinterpret_cast<char*>(fields),sizeof(fields));
            if(file.gcount()==sizeof(fields)&&(fields[5]>800||fields[6]>600))return false;
        }''' +s[b:]
s=s.replace('"5a3506e3f48190073591c1349f4e35dbfe8dde9d2ded19364fa865817deecff3"))return false;', '''"5a3506e3f48190073591c1349f4e35dbfe8dde9d2ded19364fa865817deecff3")||
           !fileIdentity(std::filesystem::path(path).parent_path()/L"rs/Video/GizmoLogo.bik",
               "c65bf6cb39b6838ab76f059152790bd0bf437d513c93b4c7b2bdd81d700dc851"))return false;''')
s=s.replace('!hdProfile(pointer,g)||height!=g.height||flags!=10||\n           caller!=0x40F2ED||x!=80||y!=60||',
 '''hdProfile(pointer,g)!=owned->profile||!owned->profile||height!=g.height||flags!=10||
           caller!=0x40F2ED||x!=owned->profile->x||y!=owned->profile->y||''')
s=s.replace('media==value(0x749020+4*0x5C5);','media==value(0x749020+4*owned->profile->resource);')
s=s.replace('if(!owned->hdReceipts||frame==112||(frame>=220&&owned->hdReceipts<3))',
 'if(!owned->hdReceipts||frame==owned->profile->frames/2||(frame>=owned->profile->frames-5&&owned->hdReceipts<3))')
s=s.replace('trace("native_hd_copy",n("handle",pointer)+', 'trace("native_hd_copy",n("external_resource",owned->profile->resource)+n("declared_frames",owned->profile->frames)+n("rate",owned->profile->rate)+n("rate_divisor",owned->profile->divisor)+n("handle",pointer)+')
p.write_text(s);print('Separate Gizmo resource/profile and both-file early gates installed')
