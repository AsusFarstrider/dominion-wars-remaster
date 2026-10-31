from pathlib import Path
base=Path('E:/Codex/research/dominion-rendering/gizmo-main-work-01/DDrawCompat/Research')
p=base/'MenuCanvas/MainBackdrop.h';s=p.read_text()
s=s.replace('inline std::weak_ptr<Layer> current;', 'inline std::weak_ptr<Layer> current;inline uint64_t frames=0;inline bool pixelReceipt=false;')
s=s.replace('auto layer=std::make_shared<Layer>(t,scope);', 'auto layer=std::make_shared<Layer>(t,++frames);(void)scope;')
s=s.replace('    layer.nativeWritten=true;\n    if(!artwork->compose(dst)||!uiPlan.overlay(', '''    std::vector<unsigned char> packed;
    const bool receipt=!pixelReceipt&&artwork->identity(dst.width,dst.height);
    if(receipt)packed.resize(artwork->bytes());
    layer.nativeWritten=true;
    if(!artwork->compose(dst))return false;
    if(receipt) {
        for(unsigned y=0;y<dst.height;++y)memcpy(packed.data()+size_t(y)*dst.width*2,static_cast<unsigned char*>(dst.pixels)+size_t(y)*dst.pitch,size_t(dst.width)*2);
        unsigned char a[32]{},b[32]{};
        const bool hashed=BCryptHash(BCRYPT_SHA256_ALG_HANDLE,nullptr,0,static_cast<PUCHAR>(const_cast<void*>(artwork->data())),ULONG(artwork->bytes()),a,32)>=0&&
            BCryptHash(BCRYPT_SHA256_ALG_HANDLE,nullptr,0,packed.data(),ULONG(packed.size()),b,32)>=0;
        const bool equal=hashed&&!memcmp(a,b,32);std::string sa,sb;const char* hex="0123456789abcdef";
        for(unsigned i=0;i<32;++i){sa+=hex[a[i]>>4];sa+=hex[a[i]&15];sb+=hex[b[i]>>4];sb+=hex[b[i]&15];}
        event("native_main_background_pixels",",\\"scope\\":"+std::to_string(layer.scope)+",\\"source_sha256\\":"+Qfr::quote(sa)+",\\"target_sha256\\":"+Qfr::quote(sb)+",\\"equal\\":"+(equal?"true":"false"));
        if(!equal){layer.fail("native_underlay_pixel_mismatch");return false;}pixelReceipt=true;
    }
    size_t overlaid=0;
    if(!uiPlan.overlay(''')
s=s.replace('layer.mask.data(),layer.mask.bytes(),dst))return false;\n    layer.composed=true;return true;', '''layer.mask.data(),layer.mask.bytes(),dst,&overlaid))return false;
    layer.composed=true;
    event("native_main_background_composed",",\\"scope\\":"+std::to_string(layer.scope)+",\\"surface\\":"+std::to_string(layer.target.surface)+
        ",\\"source_width\\":"+std::to_string(artwork->width())+",\\"source_height\\":"+std::to_string(artwork->height())+
        ",\\"target_width\\":"+std::to_string(dst.width)+",\\"target_height\\":"+std::to_string(dst.height)+
        ",\\"logical_coverage\\":"+std::to_string(layer.mask.covered())+",\\"physical_coverage\\":"+std::to_string(overlaid)+
        ",\\"identity_background\\":"+(artwork->identity(dst.width,dst.height)?"true":"false"));
    return true;''')
p.write_text(s)
p=base/'MenuCanvas/CanvasRuntime.cpp';s=p.read_text()
s=s.replace('++controllerExceptions;publication.invalidate();if(active)active->good=false;',
 '''++controllerExceptions;publication.invalidate();if(active){active->good=false;if(active->backdrop){active->backdrop->good=false;active->backdrop->mainOwns=false;}}''')
s=s.replace('event("native_canvas_commit",",\\"family\\":"', 'event("native_canvas_commit",",\\"background_scope\\":"+std::to_string(active->backdrop?active->backdrop->scope:0)+",\\"family\\":"')
p.write_text(s)
p=base/'OpeningFlow/OpeningFlow.cpp';s=p.read_text()
s=s.replace('trace("native_opening_screen_commit",n("generation",serial)', 'trace("native_opening_screen_commit",n("background_scope",p.backdrop?p.backdrop->scope:0)+n("generation",serial)')
s=s.replace('catch(...){pass={};uiPublication.interactive=false;error("opening_draw_exception");}', 'catch(...){MainBackdrop::boundary();pass={};uiPublication.interactive=false;error("opening_draw_exception");}')
p.write_text(s)
print('Bounded layer/native-pixel receipts and exception handoff guards added')
