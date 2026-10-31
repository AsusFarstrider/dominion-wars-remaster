#pragma once
#include "Runtime.h"
#include <Research/MenuCanvas/quantum_font_research.h>
#include <algorithm>
#include <cstring>
namespace Remaster {
namespace VersionLabel {
using Reader=bool(*)(uint32_t,void*,size_t);
using Writer=bool(*)(uint32_t,const void*,size_t);
inline bool logged=false,rejected=false;
template<class T>inline bool read(Reader r,uint32_t a,T& v){return r(a,&v,sizeof(v));}
inline bool substitute(uint32_t caller,uint32_t destination,uint32_t originalText,uint32_t textArgument,Reader r,Writer w) {
    if(!enabled()||version.label=="Version 1.04"||caller!=0x46AEB9)return false;
    auto refuse=[&](const char* reason){if(!rejected){note("version_label_refused",reason);rejected=true;}return false;};
    uint32_t current=0,rootId=0,rootFlags=0,parent=0,painter=0,resource=0,holder=0,outer=0,original=0,stateAddress=0;
    if(!read(r,0x4D0A24,current)||current!=7||!read(r,0x4E47C0,rootId)||rootId!=0x405||
       !read(r,0x4E47C0+4,rootFlags)||(rootFlags&~0x1F0000u)!=0x84||
       !read(r,0x4E47C0+0x77,parent)||parent!=0x5F7ED8||!read(r,0x4E47C0+0x4C,painter)||painter!=0x46ADD0||
       !read(r,0x4E47C0+0x50,resource)||resource!=0x201||!read(r,0xA39B64,holder)||!holder||!read(r,holder,outer)||outer!=destination||
       !read(r,0x4D09F4,original)||original!=originalText||!read(r,0x4EC3B8,stateAddress)||!stateAddress)return refuse("Main caller/resource/destination/active-font ownership mismatch");
    char oldText[13]{};if(!r(originalText,oldText,sizeof(oldText))||memcmp(oldText,"Version 1.04",13))return refuse("original label changed");
    QFontStateResearch s{};QFontHeaderResearch f{};
    if(!read(r,stateAddress,s)||!s.font||!read(r,s.font,f)||s.pen_x!=700||s.pen_y!=580||s.alignment||s.background_gate||(s.effects&0x0C)||
       f.first_code!=32||f.last_code!=125||f.pixel_bits!=8||f.glyph_count!=94||f.tracking!=1||f.baseline_adjust!=0)return refuse("unsupported original font/style/pen");
    int pen=s.pen_x,right=pen,bottom=s.pen_y;
    for(unsigned char c:version.label){if(c<f.first_code||c>f.last_code)return refuse("label character outside original font");
        const uint64_t address=uint64_t(s.font)+12+uint64_t(c-f.first_code)*12;QGlyphResearch glyph{};
        if(address>0xFFFFFFFFull||!read(r,uint32_t(address),glyph)||glyph.width>32||glyph.height>32)return refuse("font metric read/bounds");
        const int x=pen+glyph.origin_x,y=s.pen_y+f.baseline_adjust+glyph.origin_y;
        if(x<0||y<0||x+glyph.width>800||y+glyph.height>600)return refuse("new label exceeds logical canvas");
        right=std::max(right,x+glyph.width);bottom=std::max(bottom,y+glyph.height);
        pen+=f.tracking+glyph.origin_x+glyph.width;if(pen<0||pen>32767)return refuse("font advance overflow");
    }
    const auto replacement=uint32_t(reinterpret_cast<uintptr_t>(version.label.c_str()));
    if(!w(textArgument,&replacement,sizeof(replacement)))return refuse("font text argument write failed");
    if(!logged){note("version_label_substituted",version.label+"; original call once; logical bounds=700,580,"+std::to_string(right)+","+std::to_string(bottom)+"; measured_width="+std::to_string(pen-s.pen_x-f.tracking)+"; final_pen="+std::to_string(pen));logged=true;}
    return true;
}
}
}
